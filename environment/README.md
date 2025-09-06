# Environment

Within this folder are the core C++ files I want to share from my Unreal Engine 5.5 Visual Studio project.  

Please note: this isn’t meant to be replicated as-is — it’s mainly me sharing the final state of some of the important logic. The code itself won’t be of much use on its own. For an actual explanation, I highly recommend checking out my video.

---

## File Overview

- **UE5Game**  
  The main API that the agent interacts with. It includes all of my managers and cleanly defines `Reset` and `Step`.  

- **TCPEnvSubsystem**  
  Handles the UE5-side socketing logic, which uses JSON. It’s called a *subsystem* not for style, but because that’s the actual Unreal Engine object type.  

- **APeripheralPyramid**  
  Evolved from my raycasting experiments in UE5 (foveal vision, custom ray-casting logic, etc). Defines how many rays are cast and how far apart they are. Named *Peripheral* since it represents peripheral vision, and *Pyramid* because it projects a rectangle of rays, forming a rectangular-based pyramid with the origin as its tip.  

- **AObservationManager**  
  Manages observations. The observation tensor is composed of a target-flag grid (dimensions defined in `PeripheralPyramid`) plus 5 positional scalars. Full details are explained in the video.  

- **ARewardManager**  
  Calculates all rewards each tick and provides the net reward — ultimately the key feedback the agent learns from. Explained in-depth in the video.  

- **ADoneManager**  
  A smaller component that ensures every terminal state is properly flagged.  
