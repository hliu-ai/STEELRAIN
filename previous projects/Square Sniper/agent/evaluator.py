# evaluator.py

import numpy as np
import torch as T

from ppo_torch import PPOAgent
from sfml_game_env import SFMLGameEnv

# Configuration 
MODEL_PATH   = "tmp/ppo"   # where actor.pth & critic.pth live
NUM_EPISODES = 10          # how many full runs to average over
MAX_STEPS    = 500         # cap per episode
# ---

def choose_deterministic_action(agent, state):
    """
    Return:
      - continuous: the policy mean (no sampling)
      - discrete:   the argmax of the discrete logits
    """
    state_t = T.tensor(np.array(state), dtype=T.float32).unsqueeze(0).to(agent.device)
    cont_dist = agent.actor.get_continuous_distribution(state_t)
    disc_dist = agent.actor.get_discrete_distribution(state_t)

    # detach before numpy()
    cont_action = cont_dist.mean.cpu().detach().numpy()[0]
    logits      = disc_dist.logits.cpu().detach().numpy()[0]
    disc_action = int(np.argmax(logits))

    return {'continuous': cont_action, 'discrete': disc_action}

def evaluate(agent, env):
    rewards = []
    for ep in range(1, NUM_EPISODES + 1):
        state     = env.reset()
        ep_reward = 0.0

        for _ in range(MAX_STEPS):
            action = choose_deterministic_action(agent, state)
            state, reward, done, _ = env.step(action)
            ep_reward += reward
            if done:
                break

        rewards.append(ep_reward)
        print(f"Episode {ep:2d} -> Reward: {ep_reward:.2f}")

    avg = np.mean(rewards)
    print(f"\nAverage over {NUM_EPISODES} episodes: {avg:.2f}")

if __name__ == '__main__':
    env   = SFMLGameEnv()
    agent = PPOAgent(input_dims=[405], env=env)

    agent.load_models(MODEL_PATH)
    print(f"Loaded models from '{MODEL_PATH}'\n")

    evaluate(agent, env)
    env.close()
