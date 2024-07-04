#include <iostream>
#include <SFML/Graphics.hpp>
#include "renderer.hpp"
#include "solver.hpp"
#include <random>

sf::Color generateRainbowColor(float value) {
    // Adjust the frequency to slow down the color change
    float frequency = 0.3f;  // Adjust this value for slower change

    // Calculate the RGB values using smooth transitions
    sf::Uint8 r = static_cast<sf::Uint8>((std::sin(2 * M_PI * frequency * value) + 1) * 127.5);
    sf::Uint8 g = static_cast<sf::Uint8>((std::sin(2 * M_PI * frequency * value + 2 * M_PI / 3) + 1) * 127.5);
    sf::Uint8 b = static_cast<sf::Uint8>((std::sin(2 * M_PI * frequency * value + 4 * M_PI / 3) + 1) * 127.5);

    return sf::Color(r, g, b);
}


int main() {
    // Setting size of window
    constexpr int32_t window_width  = 1000;
    constexpr int32_t window_height = 1000;


    sf::ContextSettings settings;
    settings.antialiasingLevel = 1;
    sf::RenderWindow window(sf::VideoMode(window_width, window_height), "Verlet", sf::Style::Default, settings);
    const uint32_t frame_rate = 60;
    window.setFramerateLimit(frame_rate);

    Solver solver;
    Renderer renderer{window};
    sf::Clock clock;

    // Configure the solver
    solver.setConstraint({static_cast<float>(window_width) * 0.5f, static_cast<float>(window_height) * 0.4f}, 350.0f);
    solver.setSubStepsCount(8);
    solver.setSimulationUpdateRate(frame_rate);

    // Initial spawn attributes
    const uint32_t max_objects_count = 2000;
    const float object_spawn_delay = .01f;
    const sf::Vector2f spawn_position = {500.0f, 200.0f};
    const int object_min_radius = 3;
    const int object_max_radius = 9;
    const float object_intial_speed = 1000.0f;
    const float max_angle = 1.0f;

    // Generate RNG numbers to randomly change the size of the objects radius's
    unsigned int seed = 1000;
    std::mt19937 gen(seed); 
    std::uniform_int_distribution<> distr(object_min_radius, object_max_radius);


    // Main loop
    while (window.isOpen()) {
        sf::Event event{};
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed || sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) {
                window.close();
            }
        } 
        // Spawn and object if there is space and a set amount of time has passed since last object has been spawned (object_spawn_delay)
        if (solver.getObjectsCount() < max_objects_count && clock.getElapsedTime().asSeconds() > object_spawn_delay) {
            clock.restart();
            float t = solver.getTime();
            auto& object = solver.addObjects(spawn_position, distr(gen));
            object.color = generateRainbowColor(t);
            const float angle  = max_angle * sin(t) + 3.14159 * 0.5f;
            solver.setObjectVelocity(object, object_intial_speed * sf::Vector2f{cos(angle), sin(angle)});

        }
        // Update the solver
        solver.update();

        // Clear the previous frame
        window.clear(sf::Color(176, 196, 222));

        // Render a new frame using the information from the updated solver
        renderer.render(solver);

        // Display the new window
        window.display();
    }
    return 0;
}