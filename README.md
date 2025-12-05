# Portal Game in C++ (SDL Renderer)

A recreation of core Portal mechanics written entirely in C++ using SDL as the graphics and input framework.  
The player can walk, look around, fire two linked portals, and move through them while preserving orientation and velocity.  
This project was built from scratch for a university course and later refined into a standalone portfolio piece.

---

## Features

- First person movement and mouse camera control  
- Shoot and place two portals (blue and orange)  
- Teleportation between portals  
- Movement direction and momentum carries between portal surfaces  
- Basic collision against walls and surfaces  
- Simple puzzle style rooms to navigate

---

## Tech and Tools

| Component | Details |
|---|---|
| Language | C++17 |
| Rendering API | SDL Renderer |
| Physics | Custom collision and movement handling |
| Portal system | Linked teleport transforms with velocity preservation |
| Architecture | Manual game loop (input, update, render)

---

## How the Portal System Works

1. The game tracks positions and orientations of Portal A and Portal B  
2. When the player intersects Portal A, they are teleported to Portal B  
3. Velocity is rotated to match the new exit orientation  
4. Rendering updates immediately to respect the new camera transform  

This included challenges with collision order, rotation alignment, and camera resets.  
Solving these issues taught me a lot about simulation stability and debugging complexity.

---

## How to Build and Run

1. Clone this repository  
2. Install SDL2  
3. Build with your C++ compiler or preferred IDE  
4. Run the executable  
5. Left and right mouse to fire portals, move through them to advance rooms

Platform specific build instructions can be added later if needed.

---

## What I Learned

- Building a low level game loop without a game engine  
- Handling SDL input, rendering, and basic math transformations  
- Implementing portal teleportation with orientation and motion preservation  
- Debugging collision systems and camera movement manually  
- Breaking larger gameplay ideas into smaller mechanical systems

This was one of the most challenging and rewarding projects I have worked on.

---

## Possible Future Improvements

- Through portal rendering shader for visual accuracy  
- More puzzle chambers and progression  
- Improved movement physics and smoothing  
- Interaction with objects and physics through portals  
- Sound, UI, and visual polish

---

## Gameplay Preview

<img src="Media/portal_jump.gif" width="600">


