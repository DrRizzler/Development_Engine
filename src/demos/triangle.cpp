#include <SFML/Window.hpp>
#include <glad/gl.h>

#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#ifndef APIENTRY
#define APIENTRY
#endif

// ---------- OpenGL helpers ----------
static GLuint compileShader(GLenum type, const char* src) {
    GLuint sh = glCreateShader(type);
    glShaderSource(sh, 1, &src, nullptr);
    glCompileShader(sh);

    GLint ok = 0;
    glGetShaderiv(sh, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        GLint len = 0;
        glGetShaderiv(sh, GL_INFO_LOG_LENGTH, &len);
        std::string log(len, '\0');
        glGetShaderInfoLog(sh, len, nullptr, log.data());
        std::cerr << "Shader compile failed:\n" << log << "\n";
        glDeleteShader(sh);
        return 0;
    }
    return sh;
}

static GLuint linkProgram(GLuint vs, GLuint fs) {
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);

    GLint ok = 0;
    glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if (!ok) {
        GLint len = 0;
        glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &len);
        std::string log(len, '\0');
        glGetProgramInfoLog(prog, len, nullptr, log.data());
        std::cerr << "Program link failed:\n" << log << "\n";
        glDeleteProgram(prog);
        return 0;
    }
    return prog;
}

static void APIENTRY glDebugCb(GLenum /*source*/, GLenum /*type*/, GLuint /*id*/,
                              GLenum severity, GLsizei /*length*/,
                              const GLchar* message, const void* /*userParam*/) {
    // Ignore spammy notifications
    if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) return;
    std::cerr << "GL: " << message << "\n";
}

int main() {
    sf::ContextSettings settings;
    settings.depthBits = 24;
    settings.stencilBits = 8;
    settings.antiAliasingLevel = 4;
    settings.majorVersion = 3;
    settings.minorVersion = 3;

    sf::Window window(
        sf::VideoMode(sf::Vector2u{1280u, 720u}),
        "Engine Test (SFML3 + GLAD)",
        sf::State::Windowed,
        settings
    );

    // Load OpenGL function pointers using SFML's loader
    if (!gladLoadGL(reinterpret_cast<GLADloadfunc>(sf::Context::getFunction))) {
        std::cerr << "Failed to load OpenGL via gladLoadGL(sf::Context::getFunction)\n";
        return 1;
    }

    std::cout << "OpenGL Vendor:   " << glGetString(GL_VENDOR) << "\n";
    std::cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << "\n";
    std::cout << "OpenGL Version:  " << glGetString(GL_VERSION) << "\n";

    // Optional: OpenGL debug output (works on your GL 4.6 driver)
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(glDebugCb, nullptr);

    glEnable(GL_DEPTH_TEST);

    // ----- Triangle shaders -----
    // Using GLSL 330 core since you're requesting 3.3 context
    const char* vsSrc = R"GLSL(
    #version 330 core
    layout (location = 0) in vec2 aPos;
    layout (location = 1) in vec3 aColor;

    uniform float uTime;

    out vec3 vColor;

    void main() {
        float a = uTime;
        mat2 R = mat2(
            cos(a), -sin(a),
            sin(a),  cos(a)
        );

        vec2 rotated = R * aPos;

        vColor = aColor;
        gl_Position = vec4(rotated, 0.0, 1.0);
        }
    )GLSL";


    const char* fsSrc = R"GLSL(
        #version 330 core
        in vec3 vColor;
        out vec4 FragColor;
        void main() {
            FragColor = vec4(vColor, 1.0);
        }
    )GLSL";

    GLuint vs = compileShader(GL_VERTEX_SHADER, vsSrc);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fsSrc);
    if (!vs || !fs) return 1;

    GLuint prog = linkProgram(vs, fs);
    GLint uTimeLoc = glGetUniformLocation(prog, "uTime");

    glDeleteShader(vs);
    glDeleteShader(fs);
    if (!prog) return 1;

    // ----- Triangle geometry -----
    // Interleaved: x,y, r,g,b
    float verts[] = {
        //  x      y       r     g     b
         0.0f,  0.6f,    1.0f, 0.2f, 0.2f,
        -0.6f, -0.6f,    0.2f, 1.0f, 0.2f,
         0.6f, -0.6f,    0.2f, 0.2f, 1.0f
    };

    GLuint vao = 0, vbo = 0;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

    // aPos (vec2) at location 0
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // aColor (vec3) at location 1
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Set initial viewport once
    {
        auto s = window.getSize();
        glViewport(0, 0, static_cast<GLsizei>(s.x), static_cast<GLsizei>(s.y));
    }
    auto startTime = std::chrono::steady_clock::now();

    // ----- Main loop -----
    while (window.isOpen()) {
        while (auto ev = window.pollEvent()) {
            if (ev->is<sf::Event::Closed>()) {
                window.close();
                continue;
            }

            if (const auto* rs = ev->getIf<sf::Event::Resized>()) {
                glViewport(0, 0, static_cast<GLsizei>(rs->size.x), static_cast<GLsizei>(rs->size.y));
            }

            if (const auto* kp = ev->getIf<sf::Event::KeyPressed>()) {
                if (kp->code == sf::Keyboard::Key::Escape) {
                    window.close();
                }
            }
        }

        glClearColor(0.08f, 0.08f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float time = std::chrono::duration<float>(
            std::chrono::steady_clock::now() - startTime
        ).count();

        glUseProgram(prog);
        glUniform1f(uTimeLoc, time);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);
        glUseProgram(0);

        window.display();
    }

    // Cleanup
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteProgram(prog);

    return 0;
}
