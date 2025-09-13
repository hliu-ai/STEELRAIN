# STEELRAIN

🚀 **[Try the Demo Here!](https://github.com/hliu-ai/STEELRAIN/tree/main/STEELRAIN_DEMO)**  

This repository contains relevant source code discussed in the video, additional documentation, and a two-click packaged demo for **STEELRAIN** — a reinforcement learning framework integrating Unreal Engine 5.5 (C++) with a PyTorch agent.  

---

"Patch" Notes:

9/13 Midnight Release Update - Proud to announce that STEELRAIN_DEMO_v.1.0.0 can now be found under releases! Please refer to STEELRAIN_DEMO for documentation on how to run and what to expect. It was designed to run on most any Windows machine (no GPU required, fps capped). Thank you very much to my commentors on Reddit and friends who highlighted the importance of having something that can be downloaded and played.

9/12 Update - Thank you for your comments on how to improve this repo. My top priority right now is producing a demo build that you can download and run on your own PC. Then maybe I can finally sucker some of you into actually watching the video... standby! This will be done by tonight/tomorrow.

9/11 Video went live. 

---

A modular reinforcement learning framework integrating **Unreal Engine 5.5 (C++)**, **PyTorch + CUDA**, and a **hybrid-action Proximal Policy Optimization (PPO)** algorithm.
I use TCP Socketing with JSON to achieve frame-invariant, non-throttling synchronization between agent and envirionment by leveraging the time elapsed between engine ticks.
The system trains a virtual ground-to-air turret to intercept dynamic targets in a 3D, physics-driven simulated environment.

---

## 🎥 Video Essay (Runtime: 2h 51m)

For additional information, please consider watching my comprehensive video essay, where I detail my development journey, core RL concepts, and thoughts on the future of this field. Visio diagrams for overview of architectural decisions included.

If you have questions about (what this even is) my approach or solutions, I highly recommend checking the relevant portion (timestamps are available in the description) before reaching out — I’d love to chat.  

👉 **[Watch the Video Essay](https://www.youtube.com/watch?v=tdVDrrg8ArQ)** 👈  

---

## ⚙️ Tech Stack

### Agent
- **Python**
- **PyTorch** (CUDA Accelerated)  
  > Note: I experimented with LibTorch for previous projects, but had to "frankenstein" files from three CUDA versions. I recommend simply coding agents with PyTorch and leaving the performance-optimized C++ work for your simulations.  
- **TensorBoard** (metrics tracking — highly recommended, much better than coding your own terminal and graphs)  
- **Gymnasium**

### Environment
- **C++**
- **Unreal Engine 5.5**  
  (UE 5.6 was released recently, but I recommend 5.5 or even 5.4 for now, since free assets take time to catch up.)  
- **UE5 Native Blueprint Visual Scripting** (for game logic)  
- **TCP (JSON)**

### Previous Projects
- **C++**
- **SFML**
- **Python**
- **LibTorch** (CUDA Accelerated)
- **Pybind11** 
- **CMake** (Never again...)

### Additional Tools
- **arXiv** (research papers on cutting-edge RL)  
- **Visual Studio** (my current IDE of choice, and default for UE5)
- **Visio** (for algorithm visualization and project management)  
- **Adobe Creative Suite** (for content creation)  
