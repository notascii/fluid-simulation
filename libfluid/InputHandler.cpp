#include "InputHandler.hpp"

InputHandler::InputHandler(GLFWwindow* window, FluidSimulator& simulator)
    : window(window), simulator(simulator) {
    glfwSetWindowUserPointer(window, this);

    // Set up mouse button callback
    glfwSetMouseButtonCallback(window, [](GLFWwindow* w, int button, int action, int mods) {
        auto* handler = static_cast<InputHandler*>(glfwGetWindowUserPointer(w));
        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            handler->isLeftClickActive = (action == GLFW_PRESS);
        } else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
            handler->isRightClickActive = (action == GLFW_PRESS);
        }
    });

    // Set up mouse position callback
    glfwSetCursorPosCallback(window, [](GLFWwindow* w, double x, double y) {
        auto* handler = static_cast<InputHandler*>(glfwGetWindowUserPointer(w));
        handler->mouseX = x;
        handler->mouseY = y;
    });
}


void InputHandler::processInput() {
    int width, height;
    glfwGetWindowSize(window, &width, &height); // Get the window dimensions

    float correctedY = static_cast<float>(height) - static_cast<float>(mouseY); // Invert y-coordinate

    if (isLeftClickActive) {
        simulator.addPerturbation(static_cast<float>(mouseX), correctedY, 300.0f, -300.0f); // Attract
    }
    if (isRightClickActive) {
        simulator.addPerturbation(static_cast<float>(mouseX), correctedY, 300.0f, 300.0f); // Repel
    }

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
}
