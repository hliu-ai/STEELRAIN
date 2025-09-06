# ue5_game_env.py
# delta_time unlocks a frame-invariant, non-throttling synchronization and communication between the learning loop and the environment loop

import time
import os
import numpy as np
import gymnasium as gym
from gymnasium import spaces
from socket_client import UE5SocketClient

# Use Agg backend so rendering doesn’t pop up windows
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt


class UE5Env(gym.Env):
    def __init__(
        self,
        host="127.0.0.1",
        port=7777,
        pitch_range=(-30, 30),
        yaw_range=(-30, 30),
        periph_h=27,
        periph_w=41,
        visualization_interval=1,
    ):
        super().__init__()
        self.periph_h = periph_h
        self.periph_w = periph_w

        self.visualization_interval = max(1, visualization_interval)
        self._step_counter = 0
        self._last_obs = None
        # Initialize a default dt of 1/60s
        self._last_dt = 1.0 / 60.0

        self.pitch_low, self.pitch_high = pitch_range
        self.yaw_low,   self.yaw_high   = yaw_range

        self.client = UE5SocketClient(host=host, port=port)
        time.sleep(0.5)

        # Warm up and grab an initial dt (optional)
        obs, _, _, new_dt = self.client.reset()
        if new_dt > 0.0:
            self._last_dt = new_dt
        obs = np.array(obs, dtype=np.float32)

        # --- Begin minimal change: define correct observation_space bounds ---
        periph_size = self.periph_h * self.periph_w
        low = np.array(
            [0.0] * periph_size
            + [0.0, 0.0, 0.0, -1.0, 0.0],  # NormPitch, NormYaw, NormDist, SignedAngle, Overlap
            dtype=np.float32
        )
        high = np.array(
            [1.0] * periph_size
            + [1.0, 1.0, 1.0,  1.0, 1.0],
            dtype=np.float32
        )
        self.observation_space = spaces.Box(low=low, high=high, dtype=np.float32)
        # --- End minimal change ---

        self.action_space = spaces.Dict({
            "continuous": spaces.Box(
                low=np.array([self.pitch_low, self.yaw_low], dtype=np.float32),
                high=np.array([self.pitch_high, self.yaw_high], dtype=np.float32),
                shape=(2,), dtype=np.float32
            ),
            "discrete": spaces.Discrete(2),
        })

    def reset(self, *, seed=None, options=None):
        super().reset(seed=seed)
        obs, _, done, new_dt = self.client.reset()
        if new_dt > 0.0:
            self._last_dt = new_dt
        obs = np.array(obs, dtype=np.float32)

        self._step_counter = 0
        self._last_obs = obs.copy()

        # Gymnasium reset signature: obs, info
        return obs, {}

    def step(self, action):
        #  clamp & unpack 
        cont = np.array(action["continuous"], dtype=np.float32)
        pitch_rate = float(np.clip(cont[0], self.pitch_low, self.pitch_high))
        yaw_rate   = float(np.clip(cont[1], self.yaw_low,   self.yaw_high))
        fire_flag  = int(action["discrete"])

        # compute per-tick delta angles
        exec_pitch = pitch_rate * self._last_dt
        exec_yaw   = yaw_rate   * self._last_dt

        #  SINGLE RPC call 
        obs, reward, done, new_dt = self.client.step(exec_pitch, exec_yaw, fire_flag)
        self._last_dt = new_dt

        #throttle to ue5 tickrate
        time.sleep(self._last_dt)

        #  standard post-processing 
        obs = np.array(obs, dtype=np.float32)
        self._step_counter += 1
        self._last_obs = obs.copy()

        return obs, reward, done


    def render(self):
        if (self._step_counter % self.visualization_interval) != 0:
            return

        periph_size = self.periph_h * self.periph_w
        periph = self._last_obs[:periph_size].reshape(self.periph_h, self.periph_w)
        extras = self._last_obs[periph_size:]
        normpitch, normyaw, normdist, cosine, overlap = extras

        print(f"[UE5Env] Step {self._step_counter} Extras -> "
              f"Pitch: {normpitch:.3f}, Yaw: {normyaw:.3f}, "
              f"NormDist: {normdist:.3f}, Cosine: {cosine:.3f}, "
              f"Overlap: {overlap:.1f}")

        plt.figure(figsize=(5, 5))
        plt.title(f"Peripheral Grid @ step {self._step_counter}")
        plt.imshow(periph, origin='upper', interpolation='nearest')
        plt.colorbar(shrink=0.7)
        plt.tight_layout()

        os.makedirs("visuals", exist_ok=True)
        fn = f"visuals/obs_{self._step_counter}.png"
        plt.savefig(fn)
        plt.close()
        print(f"[UE5Env] Saved peripheral heatmap to {fn}")

    def close(self):
        self.client.close()

    def pause(self):
        self.client.pause()

    def resume(self):
        self.client.resume()
