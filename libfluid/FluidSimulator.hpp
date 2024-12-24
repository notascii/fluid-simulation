#pragma once

#include <vector>

struct Particle {
    float x, y;
    float vx, vy; // Velocity
};

class FluidSimulator {
public:
    FluidSimulator(int width, int height);
    void update(float dt);
    void addPerturbation(float x, float y, float radius, float strength);
    const std::vector<Particle>& getParticles() const;

    float maxSpeed = 10.0f;
private:
    int width, height;
    std::vector<Particle> particles;
};
