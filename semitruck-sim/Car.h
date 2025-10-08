#ifndef CAR_H
#define CAR_H

#include <SFML/Graphics.hpp>
#include <cmath>

class Car {
    public: 
        float x, y;
        float angle;
        float width, height;
        float speed;
        float maxSpeed;
        float acceleration;
        float friction;
        float turnRate;
        sf::Color color;

        // Collision metrics
        bool isColliding;
        sf::Clock collisionTimer;
        float collisionDisplayTime;

    // initialize car object
    Car(float px, float py, float w, float h, sf::Color c) {
        x = px;
        y = py;
        width = w;
        height = h;
        angle = 0.0f;  
        speed = 0.0f;
        maxSpeed = 300.0f;
        acceleration = 400.0f;
        friction = 0.95f;
        turnRate = 180.0f;
        color = c;
        isColliding = false;
        collisionDisplayTime = 2.0f;
    }

    void onCollision() {
        isColliding = true;
        collisionTimer.restart();
    }

    void handleInput(float dt) {
        // Fwd acceleration
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
            speed += acceleration * dt;
        }
        
        // Reverse acceleration
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
            speed -= acceleration * dt;
        }
        
        // only consider turning when car is moving (in reality this should be a dfq)
        if (std::abs(speed) > .5f) {
            // Turn left
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
                angle -= turnRate * dt * (speed / maxSpeed);
            }
        
            // Turn Right
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
                angle += turnRate * dt * (speed / maxSpeed);
            }

            // Constrain angle to 0 -> 360
            if (angle < 0) {angle += 360;}
            else if (angle > 360) {angle -= 360;}
        }

        // Speed limit upper & lower bound
        if (speed > maxSpeed) speed = maxSpeed;
        if (speed < -maxSpeed * .5f) speed = -maxSpeed * .5f;
    }

    void update(float dt) {
        // Apply friction
        speed *= friction;

        // if speed is below threshold, set to 0
        if (std::abs(speed) < .5f) {
            speed = 0.0f;
        }

        // Update position based on angle and speed 
        float radians = angle * M_PI / 180.0f;
        x += std::cos(radians) * speed * dt;
        y += std::sin(radians) * speed * dt;

        // Reset collision flag after time eperiod
        if (isColliding && collisionTimer.getElapsedTime().asSeconds() > collisionDisplayTime){
            isColliding = false;
        }
    }

    void draw(sf::RenderWindow& window) {
        sf::RectangleShape rect(sf::Vector2f(width, height));
        rect.setOrigin(width / 2, height / 2); // Center origin for rotation
        rect.setPosition(x, y);
        rect.setRotation(angle);
        rect.setFillColor(color);
        rect.setOutlineThickness(2);
        
        // Change outline color during collision 
        if (isColliding) {
            rect.setOutlineColor(sf::Color::Red);
        } else {
            rect.setOutlineColor(sf::Color::Black);
        }
        window.draw(rect);
        
        // Draw direction indicator (front of car)
        float radians = angle * 3.14159f / 180.0f;
        float indicatorLength = width * 0.6f;
        sf::Vertex line[] = {
            sf::Vertex(sf::Vector2f(x, y)),
            sf::Vertex(sf::Vector2f(
                x + std::cos(radians) * indicatorLength,
                y + std::sin(radians) * indicatorLength
            ))
        };
        line[0].color = sf::Color::Yellow;
        line[1].color = sf::Color::Yellow;
        window.draw(line, 2, sf::Lines);
    }

    // Get the four corners of the car for collision detection
    sf::Vector2f getCenter() const {
        return sf::Vector2f(x, y);
    }
};


#endif // CAR_H