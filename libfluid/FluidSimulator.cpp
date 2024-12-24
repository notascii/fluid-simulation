#include <glm/glm.hpp>

#include "FluidSimulator.hpp"
#include <cmath>

FluidSimulator::FluidSimulator(int width, int height)
    : width(width), height(height) {
    // Initialize particles
    for (int y = 0; y < height; y += 5) {
        for (int x = 0; x < width; x += 5) {
            particles.push_back({ static_cast<float>(x), static_cast<float>(y), 0.0f, 0.0f });
        }
    }
}

void FluidSimulator::update(float dt) {
    float damping = 0.96f; // Damping coefficient (0 < damping ≤ 1)
    float gravity = 5 * -9.8f; // Gravity force
    float interactionRadius = 100.0f; // Radius of influence

    // Update particle positions using basic Euler integration
    for (auto& p : particles) {
        p.x += p.vx * dt;
        p.y += p.vy * dt;

        p.vx *= damping;
        p.vy *= damping;

        p.vy += gravity * dt;

        // glm::vec2 force(0.0f, 0.0f);
        // for (const auto& p2 : particles) {
        //     if (&p == &p2) continue; // Skip self-interaction
        //     glm::vec2 delta(p.x - p2.x, p.y - p2.y);
        //     float distance = glm::length(delta);
        //     if (distance < interactionRadius) {
        //         // Repulsion force inversely proportional to distance
        //         float strength = 10 * std::sqrt(p.vx * p.vx + p.vy * p.vy) * (interactionRadius - distance) / interactionRadius;
        //         force += glm::normalize(delta) * strength;
        //     }
        // }
        // p.vx += force.x * dt;
        // p.vy += force.y * dt;

        // Bounce off edges
        if (p.x < 0 || p.x > width) p.vx = -p.vx;
        if (p.y < 0 || p.y > height) p.vy = -p.vy;
    }
}

void FluidSimulator::addPerturbation(float x, float y, float radius, float strength) {
    for (auto& p : particles) {
        float dx = p.x - x;
        float dy = p.y - y;
        float distance = std::sqrt(dx * dx + dy * dy);
        if (distance < radius) {
            p.vx += strength * dx / distance;
            p.vy += strength * dy / distance;
        }
    }
}

const std::vector<Particle>& FluidSimulator::getParticles() const {
    return particles;
}

