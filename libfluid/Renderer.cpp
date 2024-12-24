#include "Renderer.hpp"

const char* vertexShaderSource = R"(
#version 330 core

layout(location = 0) in vec2 aPosition;
layout(location = 1) in vec2 aVelocity;

out float velocityMagnitude;

uniform mat4 uProjection; // Projection matrix
uniform float uParticleSize; // Particle size (radius)

void main() {
    gl_Position = uProjection * vec4(aPosition, 0.0, 1.0);
    velocityMagnitude = length(aVelocity);
    gl_PointSize = uParticleSize; // Scale the point size (to simulate the radius of a circle)
}
)";

const char* fragmentShaderSource = R"(
#version 330 core

in float velocityMagnitude; // Input velocity magnitude from vertex shader
out vec4 FragColor;         // Output color

uniform float uMaxVelocity; // Maximum velocity for normalization
uniform float uParticleSize; // Particle size (radius)

void main() {
    // Normalize the velocity magnitude to the range [0, 1]
    float velocityNorm = clamp(velocityMagnitude / uMaxVelocity, 0.0, 1.0);

    // Interpolate color from blue (low speed) to red (high speed)
    vec3 color = mix(vec3(0.0, 1.0, 0.0), vec3(1.0, 0.0, 1.0), velocityNorm);

    // Convert gl_PointCoord from [0,1] to [-1,1]
    vec2 circlePos = 2.0 * gl_PointCoord - 1.0;

    // Calculate distance from center
    float dist = length(circlePos);
    
    // Discard fragments outside the circle
    if (dist > 1.0) {
        discard;
    }

    // Output the final color
    FragColor = vec4(color, 0.7);
}
)";

const char* blurVertexSource = R"(
#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

void main() {
    TexCoord = aTexCoord;
    gl_Position = vec4(aPos, 0.0, 1.0);
}
)";

const char* blurFragmentSource = R"(
#version 330 core
uniform sampler2D image;
out vec4 FragmentColor;

uniform float offset[9] = float[](0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0);
uniform float weight[9] = float[](0.2270270270, 0.1945945946, 0.1216216216,
                                  0.0540540541, 0.0162162162, 0.009, 0.005,
                                  0.003, 0.001);

in vec2 TexCoord;

void main(void) {
    vec2 texSize = textureSize(image, 0);
    vec2 texel = 1.0 / texSize;

    FragmentColor = texture(image, TexCoord) * weight[0];
    for (int i = 1; i < 9; i++) {
        FragmentColor += texture(image, TexCoord + vec2(0.0, offset[i] * texel.y)) * weight[i];
        FragmentColor += texture(image, TexCoord - vec2(0.0, offset[i] * texel.y)) * weight[i];
    }
}
)";

const char* _blurFragmentSource = R"(
#version 330 core
uniform sampler2D image;
 
out vec4 FragmentColor;
 
uniform float offset[3] = float[](0.0, 1.3846153846, 3.2307692308);
uniform float weight[3] = float[](0.2270270270, 0.3162162162, 0.0702702703);
 
void main(void) {
    FragmentColor = texture(image, vec2(gl_FragCoord) / 1024.0) * weight[0];
    for (int i=1; i<3; i++) {
        FragmentColor +=
            texture(image, (vec2(gl_FragCoord) + vec2(0.0, offset[i])) / 1024.0)
                * weight[i];
        FragmentColor +=
            texture(image, (vec2(gl_FragCoord) - vec2(0.0, offset[i])) / 1024.0)
                * weight[i];
    }
}
)";

/* ----- Improved Smoke-like ----- */
const char* smokeLikeVertexSource = R"(
#version 330 core
layout(location = 0) in vec2 aPosition;
layout(location = 1) in vec2 aVelocity;

out float velocityMagnitude;
out vec2 particlePos;

uniform mat4 uProjection;
uniform float uParticleSize;

void main() {
    gl_Position = uProjection * vec4(aPosition, 0.0, 1.0);
    velocityMagnitude = length(aVelocity);
    particlePos = aPosition;
    gl_PointSize = uParticleSize * 2.0; // Increased size for more overlap
}
)";

const char* smokeLikeFragmentSource = R"(
#version 330 core
in float velocityMagnitude;
in vec2 particlePos;
out vec4 FragColor;

uniform float uMaxVelocity;
uniform float uParticleSize;

void main() {
    // Convert gl_PointCoord from [0,1] to [-1,1]
    vec2 circlePos = 2.0 * gl_PointCoord - 1.0;
    
    // Calculate distance from center
    float dist = length(circlePos);
    
    // Create a soft, circular particle
    float alpha = smoothstep(1.0, 0.0, dist);
    
    // Normalize the velocity magnitude to the range [0, 1]
    float velocityNorm = smoothstep(0.0, uMaxVelocity, velocityMagnitude);
    
    // Create a more ethereal color scheme
    vec3 baseColor = mix(
        vec3(0.5, 0.7, 1.0),  // Light blue for slow particles
        vec3(0.8, 0.4, 1.0),  // Purple for fast particles
        velocityNorm
    );
    
    // Apply gaussian-like falloff
    float softness = exp(-dist * dist * 2.0);
    
    // Final color with smooth alpha falloff
    FragColor = vec4(baseColor, alpha * softness * 0.3);  // Reduced alpha for better blending
}
)";

const char* blurSmokeLikeFragmentSource = R"(
#version 330 core
uniform sampler2D image;
out vec4 FragColor;

in vec2 TexCoord;

const float weight[5] = float[](0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

void main() {
    vec2 texOffset = 2.0 / textureSize(image, 0); // Increased blur radius
    vec4 result = texture(image, TexCoord) * weight[0];
    
    for(int i = 1; i < 5; ++i) {
        result += texture(image, TexCoord + vec2(0.0, texOffset.y * i)) * weight[i];
        result += texture(image, TexCoord - vec2(0.0, texOffset.y * i)) * weight[i];
        result += texture(image, TexCoord + vec2(texOffset.x * i, 0.0)) * weight[i];
        result += texture(image, TexCoord - vec2(texOffset.x * i, 0.0)) * weight[i];
    }
    
    // Enhance the blur effect
    FragColor = result * 1.2; // Slight boost to visibility
}
)";

/* ------------------- */
const char* blurVertexSource2 = R"(
#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

void main() {
    gl_Position = vec4(aPos, 0.0, 1.0);
    TexCoord = aTexCoord;
}
)";

const char* blurFragmentSource2 = R"(
#version 330 core
uniform sampler2D image;
out vec4 FragColor;

in vec2 TexCoord;

// Gaussian weights for 5 samples
const float weight[5] = float[](0.227027, 0.194595, 0.121622, 0.054054, 0.016216);

void main() {
    vec2 tex_offset = 1.0 / textureSize(image, 0); // gets size of single texel
    vec4 result = texture(image, TexCoord) * weight[0]; // current fragment's contribution
    
    // Only sample fewer pixels for better performance and to prevent over-brightening
    for(int i = 1; i < 5; ++i) {
        result += texture(image, TexCoord + vec2(tex_offset.x * i, 0.0)) * weight[i] * 0.5;
        result += texture(image, TexCoord - vec2(tex_offset.x * i, 0.0)) * weight[i] * 0.5;
        result += texture(image, TexCoord + vec2(0.0, tex_offset.y * i)) * weight[i] * 0.5;
        result += texture(image, TexCoord - vec2(0.0, tex_offset.y * i)) * weight[i] * 0.5;
    }

    // Prevent over-saturation
    FragColor = min(result, vec4(1.0));
}
)";

Renderer::Renderer(int width, int height) : width(width), height(height) {
    window = glfwCreateWindow(width, height, "Fluid Simulation", nullptr, nullptr);
    if (!window) {
        throw std::runtime_error("Failed to create GLFW window");
    }
    glfwMakeContextCurrent(window);
    if (glewInit() != GLEW_OK)
        throw std::runtime_error("Failed to initialize GLEW");
    glViewport(0, 0, width, height);

    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Create shaders and program
    shaderProgram = createProgram(vertexShaderSource, fragmentShaderSource);
    // shaderProgram = createProgram(smokeLikeVertexSource, smokeLikeFragmentSource);

    // gaussianBlurShader = createProgram(blurVertexSource, blurFragmentSource);
    // gaussianBlurShader = createProgram(blurVertexSource, blurSmokeLikeFragmentSource);
    gaussianBlurShader = createProgram(blurVertexSource2, blurFragmentSource2);

    // Set up VAO and VBO
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    // Set up FBO
    // initFBO();
    initPingPongFBO();

    initFullscreenQuad();

    /* --------------------------- */

    // Quad vertices (positions and texture coordinates)
    // float quadVertices[] = {
    //     // Positions   // TexCoords
    //     -1.0f, -1.0f,  0.0f, 0.0f,
    //     1.0f, -1.0f,  1.0f, 0.0f,
    //     1.0f,  1.0f,  1.0f, 1.0f,
    //     -1.0f,  1.0f,  0.0f, 1.0f
    // };
    // float quadVertices[] = {
    //     -1.0f, -1.0f,
    //     1.0f, -1.0f,
    //     -1.0f,  1.0f,
    //     1.0f,  1.0f
    // };

    // glGenVertexArrays(1, &quadVAO);
    // glGenBuffers(1, &quadVBO);

    // glBindVertexArray(quadVAO);
    // glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    // glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    // Position attribute
    // glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    // glEnableVertexAttribArray(0);

    // // Texture coordinate attribute
    // glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    // glEnableVertexAttribArray(1);

    // glBindVertexArray(0);
}

Renderer::~Renderer() {
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
    glDeleteProgram(shaderProgram);
    glfwDestroyWindow(window);
}

void Renderer::render(const FluidSimulator& simulator) {
    // First clear the screen
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // 1. Render to first FBO
    glBindFramebuffer(GL_FRAMEBUFFER, fbo1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Render particles
    glUseProgram(shaderProgram);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    // Set up uniforms
    float particleSize = 10.0f; // Increased particle size for more overlap
    GLuint particleSizeLoc = glGetUniformLocation(shaderProgram, "uParticleSize");
    glUniform1f(particleSizeLoc, particleSize);

    glm::mat4 projection = glm::ortho(0.0f, (float)width, 0.0f, (float)height);
    GLuint projectionLoc = glGetUniformLocation(shaderProgram, "uProjection");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, &projection[0][0]);

    const std::vector<Particle>& particles = simulator.getParticles();
    float maxVelocity = 0.0f;
    for (const auto& particle : particles) {
        float speed = std::sqrt(particle.vx * particle.vx + particle.vy * particle.vy);
        maxVelocity = std::max(maxVelocity, speed);
    }
    maxVelocity = std::max(maxVelocity, 1e-5f);
    GLuint maxVelocityLoc = glGetUniformLocation(shaderProgram, "uMaxVelocity");
    glUniform1f(maxVelocityLoc, maxVelocity);

    glBufferData(GL_ARRAY_BUFFER, particles.size() * sizeof(Particle), particles.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)offsetof(Particle, x));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)offsetof(Particle, vx));
    glEnableVertexAttribArray(1);
    
    // Draw particles multiple times with slight offsets for more volume
    // for(int i = 0; i < 3; i++) {
    //     glDrawArrays(GL_POINTS, 0, particles.size());
    // }
    glDrawArrays(GL_POINTS, 0, particles.size());

    // 2. Apply multiple blur passes
    glUseProgram(gaussianBlurShader);
    glBindVertexArray(quadVAO);

    const int blurPasses = 0; // Increased blur passes
    GLuint currentFBO = fbo2;
    GLuint currentTexture = fboTexture1;

    for (int i = 0; i < blurPasses * 2; i++) {
        glBindFramebuffer(GL_FRAMEBUFFER, currentFBO);

        glUniform1i(glGetUniformLocation(gaussianBlurShader, "image"), 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, currentTexture);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        // Swap FBOs and textures
        currentFBO = (currentFBO == fbo2) ? fbo1 : fbo2;
        currentTexture = (currentTexture == fboTexture1) ? fboTexture2 : fboTexture1;
    }

    // 3. Final render to screen
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, currentTexture);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    // Cleanup
    glBindVertexArray(0);
    glUseProgram(0);

    glfwSwapBuffers(window);
    glfwPollEvents();
}

GLFWwindow* Renderer::getWindow() const {
    return window;
}

GLuint Renderer::createShader(const char* source, GLenum type) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        throw std::runtime_error("Shader compilation failed: " + std::string(infoLog));
    }

    return shader;
}

GLuint Renderer::createProgram(const char* vertexSrc, const char* fragmentSrc) {
    GLuint vertexShader = createShader(vertexSrc, GL_VERTEX_SHADER);
    GLuint fragmentShader = createShader(fragmentSrc, GL_FRAGMENT_SHADER);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        throw std::runtime_error("Shader program linking failed: " + std::string(infoLog));
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}

void Renderer::initFBO() {
    // Generate and bind FBO
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Generate and bind texture
    glGenTextures(1, &fboTexture);
    glBindTexture(GL_TEXTURE_2D, fboTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Attach the texture to the FBO as the color attachment
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboTexture, 0);

    // Optionally, create a renderbuffer for depth and stencil testing
    // glGenRenderbuffers(1, &rbo);
    // glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    // glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    // glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    // // Check if the framebuffer is complete
    // if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    //     throw std::runtime_error("Framebuffer is not complete!");
    // }

    // Unbind the framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::initPingPongFBO() {
    /* ----- FBO 1 ----- */
    // Generate and bind FBO
    glGenFramebuffers(1, &fbo1);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo1);

    // Generate and bind texture
    glGenTextures(1, &fboTexture1);
    glBindTexture(GL_TEXTURE_2D, fboTexture1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Attach the texture to the FBO as the color attachment
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboTexture1, 0);

    /* ----- FBO 2 ----- */
    // Generate and bind FBO
    glGenFramebuffers(1, &fbo2);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo2);

    // Generate and bind texture
    glGenTextures(1, &fboTexture2);
    glBindTexture(GL_TEXTURE_2D, fboTexture2);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Attach the texture to the FBO as the color attachment
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboTexture2, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::initFullscreenQuad() {
    float quadVertices[] = {
        // Positions   // Texture Coords
        -1.0f,  1.0f,  0.0f, 1.0f, // Top-left
        -1.0f, -1.0f,  0.0f, 0.0f, // Bottom-left
        1.0f,  1.0f,  1.0f, 1.0f, // Top-right
        1.0f, -1.0f,  1.0f, 0.0f  // Bottom-right
    };

    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);

    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}
