#pragma once


#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/mat4x4.hpp> // glm::mat4

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <stdexcept>

#include "FluidSimulator.hpp"


class Renderer {
public:
    Renderer(int width, int height);
    ~Renderer();
    void render(const FluidSimulator& simulator);
    GLFWwindow* getWindow() const;

private:
    GLFWwindow* window;
    int width, height;

    GLuint shaderProgram;
    GLuint vao, vbo;

    GLuint fbo, fboTexture, rbo;
    GLuint fbo1, fboTexture1, fbo2, fboTexture2; // for multiple blur per frame
    GLuint quadVAO, quadVBO;
    GLuint gaussianBlurShader;

    void initFBO();
    void initPingPongFBO(); // for multiple blur per frame
    void initFullscreenQuad();

    GLuint createShader(const char* source, GLenum type);
    GLuint createProgram(const char* vertexSrc, const char* fragmentSrc);
};
