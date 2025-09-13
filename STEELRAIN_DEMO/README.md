# 🎮 STEELRAIN Demo

This folder documents the packaged demo release of **STEELRAIN**, my reinforcement learning framework built with Unreal Engine 5.5 and PyTorch.  

The demo allows most people with a Windows 10 or newer machine to run my pre-trained RL agent in a UE5 environment without needing to build from source or install Python — close to what you see in the video!  

All you need to do is run two executables in order. Details below.  

---

## 📥 Download the Demo

The demo binaries are not stored in this folder.  
Instead, grab the latest release here:  

👉 [Go to the Release Page](https://github.com/hliu-ai/STEELRAIN/releases/tag/v.1.0.0)  

After downloading, **extract the .zip** somewhere (e.g., Desktop). Inside you’ll see two folders:  

- **Environment (Run First)** — contains `SteelRain_H.exe` (the UE5 packaged environment) along with the other files as part of the UE5 package - feel free to ignore them.  
- **Agent** — contains `Run_Agent.bat` and `SteelRain_Agent_CPU.exe`.  

---

## ▶️ How to Run the Demo

1. Run `Environment/SteelRain_H.exe`.  
   - This will start the (playable) Unreal Engine environment.  
   - You may be prompted to install **Visual C++ runtime** and **DirectX** as part of UE5 setup. This is completely normal.  
   - You can manually aim/shoot with the mouse if you'd like. The first episode is set very long to accommodate agent startup, but the agent will take over once launched.
   - Note that you technically CAN use WASD to move, but the demo is only designed to work with running Run_Agent.bat from the original starting position.

2. Run `Agent/Run_Agent.bat`.  
   - This launches the pre-trained agent.  
   - The agent will automatically connect to the environment and run for **10 episodes**, printing cumulative rewards and performance stats.  Each episode ends when all 30 shots are fired (environment will reset) or the 15 seconds cap has transpired (which should not happen because this is pre-trained).
   - Stop early any time with **Ctrl+C**.  
   - Note that it will take a minute (depending on your hardware) to load all the dependencies. The .bat will leave you with a reassuring message that loading is occuring. This is completely normal - it takes about half a minute to start on my system. 

---

## ⚙️ Requirements

- Windows 10 or newer (x64)  
- Visual C++ Redistributable (prompted if missing)  
- DirectX Runtime (prompted if missing)  
- If your PC is in **Windows S Mode**: disable S Mode to run non–Microsoft Store apps.  
- ~2GB free disk space  

**Hardware notes:**  
- This demo is **CPU-only** (no GPU required).  
- Hard-capped at **60 FPS** for fairness across machines.  
- On weaker systems, expect slower frame rates — the agent logic still works, but on-screen results may degrade significantly if FPS drops below ~15.  

---

## 🔍 Notes & Known Issues

- **Security concern:** I get that running `.exe` files can feel risky! These binaries are built directly from Unreal Engine and PyInstaller, and the full source code is here in the repo. If you’d rather not run them, that’s totally fine — the video essay shows the same behavior.

- **Startup delay:** The agent may take a moment to initialize (loading PyTorch, numpy, etc.). The console will immediately explain what’s happening.  

- **Performance:** Unreal Engine is resource-intensive. On my system (see specs below) it runs 144+ FPS uncapped, but on my dad’s old laptop it ran at 3–5 FPS. It still worked — just much slower.  

- **Exit order:** Close the Agent first (Ctrl+C or closing the terminal). However, if you close the Environment first, the Agent should still gracefully exit with a connection error.  

**My hardware (for reference):**  
- Intel i5-13600K  
- NVIDIA 4070 Ti Super (16GB)  
- 32GB DDR5 RAM  
- Z790 AORUS ELITE AX ICE  

---

## 🖥️ Development Context

- Environment built with **Unreal Engine 5.5**.  
- Agent built with **PyTorch (CPU runtime)**, packaged via PyInstaller.  
- Policy weights are **baked into the agent executable** — no external `.pth` file needed.  

---

## 📜 License

This demo is distributed for educational and demonstration purposes.  
See the full [LICENSE](https://github.com/hliu-ai/STEELRAIN/blob/main/LICENSE) file in the root repository.  

---

✨ That’s it — unzip, run Environment first, then Agent, and watch it go! 
