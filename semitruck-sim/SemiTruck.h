#ifndef SEMITRUCK_H
#define SEMITRUCK_H

#include <SFML/Graphics.hpp>
#include <cmath>
#include <iostream>

class SemiTruck{
    public:
        // Cab (Front)
        float cab_x, cab_y, cab_angle, cab_speed;

        // Trailer state
        float trailer_x, trailer_y, trailer_angle;

        // Dimensions
        float cab_length, trailer_length;
        float hitch_distance_from_cab_rear; // Where is the hitch located
        float hitch_distance_from_trailer_front; // Where hitch is on trailer

        // Steering control
        float maxSpeed;
        float acceleration;
        float friction;
        float turnRate;

        // Collision
        bool isColliding;
        sf::Clock collisionTimer;
        float collisionDisplayTime;
        bool isJackknifed;

        // Sensor system
        int numSensors;
        std::vector<float> sensorAngles; // Offset of each sensor from cab angle
        std::vector<float> sensorDistances; // Readings
        float maxSensorRange;

    SemiTruck(float start_x, float start_y, float start_angle, float start_speed){
        // Vehicle starting position / speed
        cab_x = start_x;
        cab_y = start_y;
        cab_angle = start_angle;
        cab_speed = start_speed;

        // Vehicle sizes
        cab_length = 40.0f;
        trailer_length = 80.0f;

        // Hitch location
        hitch_distance_from_cab_rear = cab_length / 2; // at the back 
        hitch_distance_from_trailer_front = trailer_length / 2; // at the front

        // Control parameters
        maxSpeed = 200.0f;
        acceleration = 300.0f;
        friction = 0.95f;
        turnRate = 120.0f;  // Slower turning than car (trucks turn slower!)

        // Position the trailer behind the cab initially
        trailer_x = cab_x - (cab_length/2 + trailer_length/2);
        trailer_y = cab_y;
        trailer_angle = 0.0f;

        // Collision
        isColliding = false;
        collisionDisplayTime = 2.0f;
        isJackknifed = false;

        // Sensors
        numSensors = 8;
        maxSensorRange = 200.0f;
        sensorDistances.resize(numSensors, maxSensorRange);

        // Sensor angles relative to cab: front, front-right, right, back-right, etc.
        for (int i = 0; i < numSensors; i++) {
            sensorAngles.push_back(i * 45.0f);  // 45 degree increments
        }
    }

    void onCollision(){
        isColliding = true;
        collisionTimer.restart();
        
    }

    void handleInput(float dt) {
        // W - Accelerate forward
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
            cab_speed += acceleration * dt;
        }
        
        // S - Brake/Reverse
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
            cab_speed -= acceleration * dt;
        }
        
        // A - Turn left (only when moving)
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) && std::abs(cab_speed) > 10.0f) {
            cab_angle -= turnRate * dt * (cab_speed / maxSpeed);
            // Normalize angle to 0-360
            while (cab_angle < 0.0f) cab_angle += 360.0f;
            while (cab_angle >= 360.0f) cab_angle -= 360.0f;
        }
        
        // D - Turn right (only when moving)
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) && std::abs(cab_speed) > 10.0f) {
            cab_angle += turnRate * dt * (cab_speed / maxSpeed);
            // Normalize angle to 0-360
            while (cab_angle < 0.0f) cab_angle += 360.0f;
            while (cab_angle >= 360.0f) cab_angle -= 360.0f;
        }
        
        // Clamp speed
        if (cab_speed > maxSpeed) cab_speed = maxSpeed;
        if (cab_speed < -maxSpeed * 0.5f) cab_speed = -maxSpeed * 0.5f;
    }

    // Physics
    void updateCab(float dt){
        // Update position based on angle and speed 
        float radians = cab_angle * M_PI / 180.0f;
        cab_x += std::cos(radians) * cab_speed * dt;
        cab_y += std::sin(radians) * cab_speed * dt;
    }

    void updateTrailer(float dt) {
        // Calculate hitch position
        float radians = cab_angle * M_PI / 180.0f;
        float hitch_x = cab_x - cos(radians) * hitch_distance_from_cab_rear;
        float hitch_y = cab_y - sin(radians) * hitch_distance_from_cab_rear;
        
        // Calculate trailer angular velocity
        float angle_diff = cab_angle - trailer_angle;
        while (angle_diff > 180.0f) angle_diff -= 360.0f;
        while (angle_diff < -180.0f) angle_diff += 360.0f;
        
        // Trailer dynamics
        float angular_velocity = (cab_speed / hitch_distance_from_trailer_front) 
                                * sin(angle_diff * M_PI / 180.0f);
        
        // Update trailer angle
        trailer_angle += angular_velocity * 180.0f / M_PI * dt;
        
        // Update trailer position to keep it at the hitch point
        float trailer_radians = trailer_angle * M_PI / 180.0f;
        trailer_x = hitch_x - cos(trailer_radians) * hitch_distance_from_trailer_front;
        trailer_y = hitch_y - sin(trailer_radians) * hitch_distance_from_trailer_front;
    }

    void update(float dt) {
        // Friction
        cab_speed *= friction;

        // Stop if speed goes below 1 px/sec
        if (std::abs(cab_speed) < 1.0f) cab_speed = 0.0f;

        // Update cab and trailer
        updateCab(dt);
        updateTrailer(dt);

        // Reset collision flag after time period
        if (isColliding && collisionTimer.getElapsedTime().asSeconds() > collisionDisplayTime){
            isColliding = false;
        }
    }

    void updateSensors(float envWidth, float envHeight, float wallThickness) {
        for (int i = 0; i < numSensors; i++) {
            float sensorWorldAngle = cab_angle + sensorAngles[i];
            float radians = sensorWorldAngle * M_PI / 180.0f;

            // ray from cab to max sensor range
            float minDist = maxSensorRange;

            // Check intersections with each wall
            // Left wall 
            if (std::cos(radians) < 0) {
                float dist = (cab_x - wallThickness) / -std::cos(radians);
                if (dist > 0 && dist < minDist) minDist = dist;
            }
            // Right wall
            if (std::cos(radians) > 0) {
                float dist = (envWidth - wallThickness - cab_x) / std::cos(radians);
                if (dist > 0 && dist < minDist) minDist = dist;
            }
            // Top wall
            if (std::sin(radians) < 0) {
                float dist = (cab_y - wallThickness) / -std::sin(radians);
                if (dist > 0 && dist < minDist) minDist = dist;
            }
            // Bottom wall
            if (std::sin(radians) > 0) {
                float dist = (envHeight - wallThickness - cab_y) / std::sin(radians);
                if (dist > 0 && dist < minDist) minDist = dist;
            }

            sensorDistances[i] = minDist;
        }
    }

    void draw(sf::RenderWindow& window) {
        // Draw trailer first (so it appears behind cab)
        sf::RectangleShape trailerRect(sf::Vector2f(trailer_length, 25.0f));
        trailerRect.setOrigin(trailer_length / 2, 12.5f);
        trailerRect.setPosition(trailer_x, trailer_y);
        trailerRect.setRotation(trailer_angle);
        trailerRect.setFillColor(sf::Color(200, 200, 200)); // Light gray
        trailerRect.setOutlineThickness(2);
        trailerRect.setOutlineColor(sf::Color::Black);
        if (isColliding) trailerRect.setOutlineColor(sf::Color::Red);
        else trailerRect.setOutlineColor(sf::Color::Black);
        window.draw(trailerRect);
        
        // Draw cab
        sf::RectangleShape cabRect(sf::Vector2f(cab_length, 30.0f));
        cabRect.setOrigin(cab_length / 2, 15.0f);
        cabRect.setPosition(cab_x, cab_y);
        cabRect.setRotation(cab_angle);
        cabRect.setFillColor(sf::Color(220, 50, 50)); // Red
        cabRect.setOutlineThickness(2);
        if (isColliding) cabRect.setOutlineColor(sf::Color::Red);
        else cabRect.setOutlineColor(sf::Color::Black);
        window.draw(cabRect);
        
        // Draw direction indicator on cab (yellow arrow)
        float radians = cab_angle * M_PI / 180.0f;
        float indicatorLength = cab_length * 0.6f;
        sf::Vertex line[] = {
            sf::Vertex(sf::Vector2f(cab_x, cab_y)),
            sf::Vertex(sf::Vector2f(
                cab_x + std::cos(radians) * indicatorLength,
                cab_y + std::sin(radians) * indicatorLength
            ))
        };
        line[0].color = sf::Color::Yellow;
        line[1].color = sf::Color::Yellow;
        window.draw(line, 2, sf::Lines);
        
        // Draw hitch point
        float hitch_radians = cab_angle * M_PI / 180.0f;
        float hitch_x = cab_x - std::cos(hitch_radians) * hitch_distance_from_cab_rear;
        float hitch_y = cab_y - std::sin(hitch_radians) * hitch_distance_from_cab_rear;
        
        sf::CircleShape hitchPoint(5.0f);
        hitchPoint.setOrigin(5.0f, 5.0f);
        hitchPoint.setPosition(hitch_x, hitch_y);
        hitchPoint.setFillColor(sf::Color::Green);
        window.draw(hitchPoint);

        // Draw sensor rays
        for (int i = 0; i < numSensors; i++) {
            float sensorWorldAngle = cab_angle + sensorAngles[i];
            float radians = sensorWorldAngle * M_PI / 180.0f;

            float endX = cab_x + std::cos(radians) * sensorDistances[i];
            float endY = cab_y + std::sin(radians) * sensorDistances[i];

            sf::Vertex sensorLine[] = {
                sf::Vertex(sf::Vector2f(cab_x, cab_y)),
                sf::Vertex(sf::Vector2f(endX, endY))
            };

            // Color based on distance (green = far, red = close)
            float intensity = sensorDistances[i] / maxSensorRange;
            sf::Color sensorColor(255 * (1 - intensity), 255 * intensity, 0, 100);
            sensorLine[0].color = sensorColor;
            sensorLine[1].color = sensorColor;
            window.draw(sensorLine, 2, sf::Lines);
        }
    }

};

#endif