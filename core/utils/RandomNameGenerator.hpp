#pragma once

#include <random>
#include <string>

namespace utils {
  inline std::string generateRandomName() {
    static const char *prefixes[] = {"Rocket", "Space",   "Star",  "Laser",
                                     "Nova",   "Astro",   "Turbo", "Quantum",
                                     "Pixel",  "Phantom", "Blitz", "Neo"};
    static const char *animals[] = {"Fox",   "Wolf", "Hawk", "Tiger", "Falcon",
                                    "Eagle", "Lion", "Bear", "Shark", "Cat",
                                    "Dog",   "Owl",  "Bat"};
    static std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<> prefixDist(
        0, sizeof(prefixes) / sizeof(prefixes[0]) - 1);
    std::uniform_int_distribution<> animalDist(
        0, sizeof(animals) / sizeof(animals[0]) - 1);
    std::string name =
        std::string(prefixes[prefixDist(gen)]) + " " + animals[animalDist(gen)];
    return name;
  }

}  // namespace utils
