# 3D Fluid Simulation
Na razie w początkowej fazie próba zbudowania symulacji cieczy szczególnie kładąc nacisk na zjawisko [kawitacji](https://pl.wikipedia.org/wiki/Kawitacja). Jak na razie udało mi się stworzyć podstawowy program symulacji mogący załadować plik modelu 3D o rozszerzeniu .obj i wyrenderowanie go w oknie.

Jak na razie w porjekcie zostały użyte następujace biblioteki:
- ```Glad``` (Nagłówki OpenGL)
- ```GLM```  (Matematyka OpenGL)
- ```SDL3``` (Zarządzanie wejściem/wyjściem oraz tworzenie okna)
- ```tiny_object_loader``` (Załadowanie plików modeli 3D o rozszerzeniu .obj)

## Phase 1 — Rozwiązywanie

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

 1. Load the mesh (Assimp or tinyobjloader).
 2. Mark grid cells as solid.
 3. Enforce no-penetration at propeller surfaces by setting normal velocity equal to the propeller’s local surface velocity.

**TEST** Flow moves around the spinning propeller with plausible vortices.

## Phase 3 — Cavitation

 1. Add arrays for:
    
    - pressure field (p)
    - vapor fraction `alpha`
      
 3. Compute local minima of pressure -> that’s where cavitation 'could' occur.
 4. Color low-pressure regions differently to preview where bubbles might form.

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
