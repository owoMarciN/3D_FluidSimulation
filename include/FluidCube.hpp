#ifndef FLUID_CUBE_H_
#define FLUID_CUBE_H_

#include <cmath>
#define IX(x, y, z) ((x) + (y) * N + (z) * N * N)

class FluidCube {
    public:
        int size;
        float dt;
        int iter;
        float diff;
        float visc;
        
        float *s;
        float *density;
        
        float *Vx;
        float *Vy;
        float *Vz;

        float *Vx0;
        float *Vy0;
        float *Vz0;

        FluidCube(int size, float dt, int iter, float diffusion, float viscosity);

        ~FluidCube();

        void AddDensity(int x, int y, int z, float amount);

        void AddVelocity(int x, int y, int z, float amountX, float amountY, float amountZ);

        void set_bounds(int b, float *x);

        void lin_solve(int b, float *x, float *x0, float a, float c);

        void diffuse(int b, float *x, float *x0, float diff, float dt);

        void project(float *velX, float *velY, float *velZ, float *p, float *div);

        void advect(int b, float *d, float *d0, float *velX, float *velY, float *velZ, float dt);
            
        void FluidCubeStep();
        
        void fadeDensity(float amount);
};



#endif