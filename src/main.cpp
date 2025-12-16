#include <SFML/Window.hpp>
#include <glad/gl.h>

#include <iostream>
#include <optional>

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
        sf::State::Windowed,   // SFML 3 uses sf::State here
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

    glEnable(GL_DEPTH_TEST);

    while (window.isOpen()) {
        while (auto ev = window.pollEvent()) {
            // Close event
            if (ev->is<sf::Event::Closed>()) {
                window.close();
                continue;
            }

            // Key pressed event (SFML3: use getIf)
            if (ev->is<sf::Event::KeyPressed>()) {
                if (const auto* kp = ev->getIf<sf::Event::KeyPressed>()) {
                    if (kp->code == sf::Keyboard::Key::Escape) {
                        window.close();
                    }
                }
            }
        }

        glViewport(0, 0,
                   static_cast<GLsizei>(window.getSize().x),
                   static_cast<GLsizei>(window.getSize().y));

        glClearColor(0.08f, 0.08f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        window.display();
    }

    return 0;
}
