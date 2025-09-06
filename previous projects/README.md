# Previous Projects

This folder contains source code from some of my earlier projects. These represent important stepping stones in my journey toward **STEELRAIN**.  

---

## Notes
- The code here — especially in **Banana Blitz** — is rough. They’re basically cave drawings, but they’re mine, and they mark the first steps toward this career.  
- Like the rest of the repo, the priority is readability, not reproducibility. Only the most relevant source files are included.  
- For the full story and deeper insights on these projects, please see the video essay.  
- If you have questions of any kind, feel free to reach out!  

---

## Banana Blitz (BB)

My very first game-dev / RL endeavor — and as you’ll see in the video and the file structure... that fact really comes through.  

- Meant to represent a simple "flash game" of the sort I played growing up
- Built as a single project with **C++** and **LibTorch**  
- **SFML** used for graphics (kept extremely simple)  
- Demonstrates the near-limit of what’s possible with **DQN**, since BB is far more complex than Frozen Lake  
- Required a custom **Noisy-Double-DQN** algorithm to succeed  

---

## Square Sniper (SS)

The intermediate project between Banana Blitz and STEELRAIN, where I first tackled **continuous and then hybrid action spaces**.  

The SS environment is coded in **C++** and built in an independent VS project. Build files were saved within the agent project. The agent itself uses **Python**, and I connected the two via **Pybind11**. While nifty, I wouldn’t recommend Pybind11 for large-scale RL applications, as I quickly ran into limitations with STEELRAIN. **TCP socketing** is simply much faster.  

- The big idea is that shooting at a target in 3D can be simplified by imagining a 2D plane perpendicular to the agent’s facing direction — in this way, SS is essentially a low-res "first-person" shooter prototype  
- Explored **Soft Actor-Critic (SAC)**, extended it to hybrid-action, then swapped to **PPO** (entropy tuning proved difficult for this case)  
- Served as a 2D testbed for designing reward and observation systems for a future 3D shooting agent  


---

Much more detail about both projects is covered in the video essay, but this folder keeps a snapshot of the raw source for reference.  
