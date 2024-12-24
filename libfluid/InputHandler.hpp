#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Renderer.hpp"
#include "FluidSimulator.hpp"


class InputHandler {
public:
    InputHandler(GLFWwindow* window, FluidSimulator& simulator);
    void processInput();

private:
    GLFWwindow* window;
    FluidSimulator& simulator;
    bool isLeftClickActive = false;
    bool isRightClickActive = false;
    double mouseX = 0.0, mouseY = 0.0;
};
