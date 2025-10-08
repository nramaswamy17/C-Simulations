#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <SFML/Graphics.hpp>
#include "Car.h"
#include "SemiTruck.h"

class Environment {
    public:
        float width, height;
        float wallThickness;
        sf::Color wallColor;
        sf::Color groundColor;
    
    Environment(float w, float h) {
        width = w;
        height = h;
        wallThickness = 20.0f;
        wallColor = sf::Color(60, 60, 60);
        groundColor = sf::Color(240, 240, 240);
    }

    void draw(sf::RenderWindow& window) {
        // Draw ground
        sf::RectangleShape ground(sf::Vector2f(width, height));
        ground.setFillColor(groundColor);
        window.draw(ground);

        // Draw walls
        //Top wall
        sf::RectangleShape topWall(sf::Vector2f(width, wallThickness));
        topWall.setFillColor(wallColor);
        topWall.setPosition(0, 0);
        window.draw(topWall);

        // Bottom Wall
        sf::RectangleShape bottomWall(sf::Vector2f(width, wallThickness));
        bottomWall.setFillColor(wallColor);
        bottomWall.setPosition(0, height - wallThickness);
        window.draw(bottomWall);
        
        // Left wall
        sf::RectangleShape leftWall(sf::Vector2f(wallThickness, height));
        leftWall.setFillColor(wallColor);
        leftWall.setPosition(0, 0);
        window.draw(leftWall);
        
        // Right wall
        sf::RectangleShape rightWall(sf::Vector2f(wallThickness, height));
        rightWall.setFillColor(wallColor);
        rightWall.setPosition(width - wallThickness, 0);
        window.draw(rightWall);
    }

    void handleCarCollision(Car & car) {
        float halfWidth = car.width / 2;
        float halfHeight = car.height / 2;

        bool collisionOccurred = false;

        // Left wall
        if (car.x - halfWidth < wallThickness) {
            car.x = wallThickness + halfWidth;
            car.speed *= -.5f;
            collisionOccurred = true;
        }

        // Right wall
        if (car.x + halfWidth > width - wallThickness) {
            car.x = width - wallThickness - halfWidth;
            car.speed *= -.5f;
            collisionOccurred = true;
        }

        // Top wall
        if (car.y - halfHeight < wallThickness) {
            car.y = wallThickness + halfHeight;
            car.speed *= -.5f;
            collisionOccurred = true;
        }

        // Bottom wall
        if (car.y + halfHeight > height - wallThickness) {
            car.y = height - wallThickness - halfHeight;
            car.speed *= -.5f;
            collisionOccurred = true;
        }

        // Pass collision to car object
        if (collisionOccurred) {
            car.onCollision();
        }
    }
    /**/
    void handleSemiCollision(SemiTruck & semiTruck) {
        /*
        Need to handle both the cab and the trailer collisions

        */

        bool collisionOccurred = false;

        // Half dimensions for simplified collision model
        float cab_maxExtent = std::max(semiTruck.cab_length, 30.0f) / 2.0f + 5.0f;
        float trailer_maxExtend = std::max(semiTruck.trailer_length, 25.0f) / 2.0f + 5.0f;

        // CAB Collision calculation

        // Left wall
        if (semiTruck.cab_x - cab_maxExtent < wallThickness) {
            semiTruck.cab_x = wallThickness + cab_maxExtent;
            semiTruck.cab_speed *= -0.5f;
            collisionOccurred = true;
        }

        // Right wall
        if (semiTruck.cab_x + cab_maxExtent > width - wallThickness) {
            semiTruck.cab_x = width - wallThickness - cab_maxExtent;
            semiTruck.cab_speed *= -0.5f;
            collisionOccurred = true;
        }

        // Top wall
        if (semiTruck.cab_y - cab_maxExtent < wallThickness) {
            semiTruck.cab_y = wallThickness + cab_maxExtent;
            semiTruck.cab_speed *= -0.5f;
            collisionOccurred = true;
        }
        
        // Bottom wall
        if (semiTruck.cab_y + cab_maxExtent > height - wallThickness) {
            semiTruck.cab_y = height - wallThickness - cab_maxExtent;
            semiTruck.cab_speed *= -0.5f;
            collisionOccurred = true;
        }

        // TRAILER Collision calculation
        // Left wall
        if (semiTruck.trailer_x - trailer_maxExtend < wallThickness) {
            semiTruck.trailer_x = wallThickness + trailer_maxExtend;
            semiTruck.cab_speed *= -0.5f;
            collisionOccurred = true;
        }

        // Right wall
        if (semiTruck.trailer_x + trailer_maxExtend > width - wallThickness) {
            semiTruck.trailer_x = width - wallThickness - trailer_maxExtend;
            semiTruck.cab_speed *= -0.5f;
            collisionOccurred = true;
        }

        // Top wall
        if (semiTruck.trailer_y - trailer_maxExtend < wallThickness) {
            semiTruck.trailer_y = wallThickness + trailer_maxExtend;
            semiTruck.cab_speed *= -0.5f;
            collisionOccurred = true;
        }

        // Bottom wall
        if (semiTruck.trailer_y + trailer_maxExtend > height - wallThickness) {
            float temp = semiTruck.trailer_y;
            semiTruck.trailer_y = height - wallThickness - trailer_maxExtend;
            semiTruck.cab_speed *= -0.5f;
            collisionOccurred = true;
            std::cout<<"Collision! " << semiTruck.trailer_y << "| " << temp << std::endl;
        }

        // Pass collision to semiTruck object
        if (collisionOccurred) {
            semiTruck.onCollision();
        }
    }

};


#endif