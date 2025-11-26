#include "Fluid.h"

Fluid::Fluid(int size, float dt, int iter, float diffusion, float viscosity) {
    int N = size;
    
    this->size = size;
    this->dt = dt;
    this->iter = iter;
    this->diff = diffusion;
    this->visc = viscosity;
    
    int total_size = N * N * N;
    this->s = new float[total_size];
    this->density = new float[total_size];
    
    this->Vx = new float[total_size];
    this->Vy = new float[total_size];
    this->Vz = new float[total_size];
    
    this->Vx0 = new float[total_size];
    this->Vy0 = new float[total_size];
    this->Vz0 = new float[total_size];
}

Fluid::~Fluid() {
    delete[] s;
    delete[] density;

    delete[] Vx;
    delete[] Vy;
    delete[] Vz;

    delete[] Vx0;
    delete[] Vy0;
    delete[] Vz0;
}

void Fluid::set_bounds(int b, float *x) {
    int N = this->size;
    for(int j = 1; j < N - 1; j++) {
        for(int i = 1; i < N - 1; i++) {
            x[IX(i, j, 0  )] = b == 3 ? -x[IX(i, j, 1  )] : x[IX(i, j, 1  )];
            x[IX(i, j, N-1)] = b == 3 ? -x[IX(i, j, N-2)] : x[IX(i, j, N-2)];
        }
    }
    for(int k = 1; k < N - 1; k++) {
        for(int i = 1; i < N - 1; i++) {
            x[IX(i, 0  , k)] = b == 2 ? -x[IX(i, 1  , k)] : x[IX(i, 1  , k)];
            x[IX(i, N-1, k)] = b == 2 ? -x[IX(i, N-2, k)] : x[IX(i, N-2, k)];
        }
    }
    for(int k = 1; k < N - 1; k++) {
        for(int j = 1; j < N - 1; j++) {
            x[IX(0  , j, k)] = b == 1 ? -x[IX(1  , j, k)] : x[IX(1  , j, k)];
            x[IX(N-1, j, k)] = b == 1 ? -x[IX(N-2, j, k)] : x[IX(N-2, j, k)];
        }
    }
    
    x[IX(0, 0, 0)]       = 0.33f * (x[IX(1, 0, 0)]
                                  + x[IX(0, 1, 0)]
                                  + x[IX(0, 0, 1)]);
    x[IX(0, N-1, 0)]     = 0.33f * (x[IX(1, N-1, 0)]
                                  + x[IX(0, N-2, 0)]
                                  + x[IX(0, N-1, 1)]);
    x[IX(0, 0, N-1)]     = 0.33f * (x[IX(1, 0, N-1)]
                                  + x[IX(0, 1, N-1)]
                                  + x[IX(0, 0, N)]);
    x[IX(0, N-1, N-1)]   = 0.33f * (x[IX(1, N-1, N-1)]
                                  + x[IX(0, N-2, N-1)]
                                  + x[IX(0, N-1, N-2)]);
    x[IX(N-1, 0, 0)]     = 0.33f * (x[IX(N-2, 0, 0)]
                                  + x[IX(N-1, 1, 0)]
                                  + x[IX(N-1, 0, 1)]);
    x[IX(N-1, N-1, 0)]   = 0.33f * (x[IX(N-2, N-1, 0)]
                                  + x[IX(N-1, N-2, 0)]
                                  + x[IX(N-1, N-1, 1)]);
    x[IX(N-1, 0, N-1)]   = 0.33f * (x[IX(N-2, 0, N-1)]
                                  + x[IX(N-1, 1, N-1)]
                                  + x[IX(N-1, 0, N-2)]);
    x[IX(N-1, N-1, N-1)] = 0.33f * (x[IX(N-2, N-1, N-1)]
                                  + x[IX(N-1, N-2, N-1)]
                                  + x[IX(N-1, N-1, N-2)]);
}

void Fluid::lin_solve(int b, float *x, float *x0, float a, float c) {
    int N = this->size;

    float cRecip = 1.0f / c;
    for (int k = 0; k < this->iter; k++) {
        for (int m = 1; m < N - 1; m++) {
            for (int j = 1; j < N - 1; j++) {
                for (int i = 1; i < N - 1; i++) {
                    x[IX(i, j, m)] =
                        (x0[IX(i, j, m)]
                            + a*(    x[IX(i+1, j  , m  )]
                                    +x[IX(i-1, j  , m  )]
                                    +x[IX(i  , j+1, m  )]
                                    +x[IX(i  , j-1, m  )]
                                    +x[IX(i  , j  , m+1)]
                                    +x[IX(i  , j  , m-1)]
                           )) * cRecip;
                }
            }
        }
        set_bounds(b, x);
    }
}

void Fluid::diffuse(int b, float *x, float *x0, float diff, float dt) {
    int N = this->size;
    float a = dt * diff * (N - 2) * (N - 2);
    this->lin_solve(b, x, x0, a, 1 + 6 * a);
}

void Fluid::advect(int b, float *d, float *d0,  float *velocX, float *velocY, float *velocZ, float dt) {
    int N = this->size;
    float i0, i1, j0, j1, k0, k1;
    
    float dtx = dt * (N - 2);
    float dty = dt * (N - 2);
    float dtz = dt * (N - 2);
    
    float s0, s1, t0, t1, u0, u1;
    float tmp1, tmp2, tmp3, x, y, z;
    
    float Nfloat = N;
    float ifloat, jfloat, kfloat;
    int i, j, k;
    
    for(k = 1, kfloat = 1; k < N - 1; k++, kfloat++) {
        for(j = 1, jfloat = 1; j < N - 1; j++, jfloat++) { 
            for(i = 1, ifloat = 1; i < N - 1; i++, ifloat++) {
                tmp1 = dtx * velocX[IX(i, j, k)];
                tmp2 = dty * velocY[IX(i, j, k)];
                tmp3 = dtz * velocZ[IX(i, j, k)];
                x    = ifloat - tmp1; 
                y    = jfloat - tmp2;
                z    = kfloat - tmp3;
                
                if(x < 0.5f) x = 0.5f; 
                if(x > Nfloat + 0.5f) x = Nfloat + 0.5f; 
                i0 = floorf(x); 
                i1 = i0 + 1.0f;
                if(y < 0.5f) y = 0.5f; 
                if(y > Nfloat + 0.5f) y = Nfloat + 0.5f; 
                j0 = floorf(y);
                j1 = j0 + 1.0f; 
                if(z < 0.5f) z = 0.5f;
                if(z > Nfloat + 0.5f) z = Nfloat + 0.5f;
                k0 = floorf(z);
                k1 = k0 + 1.0f;
                
                s1 = x - i0; 
                s0 = 1.0f - s1; 
                t1 = y - j0; 
                t0 = 1.0f - t1;
                u1 = z - k0;
                u0 = 1.0f - u1;
                
                int i0i = i0;
                int i1i = i1;
                int j0i = j0;
                int j1i = j1;
                int k0i = k0;
                int k1i = k1;
                
                d[IX(i, j, k)] = 
                
                    s0 * ( t0 * (u0 * d0[IX(i0i, j0i, k0i)]
                                +u1 * d0[IX(i0i, j0i, k1i)])
                        +( t1 * (u0 * d0[IX(i0i, j1i, k0i)]
                                +u1 * d0[IX(i0i, j1i, k1i)])))
                   +s1 * ( t0 * (u0 * d0[IX(i1i, j0i, k0i)]
                                +u1 * d0[IX(i1i, j0i, k1i)])
                        +( t1 * (u0 * d0[IX(i1i, j1i, k0i)]
                                +u1 * d0[IX(i1i, j1i, k1i)])));
            }
        }
    }
    set_bounds(b, d);
}

void  Fluid::project(float *velX, float *velY, float *velZ, float *p, float *div) {
    int N = this->size;
    for (int k = 1; k < N - 1; k++) {
        for (int j = 1; j < N - 1; j++) {
            for (int i = 1; i < N - 1; i++) {
                div[IX(i, j, k)] = -0.5f*(
                         velX[IX(i+1, j  , k  )]
                        -velX[IX(i-1, j  , k  )]
                        +velY[IX(i  , j+1, k  )]
                        -velY[IX(i  , j-1, k  )]
                        +velZ[IX(i  , j  , k+1)]
                        -velZ[IX(i  , j  , k-1)]
                    )/N;
                p[IX(i, j, k)] = 0;
            }
        }
    }

    set_bounds(0, div); 
    set_bounds(0, p);
    lin_solve(0, p, div, 1, 6);
    
    for (int k = 1; k < N - 1; k++) {
        for (int j = 1; j < N - 1; j++) {
            for (int i = 1; i < N - 1; i++) {
                velX[IX(i, j, k)] -= 0.5f * (  p[IX(i+1, j, k)]
                                                -p[IX(i-1, j, k)]) * N;
                velY[IX(i, j, k)] -= 0.5f * (  p[IX(i, j+1, k)]
                                                -p[IX(i, j-1, k)]) * N;
                velZ[IX(i, j, k)] -= 0.5f * (  p[IX(i, j, k+1)]
                                                -p[IX(i, j, k-1)]) * N;
            }
        }
    }
    set_bounds(1, velX);
    set_bounds(2, velY);
    set_bounds(3, velZ);
}

void Fluid::FluidStep() {
    this->diffuse(1, this->Vx0, this->Vx, this->visc, this->dt);
    this->diffuse(2, this->Vy0, this->Vy, this->visc, this->dt);
    this->diffuse(3, this->Vz0, this->Vz, this->visc, this->dt);
    
    this->project(this->Vx0, this->Vy0, this->Vz0, this->Vx, this->Vy);
    
    this->advect(1, this->Vx, this->Vx0, this->Vx0, this->Vy0, this->Vz0, this->dt);
    this->advect(2, this->Vy, this->Vy0, this->Vx0, this->Vy0, this->Vz0, this->dt);
    this->advect(3, this->Vz, this->Vz0, this->Vx0, this->Vy0, this->Vz0, this->dt);
    
    this->project(this->Vx, this->Vy, this->Vz, this->Vx0, this->Vy0);
    
    this->diffuse(0, this->s, this->density, this->diff, this->dt);
    this->advect(0, this->density, this->s, this->Vx, this->Vy, this->Vz, this->dt);

}

void Fluid::AddDensity(int x, int y, int z, float amount) {
    int N = this->size;
    this->density[IX(x, y, z)] += amount;
}

void Fluid::AddVelocity(int x, int y, int z, float amountX, float amountY, float amountZ) {
    int N = this->size;
    int index = IX(x, y, z);
    
    this->Vx[index] += amountX;
    this->Vy[index] += amountY;
    this->Vz[index] += amountZ;
}