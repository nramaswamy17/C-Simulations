#include <SFML/Graphics.hpp>
#include <iostream>
#include <sstream>
#include <iomanip>
#include "Car.h"
#include "Environment.h"
#include "ParkingSpot.h"
#include "SemiTruck.h"

int main() {
    // Create window
    const float WINDOW_WIDTH = 1000.0f;
    const float WINDOW_HEIGHT = 700.0f;
    
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Autonomous Car Simulator");
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

    // Create environment and car
    Environment environment(WINDOW_WIDTH, WINDOW_HEIGHT);
    //Car car(WINDOW_WIDTH/2, WINDOW_HEIGHT / 2, 50, 30, sf::Color(49, 130, 206));
    SemiTruck semiTruck(500, 500, 0.0f, 0.0f);


    // Create parking spot
    ParkingSpot parkingSpot;
    parkingSpot.generateRandom(WINDOW_WIDTH, WINDOW_HEIGHT, environment.wallThickness);

    int successfulParkings = 0;
    int attempts = 1;
    bool justParked = false;
    sf::Clock parkingTimer;

    sf::Clock clock;

    sf::Clock loopTimer; // Measures loop iterations

    while (window.isOpen()) {
        loopTimer.restart(); // Start iteration timing

        sf:: Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        
            // Reset on R key
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::R){
                //car = Car(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2, 50, 30, sf::Color(49, 130, 206));
                semiTruck = SemiTruck(500, 500, 0.0f, 0.0f);
                parkingSpot.generateRandom(WINDOW_WIDTH, WINDOW_HEIGHT, environment.wallThickness);
                attempts++;
                justParked = false;
            }
        }
    
        // Update the physics
        float dt = clock.restart().asSeconds();
        semiTruck.handleInput(dt);
        semiTruck.update(dt);
        environment.handleSemiCollision(semiTruck);
        
        // Check if semi is parked successfully 
        if (parkingSpot.checkIfParked(semiTruck) && !justParked) {
            successfulParkings++;
            justParked = true;
            parkingTimer.restart();
        }
        
        // Auto-generate new spot after 3 seconds of successful parking
        if (justParked && parkingTimer.getElapsedTime().asSeconds() > 3.0f) {
            //car = Car(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2, 50, 30, sf::Color(49, 130, 206));
            semiTruck = SemiTruck(500, 500, 0.0f, 0.0f);
            parkingSpot.generateRandom(WINDOW_WIDTH, WINDOW_HEIGHT, environment.wallThickness);
            attempts++;
            justParked = false;
        }

        // drawing
        window.clear();

        environment.draw(window);
        parkingSpot.draw(window);
        semiTruck.draw(window);

        // Draw car info
        sf::Text text;
        text.setFont(font);
        text.setCharacterSize(18);
        text.setFillColor(sf::Color::Black);

        std::stringstream ss;
        ss << "Car Status\n"
           << "Latency: " << std::fixed << std::setprecision(2) << loopTimer.getElapsedTime().asMicroseconds() / 1000.0f << " ms\n"
           << "Position: (" << std::fixed << std::setprecision(1) 
           << semiTruck.cab_x << ", " << semiTruck.cab_y << ")\n"
           << "Angle: " << std::setprecision(0) << semiTruck.cab_angle << " deg\n"
           << "Speed: " << std::setprecision(1) << semiTruck.cab_speed << " px/s\n"
           << "Collision Status: " << (semiTruck.isColliding ? "True" : "False") << "\n"
           << "Target Angle: " << std::setprecision(0) << parkingSpot.targetAngle << "deg\n"
           << "Position Error: " << std::setprecision(1) << parkingSpot.getPositionError(semiTruck) << " px\n"
           << "Angle Error: " << std::setprecision(1) << parkingSpot.getAngleError(semiTruck) << " deg\n"
           << "Parking Status: " << (parkingSpot.isParked ? "PARKED!" : "Not Parked") << "\n"
           << "Score" << successfulParkings << " / " << attempts << "\n";
        
        text.setString(ss.str());
        text.setPosition(30, 30);
        window.draw(text);
        
        window.display();
    }

    return 0;
}