# 3D_FluidSimulation
3D fluid simulation using SDL3 + OpenGL libraries

## Phase 1 — Core fluid solver (grid-based “Stable Fluids”)

Goal: get a 3D incompressible flow running, visualize it

### Steps

1. Set up the grid in 3D

    - Generate basic grid structure
    - Allocate arrays for velocities (x, y, z), previous time step velocities(x0, y0, z0), density and previous density.

2. Translate the 2D setup to 3D

3. Add forces additional forces
    
    - Gravity, propeller body force, or external field.

5. Final visualization steps

    - Render velocity magnitude, streamlines, or dye density.
    - Verify things stay bounded and flow behaves plausibly.


## Phase 2 — Propeller interaction

    - Load the mesh (Assimp or tinyobjloader).
    - Mark grid cells as solid.
    - Enforce no-penetration at propeller surfaces by setting normal velocity equal to the propeller’s local surface velocity.

**TEST** Flow moves around the spinning propeller with plausible vortices.


## Phase 3 — Cavitation

    - Add arrays for: 
        - pressure field (p)
        - vapor fraction `alpha`
    - Compute local minima of pressure -> that’s where cavitation 'could' occur.
    - Color low-pressure regions differently to preview where bubbles might form.

**TEST** Visual low-pressure zones appear behind propeller blades.


## Phase 4 — Rayleigh–Plesset & particles

1. Implement a 'Particle' or 'Bubble' class with:

   ```cpp
   struct Particle {
       Vec3 pos;
       double R, V;
       void RP(double p_liquid, double dt);
   };
   ```
2. Integrate RP using implicit Euler or other integrating method
3. Interpolate pressure from grid -> particle.

**TEST** Bubbles grow/shrink realistically in low-pressure zones.

## Phase 5 — Bubbles *affecting* the flow.

1. From particle volumes compute per-cell void fraction α.
2. Modify mixture density
3. Verify conservation and stability.

