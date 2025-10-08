#ifndef PARKINGSPOT_H
#define PARKINGSPOT_H

#include <SFML/Graphics.hpp>
#include <cmath>
#include <cstdlib>
#include "Car.h"
#include <random>
#include "SemiTruck.h"

class ParkingSpot {
    public:
        float x, y;
        float width, height;
        float targetAngle;
        sf::Color spotColor;
        sf::Color successColor;
        bool isParked;

        // Parking criteria
        float positionTolerance;
        float angleTolerance;
        float speedTolerance;

    // Random number generator
    static std::mt19937& getRNG() {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        return gen;
    }

    ParkingSpot() {
        width = 130.0f;
        height = 40.0f;
        spotColor = sf::Color(100, 200, 100, 100);      // Semi-transparent green
        successColor = sf::Color(50, 255, 50, 150);     // Brighter green
        isParked = false;
        positionTolerance = 10.0f;
        angleTolerance = 10.0f;
        speedTolerance = 20.0f;
    }

    void generateRandom(float windowWidth, float windowHeight, float wallThickness) {
        auto& gen = getRNG();
        
        // Generate random position (avoid edges)
        float margin = 100.0f;
        std::uniform_real_distribution<float> distX(margin, windowWidth - margin);
        std::uniform_real_distribution<float> distY(margin, windowHeight - margin);
        
        x = distX(gen);
        y = distY(gen);
        
        // Random target angle (0, 90, 180, or 270 degrees for simplicity)
        std::uniform_int_distribution<int> distAngle(0, 3);
        targetAngle = distAngle(gen) * 90.0f;
        
        isParked = false;
    }

    void draw(sf::RenderWindow& window) {
        // Draw parking spot rectangle
        sf::RectangleShape spot(sf::Vector2f(width, height));
        spot.setOrigin(width / 2, height / 2);
        spot.setPosition(x, y);
        spot.setRotation(targetAngle);
        spot.setFillColor(isParked ? successColor : spotColor);
        spot.setOutlineThickness(2);
        spot.setOutlineColor(sf::Color(50, 150, 50));
        window.draw(spot);
        
        // Draw target orientation arrow
        float arrowLength = width * 0.6f;
        float radians = targetAngle * 3.14159f / 180.0f;
        sf::Vertex arrow[] = {
            sf::Vertex(sf::Vector2f(x, y)),
            sf::Vertex(sf::Vector2f(
                x + std::cos(radians) * arrowLength,
                y + std::sin(radians) * arrowLength
            ))
        };
        arrow[0].color = sf::Color(0, 100, 0);
        arrow[1].color = sf::Color(0, 100, 0);
        window.draw(arrow, 2, sf::Lines);
    }

    bool checkIfParked(const SemiTruck& semiTruck) {
        //Check position distance from center
        float dx = semiTruck.cab_x - x;
        float dy = semiTruck.cab_y - y;
        float distance = std::sqrt(dx * dx + dy * dy);

        // Check angle
        float angleDiff = std::abs(semiTruck.cab_angle - targetAngle);
        while (angleDiff > 180.0f) angleDiff = 360.0f - angleDiff;

        // Check speed (car should be stopped when parked)
        bool positionCorrect = distance < positionTolerance;
        bool angleCorrect = angleDiff < angleTolerance;
        bool speedCorrect = std::abs(semiTruck.cab_speed) < speedTolerance;

        isParked = positionCorrect && angleCorrect && speedCorrect;
        return isParked;
    }

    float getPositionError(const Car& car) const {
        float dx = car.x - x;
        float dy = car.y - y;
        return std::sqrt(dx * dx + dy * dy);
    }

    float getAngleError(const Car& car) const {
        float angleDiff = std::abs(car.angle - targetAngle);
        while (angleDiff > 180.0f) angleDiff = 360.0f - angleDiff;
        return angleDiff;
    }

};



#endif