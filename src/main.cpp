// Implementacja testowa symulacji z wolnym ruchem kamery
//-------------------------------------------------------
// Kompilacja WIN32:        
//            g++ src/*.cpp src/*c -Iinclude -L/ucrt64/lib -o turbine -lSDL3 -lopengl32 
//            turbine.exe
//-------------------------------------------------------
// Kompilacja LINUX: 
//            g++ src/*.cpp src/*.c -Iinclude -L/usr/local/lib -o turbine -lSDL3 -lGL 
//            ./turbine
//-------------------------------------------------------
// Jeśli będą problemy z pamięcią: 
//            g++ -g -O1 -fsanitize=address,undefined -fno-omit-frame-pointer src/*.cpp src/*.c -Iinclude -L/usr/local/lib -o turbine -lSDL3 -lGL  
//            ./turbine                   
//-------------------------------------------------------

#include "Simulation.h"

int main(int argc, char** argv) {
    std::cout << "Symulacja rozpoczęta... \nNaciśnij: \nQ - przełącz tryb myszy\nEsc - wyjdź z symulacji" << std::endl;

    Simulation& sim = Simulation::Instance();

    sim.Run();

    return 0;
}
