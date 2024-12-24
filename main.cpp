// #define GLFW_INCLUDE_NONE
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>

#include "libfluid/FluidSimulator.hpp"
#include "libfluid/InputHandler.hpp"
#include "libfluid/Renderer.hpp"

#define _DEBUG 1


static void PrintOpenGLErrors(const char* Function, const char* File, int Line)
{
	bool Succeeded = true;

	GLenum Error = glGetError();
	if (Error != GL_NO_ERROR)
	{
		const char* ErrorString = (const char*)gluErrorString(Error);
		if (ErrorString)
			std::cerr << ("OpenGL Error in %s at line %d calling function %s: '%s'", File, Line, Function, ErrorString) << std::endl;
		else
			std::cerr << ("OpenGL Error in %s at line %d calling function %s: '%d 0x%X'", File, Line, Function, Error, Error) << std::endl;
	}
}

#ifdef _DEBUG
#define CheckedGLCall(x) do { PrintOpenGLErrors(">>BEFORE<< "#x, __FILE__, __LINE__); (x); PrintOpenGLErrors(#x, __FILE__, __LINE__); } while (0)
#define CheckedGLResult(x) (x); PrintOpenGLErrors(#x, __FILE__, __LINE__);
#define CheckExistingErrors(x) PrintOpenGLErrors(">>BEFORE<< "#x, __FILE__, __LINE__);
#else
#define CheckedGLCall(x) (x)
#define CheckedGLResult(x) (x)
#define CheckExistingErrors(x)
#endif


static void error_callback(int error, const char* description)
{
	std::cerr << "GLFW Error: " << description << std::endl;
}


static void key_callback(GLFWwindow* window, const int key, const int scancode, const int action, const int mods)
{
	if (action == GLFW_RELEASE && key == GLFW_KEY_ESCAPE)
		glfwSetWindowShouldClose(window, GLFW_TRUE);
}


int main() {
    glfwSetErrorCallback(error_callback);

    if (!glfwInit()) return EXIT_FAILURE;

    // int width = 960, height = 1080;
    int width = 1920, height = 1080;
    try {
        FluidSimulator simulator(width, height);
        Renderer renderer(width, height);
        InputHandler inputHandler(renderer.getWindow(), simulator);

        while (!glfwWindowShouldClose(renderer.getWindow())) {
            inputHandler.processInput();
            simulator.update(0.016f); // Assuming 60 FPS
            renderer.render(simulator);
            glfwPollEvents();
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwTerminate();

    return EXIT_SUCCESS;
}
