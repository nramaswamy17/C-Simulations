#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <SFML/Graphics.hpp>
#include "Car.h"
#include "SemiTruck.h"
#include "Lane.h"

class Environment {
    public:
        float width, height;
        float wallThickness;
        sf::Color wallColor;
        sf::Color groundColor;
        Road* road;  // Pointer to road with lanes
    
    Environment(float w, float h) {
        width = w;
        height = h;
        wallThickness = 20.0f;
        wallColor = sf::Color(60, 60, 60);
        groundColor = sf::Color(34, 139, 34);  // Grass green for sides
        road = nullptr;
    }
    
    ~Environment() {
        delete road;
    }
    
    void setRoad(Road* r) {
        road = r;
    }

    void draw(sf::RenderWindow& window) {
        // Draw ground (grass everywhere as base)
        sf::RectangleShape ground(sf::Vector2f(width, height));
        ground.setFillColor(groundColor);
        window.draw(ground);
        
        // Draw road if it exists (road will draw its own background)
        if (road) {
            road->draw(window);
        }

        // Draw walls (barriers on edges) - make them look like barriers/fences
        sf::Color barrierColor = sf::Color(180, 50, 50); // Red barriers
        
        // Top wall
        sf::RectangleShape topWall(sf::Vector2f(width, wallThickness));
        topWall.setFillColor(barrierColor);
        topWall.setPosition(0, 0);
        window.draw(topWall);

        // Bottom Wall
        sf::RectangleShape bottomWall(sf::Vector2f(width, wallThickness));
        bottomWall.setFillColor(barrierColor);
        bottomWall.setPosition(0, height - wallThickness);
        window.draw(bottomWall);
        
        // Left wall
        sf::RectangleShape leftWall(sf::Vector2f(wallThickness, height));
        leftWall.setFillColor(barrierColor);
        leftWall.setPosition(0, 0);
        window.draw(leftWall);
        
        // Right wall
        sf::RectangleShape rightWall(sf::Vector2f(wallThickness, height));
        rightWall.setFillColor(barrierColor);
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
    
    void handleSemiCollision(SemiTruck & semiTruck) {
        bool collisionOccurred = false;

        // Calculate true corners of rotated cab and trailer
        float cab_radians = semiTruck.cab_angle * M_PI / 180.0f;
        float trailer_radians = semiTruck.trailer_angle *  M_PI / 180.0f;

        // Cab dimensions
        float cab_half_length = semiTruck.cab_length / 2.0f;
        float cab_half_width = 15.0f;

        // Trailer dimensions
        float trailer_half_length = semiTruck.trailer_length / 2.0f;
        float trailer_half_width = 12.5f;

        // Calculate cab corners (4 corners of a rectangle)
        float cab_corners_x[4], cab_corners_y[4];
        float cos_cab = std::cos(cab_radians);
        float sin_cab = std::sin(cab_radians);

        // APPLY ROTATION MATRIX
        // Front right corner
        cab_corners_x[0] = semiTruck.cab_x + cos_cab * cab_half_length - sin_cab * cab_half_width;
        cab_corners_y[0] = semiTruck.cab_y + sin_cab * cab_half_length + cos_cab * cab_half_width;
        
        // Front left corner
        cab_corners_x[1] = semiTruck.cab_x + cos_cab * cab_half_length + sin_cab * cab_half_width;
        cab_corners_y[1] = semiTruck.cab_y + sin_cab * cab_half_length - cos_cab * cab_half_width;
        
        // Back left corner
        cab_corners_x[2] = semiTruck.cab_x - cos_cab * cab_half_length + sin_cab * cab_half_width;
        cab_corners_y[2] = semiTruck.cab_y - sin_cab * cab_half_length - cos_cab * cab_half_width;
        
        // Back right corner
        cab_corners_x[3] = semiTruck.cab_x - cos_cab * cab_half_length - sin_cab * cab_half_width;
        cab_corners_y[3] = semiTruck.cab_y - sin_cab * cab_half_length + cos_cab * cab_half_width;
    
        // Calculate trailer corners
        float trailer_corners_x[4], trailer_corners_y[4];
        float cos_trailer = std::cos(trailer_radians);
        float sin_trailer = std::sin(trailer_radians);

        // Apply rotation matrix
        trailer_corners_x[0] = semiTruck.trailer_x + cos_trailer * trailer_half_length - sin_trailer * trailer_half_width;
        trailer_corners_y[0] = semiTruck.trailer_y + sin_trailer * trailer_half_length + cos_trailer * trailer_half_width;
        trailer_corners_x[1] = semiTruck.trailer_x + cos_trailer * trailer_half_length + sin_trailer * trailer_half_width;
        trailer_corners_y[1] = semiTruck.trailer_y + sin_trailer * trailer_half_length - cos_trailer * trailer_half_width;
        trailer_corners_x[2] = semiTruck.trailer_x - cos_trailer * trailer_half_length + sin_trailer * trailer_half_width;
        trailer_corners_y[2] = semiTruck.trailer_y - sin_trailer * trailer_half_length - cos_trailer * trailer_half_width;
        trailer_corners_x[3] = semiTruck.trailer_x - cos_trailer * trailer_half_length - sin_trailer * trailer_half_width;
        trailer_corners_y[3] = semiTruck.trailer_y - sin_trailer * trailer_half_length + cos_trailer * trailer_half_width;
        

        // Check cab corners against walls
        for (int i = 0; i < 4; i++) {
            // Left wall
            if (cab_corners_x[i] < wallThickness) {
                semiTruck.cab_x += (wallThickness - cab_corners_x[i]);
                semiTruck.cab_speed *= -.5f;
                collisionOccurred = true;
            } 
            // Right wall
            if (cab_corners_x[i] > width - wallThickness) {
                semiTruck.cab_x -= (cab_corners_x[i] - (width - wallThickness));
                semiTruck.cab_speed *= -0.5f;
                collisionOccurred = true;
            }
            // Top wall
            if (cab_corners_y[i] < wallThickness) {
                semiTruck.cab_y += (wallThickness - cab_corners_y[i]);
                semiTruck.cab_speed *= -0.5f;
                collisionOccurred = true;
            }
            // Bottom wall
            if (cab_corners_y[i] > height - wallThickness) {
                semiTruck.cab_y -= (cab_corners_y[i] - (height - wallThickness));
                semiTruck.cab_speed *= -0.5f;
                collisionOccurred = true;
            }
        }

        // Check all trailer corners against walls
        for (int i = 0; i < 4; i++) {
            // Left wall
            if (trailer_corners_x[i] < wallThickness) {
                semiTruck.cab_speed *= -0.5f;
                collisionOccurred = true;
            }
            // Right wall
            if (trailer_corners_x[i] > width - wallThickness) {
                semiTruck.cab_speed *= -0.5f;
                collisionOccurred = true;
            }
            // Top wall
            if (trailer_corners_y[i] < wallThickness) {
                semiTruck.cab_speed *= -0.5f;
                collisionOccurred = true;
            }
            // Bottom wall
            if (trailer_corners_y[i] > height - wallThickness) {
                semiTruck.cab_speed *= -0.5f;
                collisionOccurred = true;
            }
        }

        // Pass collision to semiTruck object
        if (collisionOccurred) {
            semiTruck.onCollision();
        }

    }

};

#endif