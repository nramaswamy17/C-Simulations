#ifndef VISUALIZE_H
#define VISUALIZE_H

#include <SFML/Graphics.hpp>
#include "kinematics.h"

class RobotVisualizer {
private:
    sf::RenderWindow window;
    float scale;  // pixels per meter
    sf::Vector2f origin;  // screen origin
    float space_size;
    
public:
    RobotVisualizer(float window_size = 800.0f, float meters_shown = 5.0f) 
        : window(sf::VideoMode(window_size, window_size), "Robot Arm Visualization")
    {
        space_size = meters_shown;
        scale = window_size / meters_shown;
        origin = sf::Vector2f(window_size / 2, window_size / 2);
        window.setFramerateLimit(60);
    }
    
    bool isOpen() {
        return window.isOpen();
    }
    
    void handleEvents() {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }
    }
    
    void draw(double angle, Position target, double link_length) {
        window.clear(sf::Color::White);
        
        // Draw grid
        drawGrid();
        
        // Draw target (red circle)
        sf::CircleShape targetCircle(8);
        targetCircle.setFillColor(sf::Color::Red);
        targetCircle.setOrigin(8, 8);
        sf::Vector2f targetPos = toScreen(target.x, target.y);
        targetCircle.setPosition(targetPos);
        window.draw(targetCircle);
        
        // Draw robot base (black circle)
        sf::CircleShape base(10);
        base.setFillColor(sf::Color::Black);
        base.setOrigin(10, 10);
        base.setPosition(origin);
        window.draw(base);
        
        // Calculate end effector position using forward kinematics
        Kinematics kin(link_length);
        Position end = kin.forward(angle);
        double end_x = end.x;
        double end_y = end.y;
        
        // Draw arm (blue line)
        sf::Vertex arm[] = {
            sf::Vertex(origin, sf::Color::Blue),
            sf::Vertex(toScreen(end_x, end_y), sf::Color::Blue)
        };
        window.draw(arm, 2, sf::Lines);
        
        // Draw end effector (blue circle)
        sf::CircleShape endEffector(6);
        endEffector.setFillColor(sf::Color::Blue);
        endEffector.setOrigin(6, 6);
        endEffector.setPosition(toScreen(end_x, end_y));
        window.draw(endEffector);
        
        window.display();
    }
    
private:
    sf::Vector2f toScreen(double x, double y) {
        // Convert world coordinates to screen coordinates
        // Note: SFML Y-axis points down, so we negate y
        return sf::Vector2f(origin.x + x * scale, origin.y - y * scale);
    }
    
    void drawGrid() {
        // Draw simple grid lines
        sf::Color gridColor(200, 200, 200);
        
        // Vertical and horizontal axes
        sf::Vertex xAxis[] = {
            sf::Vertex(sf::Vector2f(0, origin.y), sf::Color::Black),
            sf::Vertex(sf::Vector2f(window.getSize().x, origin.y), sf::Color::Black)
        };
        sf::Vertex yAxis[] = {
            sf::Vertex(sf::Vector2f(origin.x, 0), sf::Color::Black),
            sf::Vertex(sf::Vector2f(origin.x, window.getSize().y), sf::Color::Black)
        };
        window.draw(xAxis, 2, sf::Lines);
        window.draw(yAxis, 2, sf::Lines);
        
        // Grid lines every 0.5 meters
        for (float i = -space_size/2; i <= space_size/2; i += 0.5f) {
            if (fabs(i) < 0.01) continue; // skip center
            
            // Vertical lines
            sf::Vertex vline[] = {
                sf::Vertex(toScreen(i, -space_size/2), gridColor),
                sf::Vertex(toScreen(i, space_size/2), gridColor)
            };
            window.draw(vline, 2, sf::Lines);
            
            // Horizontal lines
            sf::Vertex hline[] = {
                sf::Vertex(toScreen(-space_size/2, i), gridColor),
                sf::Vertex(toScreen(space_size/2, i), gridColor)
            };
            window.draw(hline, 2, sf::Lines);
        }
    }
};

#endif