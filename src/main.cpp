#include "Simulation.hpp"

int main(int argc, char** argv) {
    Simulation& sim = Simulation::Instance();
    sim.Run();

    return 0;
}