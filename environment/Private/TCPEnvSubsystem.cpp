// TCPEnvSubsystem.cpp
#include "TCPEnvSubsystem.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "Json.h"
#include "JsonUtilities.h"
#include "UE5Game.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Async/Async.h"
#include "HAL/PlatformProcess.h"

void UTCPEnvSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    bShouldStop.store(false);

    // Hook world init so we can init UE5Game when PIE/Game spawns
    FWorldDelegates::OnPostWorldInitialization.AddUObject(this, &UTCPEnvSubsystem::OnPostWorldInit);

    // Start listening right away
    ListenerThread = std::thread(&UTCPEnvSubsystem::ListenerThreadFunc, this);
    UE_LOG(LogTemp, Log, TEXT("TCPEnvSubsystem: Listener thread started on port %d"), Port);
}

void UTCPEnvSubsystem::Deinitialize()
{
    bShouldStop.store(true);
    FWorldDelegates::OnPostWorldInitialization.RemoveAll(this);

    if (ListenSocket)
        ListenSocket->Close();

    if (ListenerThread.joinable())
        ListenerThread.join();

    if (ClientSocket)
    {
        ClientSocket->Close();
        ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(ClientSocket);
        ClientSocket = nullptr;
    }

    if (ListenSocket)
    {
        ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(ListenSocket);
        ListenSocket = nullptr;
    }

    if (Env)
    {
        Env->RemoveFromRoot();
        Env = nullptr;
    }

    UE_LOG(LogTemp, Log, TEXT("TCPEnvSubsystem: Deinitialized"));
    Super::Deinitialize();
}

void UTCPEnvSubsystem::OnPostWorldInit(UWorld* World, UWorld::InitializationValues InitValues)
{
    // Only once, only for Game or PIE worlds
    if (Env != nullptr)
        return;

    if (!World->IsGameWorld() && World->WorldType != EWorldType::PIE)
        return;

    Env = NewObject<UUE5Game>(this);
    Env->AddToRoot();
    Env->Initialize(World);
    UE_LOG(LogTemp, Log, TEXT("TCPEnvSubsystem: Env initialized with world '%s'"), *World->GetName());
}

void UTCPEnvSubsystem::ListenerThreadFunc()
{
    ISocketSubsystem* Subsys = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
    ListenSocket = Subsys->CreateSocket(NAME_Stream, TEXT("EnvListener"), false);
    ListenSocket->SetReuseAddr(true);
    ListenSocket->SetNonBlocking(true);

    TSharedRef<FInternetAddr> Addr = Subsys->CreateInternetAddr();
    bool bValid = false;
    Addr->SetIp(TEXT("127.0.0.1"), bValid);
    Addr->SetPort(Port);

    if (!bValid || !ListenSocket->Bind(*Addr) || !ListenSocket->Listen(1))
    {
        UE_LOG(LogTemp, Error, TEXT("TCPEnvSubsystem: Failed to bind/listen on port %d"), Port);
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("TCPEnvSubsystem: Listening on 127.0.0.1:%d"), Port);

    TSharedPtr<FInternetAddr> ClientAddr = Subsys->CreateInternetAddr();

    while (!bShouldStop.load())
    {
        // wait for a client
        while (!bShouldStop.load() && ClientSocket == nullptr)
        {
            bool bPending = false;
            if (ListenSocket->HasPendingConnection(bPending) && bPending)
            {
                ClientSocket = ListenSocket->Accept(*ClientAddr, TEXT("EnvClient"));
                if (ClientSocket)
                {
                    ClientSocket->SetNonBlocking(false);
                    UE_LOG(LogTemp, Log, TEXT("TCPEnvSubsystem: Client connected"));
                    break;
                }
            }
            FPlatformProcess::Sleep(0.01f);
        }

        // serve until disconnect
        while (!bShouldStop.load() && ClientSocket && ClientSocket->GetConnectionState() == SCS_Connected)
        {
            TSharedPtr<FJsonObject> Req = ReceiveJson(ClientSocket);
            if (!Req) break;

            FString Cmd = Req->GetStringField(TEXT("cmd"));
            TSharedPtr<FJsonObject> Resp = MakeShared<FJsonObject>();

            if (Cmd == TEXT("reset"))
            {
                FEvent* Sync = FPlatformProcess::GetSynchEventFromPool(true);
                FStepResult SR;
                AsyncTask(ENamedThreads::GameThread, [this, &SR, Sync]()
                    {
                        SR = Env->Reset();
                        Sync->Trigger();
                    });
                Sync->Wait();
                FPlatformProcess::ReturnSynchEventToPool(Sync);

                TArray<TSharedPtr<FJsonValue>> Arr;
                for (float v : SR.Obs) Arr.Add(MakeShared<FJsonValueNumber>(v));
                Resp->SetArrayField(TEXT("obs"), Arr);
                Resp->SetNumberField(TEXT("reward"), SR.Reward);
                Resp->SetBoolField(TEXT("done"), SR.Done);
                Resp->SetNumberField(TEXT("delta_time"), SR.DeltaTime);
            }
            else if (Cmd == TEXT("step"))
            {
                auto& JA = Req->GetArrayField(TEXT("action"));
                float p = JA[0]->AsNumber();
                float y = JA[1]->AsNumber();
                int32 f = static_cast<int32>(JA[2]->AsNumber());

                FEvent* Sync = FPlatformProcess::GetSynchEventFromPool(true);
                FStepResult SR;
                AsyncTask(ENamedThreads::GameThread, [this, p, y, f, &SR, Sync]()
                    {
                        SR = Env->Step(p, y, f);
                        Sync->Trigger();
                    });
                Sync->Wait();
                FPlatformProcess::ReturnSynchEventToPool(Sync);

                TArray<TSharedPtr<FJsonValue>> Arr;
                for (float v : SR.Obs) Arr.Add(MakeShared<FJsonValueNumber>(v));
                Resp->SetArrayField(TEXT("obs"), Arr);
                Resp->SetNumberField(TEXT("reward"), SR.Reward);
                Resp->SetBoolField(TEXT("done"), SR.Done);
                Resp->SetNumberField(TEXT("delta_time"), SR.DeltaTime);
            }
            else if (Cmd == TEXT("pause") || Cmd == TEXT("resume"))
            {
                bool bPause = (Cmd == TEXT("pause"));
                FEvent* Sync = FPlatformProcess::GetSynchEventFromPool(true);
                AsyncTask(ENamedThreads::GameThread, [bPause, Sync]()
                    {
                        for (auto& Ctx : GEngine->GetWorldContexts())
                            if (UWorld* W = Ctx.World())
                                UGameplayStatics::SetGamePaused(W, bPause);
                        Sync->Trigger();
                    });
                Sync->Wait();
                FPlatformProcess::ReturnSynchEventToPool(Sync);

                Resp->SetStringField(TEXT("status"), bPause ? TEXT("paused") : TEXT("resumed"));
            }

            SendJson(ClientSocket, Resp);
        }

        // clean up
        if (ClientSocket)
        {
            ClientSocket->Close();
            Subsys->DestroySocket(ClientSocket);
            ClientSocket = nullptr;
            UE_LOG(LogTemp, Log, TEXT("TCPEnvSubsystem: Client disconnected"));
        }
    }
}

bool UTCPEnvSubsystem::SendJson(FSocket* Socket, const TSharedPtr<FJsonObject>& JsonObj)
{
    // Serialize JSON
    FString Out;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Out);
    FJsonSerializer::Serialize(JsonObj.ToSharedRef(), Writer);
    FTCHARToUTF8 Converter(*Out);

    // Little-endian length header to match Python struct.pack('<I', …)
    uint32_t Len = Converter.Length();
    int32 Sent = 0;
    Socket->Send(reinterpret_cast<const uint8*>(&Len), sizeof(Len), Sent);

    // Payload
    Socket->Send(reinterpret_cast<const uint8*>(Converter.Get()), Len, Sent);
    return true;
}

TSharedPtr<FJsonObject> UTCPEnvSubsystem::ReceiveJson(FSocket* Socket)
{
    // Read 4-byte little-endian length
    uint32_t Len = 0;
    int32 Recv = 0;
    if (!Socket->Recv(reinterpret_cast<uint8*>(&Len), sizeof(Len), Recv) || Recv != sizeof(Len))
        return nullptr;

    // Read payload
    TArray<uint8> Buf;
    Buf.SetNumUninitialized(Len + 1);
    if (!Socket->Recv(Buf.GetData(), Len, Recv) || Recv != (int32)Len)
        return nullptr;
    Buf[Len] = 0;

    // Parse JSON
    FString JsonStr = FString(UTF8_TO_TCHAR(Buf.GetData()));
    TSharedPtr<FJsonObject> Obj;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonStr);
    if (FJsonSerializer::Deserialize(Reader, Obj))
        return Obj;

    return nullptr;
}

void UTCPEnvSubsystem::ResetEpisode()
{
    if (Env)
    {
        Env->Reset();
        UE_LOG(LogTemp, Log, TEXT("TCPEnvSubsystem: ResetEpisode called"));
    }
}
