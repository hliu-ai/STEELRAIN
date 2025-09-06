# Agent

Within this folder is some of the core Python source code I determined to be most relevant.
All of these files are part of the Visual Studio project that contains my PyTorch agent. There is only Python here, the C++ and JSON stuff is environment-side only.
On my PC, there's a bunch of miscellaneous stuff for minimal examples and smoke tests, which I have omitted for brevity.

If you have any questions about this stuff, reach out and I'll try and explain my decision making more in-depth, but I suggest watching the video first! Thanks.

---

## File Overview

- **main.py**  
  The launchpad for the agent. This should be run **after** the simulation is running, as the agent connects to the listening port on the UE5 side.  

- **hppo_torch.py**  
  Contains the main PPO algorithm logic, including, critically, the `learn` step.  

- **buffer.py**  
  Houses the replay buffer and its methods.  

- **networks.py**  
  Defines the Actor and Critic networks as well as their respective methods.  

- **socket_client.py**  
  Handles the agent-side networking.  

- **ue5_game_env.py**  
  Provides a Gymnasium wrapper and uses the `delta_time` value returned from the environment to perform real-time calculations for frame-invariant, non-throttling synchronization.  
