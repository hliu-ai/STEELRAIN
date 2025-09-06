import gym
from gym import spaces
import numpy as np
import sys, os
build_folder = r"C:\VS Projects\PPO_SFML\sfml_game_build_files"
if build_folder not in sys.path:
    sys.path.insert(0, build_folder)

import pybind_sfml_game

class SFMLGameEnv(gym.Env):
    """
    Gymnasium wrapper for the SFML target shooting game with hybrid action space.
    The action is now a dictionary:
      - 'continuous': Box for [delta_x, delta_y] in [-1, 1]
      - 'discrete': Discrete(2) for shooting (0 = no shoot, 1 = shoot)
    """
    def __init__(self):
        super(SFMLGameEnv, self).__init__()
        
        self.game = pybind_sfml_game.SFMLGame()

        self.game.set_agent_mode(True) #CHANGE TO FALSE FOR HUMAN MODE TO DEBUG

        # Observation space is now 405 floats:
        #   • first five: [0,1], [0,1], [0,1], [-1,1], [0,1]
        #   • remaining 400: binary {0,1}
        low  = np.array([0., 0., 0., -1., 0.] + [0.]*400, dtype=np.float32) #lowest values they can be (note that having -1 for angle is intentional - sign can contribute to teaching direction.)
        high = np.array([1., 1., 1.,  1., 1.] + [1.]*400, dtype=np.float32) # highest values they can be
        self.observation_space = spaces.Box(low=low, high=high, dtype=np.float32)
        
        # Define a hybrid action space as a Dict:
        self.action_space = spaces.Dict({
            'continuous': spaces.Box(low=-1.0, high=1.0, shape=(2,), dtype=np.float32),
            'discrete': spaces.Discrete(2)
        })
    
    def reset(self):
        state = self.game.reset()
        return np.array(state, dtype=np.float32)
    
    def step(self, action):
        # Extract the two parts of the action.
        cont_action = action['continuous']
        disc_action = action['discrete']
        delta_x, delta_y = cont_action
        
        # Create the C++ Action object with the discrete part.
        act = pybind_sfml_game.Action(delta_x, delta_y, disc_action)
        state, reward, done = self.game.step(act)
        return np.array(state, dtype=np.float32), reward, done, {}
    
    def render(self, mode='human'):
        pass

    def close(self):
        pass
