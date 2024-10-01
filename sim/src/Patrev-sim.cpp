#include <pirate/tr/bar/BarSimRunner.hpp>
using pirate::tr::bar::BarSimRunner;

#include <Patrev-factory.hpp>

int main(int argc, char** argv) {
    Factory stratFactory;
    BarSimRunner sim("Patrev");
    return sim.runWithArgs(argc, argv, &stratFactory);
}
