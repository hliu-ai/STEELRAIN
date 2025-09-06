import os
import time
import numpy as np
import torch as T
from hppo_torch import PPOAgent
from ue5_game_env import UE5Env
from torch.utils.tensorboard import SummaryWriter
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import copy

# quick diagnostic before each training run
print("PyTorch version:", T.__version__)
print("CUDA available:", T.cuda.is_available())
print("CUDA version (PyTorch):", T.version.cuda)

def train(
    #setting how long training/episodes are allowed to last
    num_episodes=10000,
    max_steps_per_episode=5000, #max steps is limited by episode timeout in current implementation, this is just good practice to have in case of bugs
    update_timestep=2048,#modded to not overwrite current model.
    save_interval=25,
):
    writer = SummaryWriter(log_dir="runs/ppo_experiment")
    # To view logs: run "tensorboard --logdir=runs/ppo_experiment" in powershell when training then open up the local port 

    env = UE5Env()
    agent = PPOAgent(input_dims=[1112], env=env)

    # This section was just to grab some visuals for the video essay
    # --- NEW: log model graph on first real observation
    example_obs, _ = env.reset()  # grab a real state
    example_tensor = T.tensor(example_obs, dtype=T.float32).unsqueeze(0).to(agent.device)
    writer.add_graph(agent.actor, example_tensor)
    # --- NEW: log conv1 filters as images
    # conv1.weight shape is already (8,1,5,5), which is NCHW
    filters = agent.actor.conv1.weight.detach().cpu()
    writer.add_images("Actor/conv1_filters", filters, global_step=0)
    actor_for_trace = copy.deepcopy(agent.actor).cpu().eval()
    traced = T.jit.trace(actor_for_trace, example_tensor.cpu())
    traced.save("actor_onxx.pt")
    print("Wrote actor_onxx.pt - load this in Netron for a static graph")
    #--- print obs
    debug_obs_interval = 10000  # dump every 10000 steps, temporary measure for debugging
    os.makedirs("obs_debug", exist_ok=True)
    #--- print obs

    load_saved_models = True #set to true to use the latest policy, false for a fresh one
    saved_model_path = "tmp/ppo"
    if load_saved_models and os.path.exists(saved_model_path):
        agent.load_models(saved_model_path)
        print("Loaded saved models from", saved_model_path)
        print("Initializations successful. Training in progress...")
    else:
        print("Starting training from scratch.")

    total_steps = 0
    episode_rewards = []
    update_counter = 0

    # for fps counter
    report_interval   = 10   # seconds
    last_report_time  = time.time()
    step_counter      = 0

    for episode in range(1, num_episodes + 1):
        state, _ = env.reset()
        done = False
        episode_reward = 0
        episode_steps = 0

        while not done and episode_steps < max_steps_per_episode:
            # FPS counter
            step_counter += 1
            now = time.time()
            if now - last_report_time >= report_interval:
                fps = step_counter / (now - last_report_time)
                print(f"[PERF] RL loop running at {fps:.1f} steps/sec")
                step_counter     = 0
                last_report_time = now

            episode_steps += 1
            total_steps += 1

            action, log_prob_d, log_prob_c, value = agent.choose_action(state)
            action['discrete'] = int(action['discrete'])

            next_state, reward, done = env.step(action)

            #--- print obs
            if total_steps % debug_obs_interval == 0:
                fn = f"obs_debug/obs_{total_steps}.txt"
                periph_size = env.periph_h * env.periph_w
                grid = next_state[:periph_size].reshape(env.periph_h, env.periph_w)
                extras = next_state[periph_size:]
                with open(fn, 'w') as f:
                    for row in grid:
                        f.write(' '.join(f"{v:.0f}" for v in row) + '\n')
                    f.write('\n')
                    f.write(' '.join(f"{v:.6f}" for v in extras) + '\n')
                print(f"[DEBUG] Saved formatted obs array to {fn}")
            #--- print obs

            episode_reward += reward
            agent.buffer.store(
                state, action, log_prob_d, log_prob_c, reward, value, done
            )
            state = next_state

            if total_steps % update_timestep == 0:
                print("update_timestep hit. Learning...")
                last_value = agent.critic(
                    T.tensor(np.array([state]), dtype=T.float32)
                    .to(agent.device)
                ).item() if not done else 0
                agent.buffer.finish_trajectory(last_value)

                env.pause()
                print("Learning...")
                try:
                    agent.learn()
                finally:
                    env.resume()

                update_counter += 1
                writer.add_scalar("Update/Count", update_counter, total_steps)

        if done:
            agent.buffer.finish_trajectory(last_value=0)

        episode_rewards.append(episode_reward)
        avg_reward = np.mean(episode_rewards[-10:])
        print("Episode", episode, "Reward:", episode_reward,
              "Avg Reward:", avg_reward)

        # TensorBoard logging (unchanged) - all of these are variables captured throughout the VS project in order to figure out how to get this thing working. 
        # this is not some sort of comprehensive industry best practice, just what I thought was worth watching.
        writer.add_scalar("Reward/Episode", episode_reward, episode)
        writer.add_scalar("Reward/Average", avg_reward, episode)
        writer.add_scalar("Episode/Length", episode_steps, episode)
        writer.add_scalar("Total_Steps", total_steps, episode)
        writer.add_scalar("Update/Count", update_counter, episode)
        writer.add_scalar("Loss/Actor", agent.last_actor_loss, episode)
        writer.add_scalar("Loss/Critic", agent.last_critic_loss, episode)
        writer.add_scalar("Loss/Total", agent.last_total_loss, episode)
        writer.add_scalar("LogProb/Discrete", agent.last_discrete_log_prob, episode)
        writer.add_scalar("LogProb/Continuous", agent.last_continuous_log_prob, episode)
        writer.add_scalar("Entropy/Discrete", agent.last_discrete_entropy, episode)
        writer.add_scalar("Entropy/Continuous", agent.last_continuous_entropy, episode)
        writer.add_scalar("KL/Discrete", agent.last_kl_d, episode)
        writer.add_scalar("KL/Continuous", agent.last_kl_c, episode)
        writer.add_scalar("GradNorm/Actor", agent.last_actor_grad_norm, episode)
        writer.add_scalar("GradNorm/Critic", agent.last_critic_grad_norm, episode)
        writer.add_scalar("ValueError", agent.last_value_error, episode)
        writer.add_scalar("Sigma/Mean", agent.last_sigma, episode)

        if agent.buffer.advantages is not None:
            writer.add_scalar(
                "Buffer/Advantage_Mean", np.mean(agent.buffer.advantages), episode
            )
            writer.add_scalar(
                "Buffer/Return_Mean", np.mean(agent.buffer.returns), episode
            )

        if episode % save_interval == 0:
            save_path = "tmp/ppo"
            os.makedirs(save_path, exist_ok=True)
            agent.save_models(save_path)
            print("Saved models at episode", episode)

    env.close()
    writer.close()
    print("Training complete!")
    print("Episode rewards:", episode_rewards)


if __name__ == "__main__":
    train()
    input("Press Enter to exit...")
