# STEELRAIN

A modular reinforcement learning framework integrating **Unreal Engine 5.5 (C++)**, **PyTorch + CUDA**, and a **hybrid-action Proximal Policy Optimization (PPO)** algorithm.  
The system trains a virtual ground-to-air agent to intercept dynamic targets in a 3D, physics-driven simulated environment.

---

## 🎥 Video Essay (Runtime: 2h 51m)

Please consider watching my comprehensive video essay, where I detail my development journey, core RL concepts, and thoughts on the future of this field.  

If you have questions about my approach or solutions, I highly recommend checking the relevant portion (timestamps are available in the description) before reaching out — I’d love to chat.  

On a personal note: I am currently seeking work in creating autonomous defense systems (North America only), and this video serves a dual function as a "technical resume" of sorts. 

👉 **[Watch the Video Essay](PLACEHOLDER_LINK)** 👈  

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
