#include <SFML/Graphics.hpp>
#include <iostream>
#include <sstream>
#include <iomanip>
#include "Car.h"
#include "Environment.h"
#include "Lane.h"
#include "SemiTruck.h"
#include "Controller.h"

int main() {
    // Create window - larger to fit the full oval track
    const float WINDOW_WIDTH = 1400.0f;
    const float WINDOW_HEIGHT = 900.0f;
    
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), 
                           "Semi Truck Lane Keeping System");
    window.setFramerateLimit(60);
    
    // Load font for text
    sf::Font font;
    if (!font.loadFromFile("/System/Library/Fonts/Helvetica.ttc")) {
        // Try Windows font path
        if (!font.loadFromFile("C:\\Windows\\Fonts\\arial.ttf")) {
            // Try Linux font path
            if (!font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf")) {
                std::cout << "Warning: Could not load font. Text will not display." << std::endl;
            }
        }
    }

    // Create environment and road
    Environment environment(WINDOW_WIDTH, WINDOW_HEIGHT);
    Road road(WINDOW_WIDTH, WINDOW_HEIGHT, environment.wallThickness);
    environment.setRoad(&road);
    
    // Create semi truck - position it precisely on middle lane at bottom of oval
    // Bottom of oval: theta = π/2
    // Position: (centerX, centerY + radiusY) where radiusY = (HEIGHT/2 - 80)
    float startX = WINDOW_WIDTH / 2;  // = 700
    float startY = WINDOW_HEIGHT / 2 + (WINDOW_HEIGHT / 2 - 80);  // = 450 + 370 = 820
    float startAngle = 180.0f; // Facing left for clockwise motion
    SemiTruck semiTruck(startX, startY, startAngle, 0.0f, true);
    
    // Create lane keeping controller
    Controller controller;
    controller.setTargetLane(1); // Middle lane

    // Autonomous NPC Truck 1
    float truck2_startX = WINDOW_WIDTH / 2;
    float truck2_startY = WINDOW_HEIGHT / 2 - (WINDOW_HEIGHT / 2 - 80);
    float truck2_startAngle = 0.0f; // facing right
    SemiTruck semiTruck2(truck2_startX, truck2_startY, truck2_startAngle, 80.0f, true);

    Controller controller2;
    controller2.setTargetLane(0);
    controller2.enable(); // start in autonomous mode

    // Autonomous NPC Truck 2
    float truck3_startX = WINDOW_WIDTH / 2;
    float truck3_startY = WINDOW_HEIGHT / 2 - (WINDOW_HEIGHT / 2 - 80);
    float truck3_startAngle = 0.0f; // facing right
    SemiTruck semiTruck3(truck3_startX, truck3_startY, truck3_startAngle, 80.0f, true);

    Controller controller3;
    controller3.setTargetLane(2);
    controller3.enable(); // start in autonomous mode

    
    // Performance metrics
    float totalDistanceTraveled = 0.0f;
    float timeInLane = 0.0f;
    float timeOutOfLane = 0.0f;
    int laneDepartures = 0;
    bool wasInLane = true;
    
    sf::Clock clock;
    sf::Clock loopTimer;
    sf::Clock totalTimer;

    while (window.isOpen()) {
        loopTimer.restart();

        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            // Toggle autonomous mode with spacebar
            if (event.type == sf::Event::KeyPressed && 
                event.key.code == sf::Keyboard::Space) {
                controller.toggle();
                std::cout << "Lane Keeping: " << (controller.isEnabled ? "ON" : "OFF") 
                         << std::endl;
            }
            
            // Lane selection with number keys (1, 2, 3)
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Num1) {
                    controller.setTargetLane(0);
                    std::cout << "Target: Left Lane" << std::endl;
                } else if (event.key.code == sf::Keyboard::Num2) {
                    controller.setTargetLane(1);
                    std::cout << "Target: Middle Lane" << std::endl;
                } else if (event.key.code == sf::Keyboard::Num3) {
                    controller.setTargetLane(2);
                    std::cout << "Target: Right Lane" << std::endl;
                }
            }
        
            // Reset on R key
            if (event.type == sf::Event::KeyPressed && 
                event.key.code == sf::Keyboard::R) {
                float resetX = WINDOW_WIDTH / 2;
                float resetY = WINDOW_HEIGHT / 2 + (WINDOW_HEIGHT / 2 - 80);
                semiTruck = SemiTruck(resetX, resetY, 180.0f, 0.0f, false);
                controller.setTargetLane(1);
                totalDistanceTraveled = 0.0f;
                timeInLane = 0.0f;
                timeOutOfLane = 0.0f;
                laneDepartures = 0;
                wasInLane = true;
                totalTimer.restart();
                std::cout << "System reset" << std::endl;
            }
        }
    
        // Update physics
        float dt = clock.restart().asSeconds();
        
        if (controller.isEnabled) {
            controller.update(semiTruck, road, dt);
        } else {
            semiTruck.handleInput(dt);
        }
        
        // Player semi truck
        semiTruck.update(dt);
        semiTruck.updateSensors(WINDOW_WIDTH, WINDOW_HEIGHT, 
                               environment.wallThickness);
        environment.handleSemiCollision(semiTruck);

        // NPC Semi truck 1
        controller2.update(semiTruck2, road, dt);
        semiTruck2.update(dt);
        semiTruck2.updateSensors(WINDOW_WIDTH, WINDOW_HEIGHT, 
                               environment.wallThickness);
        environment.handleSemiCollision(semiTruck2);

        // NPC Semi truck 2
        controller3.update(semiTruck3, road, dt);
        semiTruck3.update(dt);
        semiTruck3.updateSensors(WINDOW_WIDTH, WINDOW_HEIGHT, 
                               environment.wallThickness);
        environment.handleSemiCollision(semiTruck3);

        
        // Update metrics
        totalDistanceTraveled += std::abs(semiTruck.cab_speed) * dt;
        
        // Check lane status
        const Lane& currentLane = road.lanes[controller.targetLaneIndex];
        bool isInLane = currentLane.isInLane(semiTruck);
        
        if (isInLane) {
            timeInLane += dt;
            if (!wasInLane) {
                // Just re-entered lane
                wasInLane = true;
            }
        } else {
            timeOutOfLane += dt;
            if (wasInLane) {
                // Just departed from lane
                laneDepartures++;
                wasInLane = false;
            }
        }
        
        // Drawing
        window.clear();
        
        environment.draw(window);
        semiTruck.draw(window);
        semiTruck2.draw(window);
        semiTruck3.draw(window);

        // Draw controller guidance visualization
        /*
        if (controller.isEnabled) {
            float desiredHeading = controller.getDesiredHeading(semiTruck, road);
            semiTruck.drawControllerGuidance(window, desiredHeading, controller.isEnabled);
        }
        */

        // Draw UI
        sf::Text text;
        text.setFont(font);
        text.setCharacterSize(16);
        text.setFillColor(sf::Color::White);
        
        // Get current lane info
        float lateralError = currentLane.getLateralError(semiTruck);
        float headingError = currentLane.getHeadingError(semiTruck);
        float distToLeft = currentLane.getDistanceToLeftEdge(semiTruck);
        float distToRight = currentLane.getDistanceToRightEdge(semiTruck);
        
        std::stringstream ss;
        ss << "=== LANE KEEPING SYSTEM ===\n"
           << "Mode: " << (controller.isEnabled ? "AUTONOMOUS" : "MANUAL") << "\n"
           << "State: " << controller.getStateName() << "\n\n"
           << "--- Truck Status ---\n"
           << "Position: (" << std::fixed << std::setprecision(0) 
           << semiTruck.cab_x << ", " << semiTruck.cab_y << ")\n"
           << "Heading: " << std::setprecision(0) << semiTruck.cab_angle << " deg\n"
           << "Speed: " << std::setprecision(1) << semiTruck.cab_speed << " px/s\n"
           << "Collision: " << (semiTruck.isColliding ? "YES" : "NO") << "\n\n"
           << "--- Lane Info ---\n"
           << "Target Lane: " << (controller.targetLaneIndex + 1) << " of 3\n"
           << "In Lane: " << (isInLane ? "YES" : "NO") << "\n"
           << "Lateral Error: " << std::setprecision(1) << lateralError << " px\n"
           << "Heading Error: " << std::setprecision(1) << headingError << " deg\n"
           << "Dist to Left: " << std::setprecision(0) << distToLeft << " px\n"
           << "Dist to Right: " << std::setprecision(0) << distToRight << " px\n\n"
           << "--- Performance ---\n"
           << "Distance: " << std::setprecision(0) << totalDistanceTraveled << " px\n"
           << "Time in Lane: " << std::setprecision(1) 
           << timeInLane << "s (" 
           << std::setprecision(0) << (timeInLane / (timeInLane + timeOutOfLane + 0.001f) * 100.0f) 
           << "%)\n"
           << "Lane Departures: " << laneDepartures << "\n"
           << "Latency: " << std::setprecision(3) << loopTimer.getElapsedTime().asSeconds() * 1000.0f << " ms\n\n"
            ;
        
        text.setString(ss.str());
        text.setPosition(WINDOW_WIDTH - 290, 10);  // Move to top-right corner
        
        // Add semi-transparent background for readability
        sf::RectangleShape textBg(sf::Vector2f(280, 500));
        textBg.setPosition(WINDOW_WIDTH - 295, 5);
        textBg.setFillColor(sf::Color(0, 0, 0, 180));
        window.draw(textBg);
        
        window.draw(text);
        window.display();
    }

    return 0;
}