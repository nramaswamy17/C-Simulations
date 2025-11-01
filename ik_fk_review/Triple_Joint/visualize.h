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
    
    void draw(double theta1, double theta2, double theta3, Position target, double L1, double L2, double L3) {
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

        // Calculate end effector position using forward kinematics
        Kinematics kin(L1, L2, L3);
        Result result = kin.forward(theta1, theta2, theta3);

        double elbow_x = result.pos1.x;
        double elbow_y = result.pos1.y;
        double wrist_x = result.pos2.x;
        double wrist_y = result.pos2.y;
        double end_x = result.pos3.x;
        double end_y = result.pos3.y;

        // Draw base -> elbow link
        sf::Vertex link1[] = {
            sf::Vertex(origin, sf::Color::Blue),
            sf::Vertex(toScreen(elbow_x, elbow_y), sf::Color::Blue)
        };
        window.draw(link1, 2, sf::Lines);

        // Draw elbow -> wrist link
        sf::Vertex link2[] = {
            sf::Vertex(toScreen(elbow_x, elbow_y), sf::Color::Green),
            sf::Vertex(toScreen(wrist_x, wrist_y), sf::Color::Green)
        };
        window.draw(link2, 2, sf::Lines);

        // Draw wrist -> end effector link
        sf::Vertex link3[] = {
            sf::Vertex(toScreen(wrist_x, wrist_y), sf::Color::Green),
            sf::Vertex(toScreen(end_x, end_y), sf::Color::Green)
        };
        window.draw(link3, 2, sf::Lines);
        
        // Draw robot base (black circle)
        sf::CircleShape base(10);
        base.setFillColor(sf::Color::Black);
        base.setOrigin(10, 10);
        base.setPosition(origin);
        window.draw(base);
    
        
        // Draw elbow joint
        sf::CircleShape elbow(8);
        elbow.setFillColor(sf::Color::Blue);
        elbow.setOrigin(8,8);
        elbow.setPosition(toScreen(elbow_x, elbow_y));
        window.draw(elbow);
        
        // Draw end effector (green circle)
        sf::CircleShape wrist(8);
        wrist.setFillColor(sf::Color::Green);
        wrist.setOrigin(8, 8);
        wrist.setPosition(toScreen(wrist_x, wrist_y));
        window.draw(wrist);

        // Draw end effector 
        sf::CircleShape endEffector(6);
        endEffector.setFillColor(sf::Color::Magenta);
        endEffector.setOrigin(6, 6);
        endEffector.setPosition(toScreen(end_x, end_y));
        window.draw(endEffector);

        // Draw arrow
        double phi = theta1 + theta2 + theta3;
        double arrow_length = .5;
        double arrow_end_x = end_x + arrow_length * cos(phi);
        double arrow_end_y = end_y + arrow_length * sin(phi);

        sf::Vertex arrow[] = {
            sf::Vertex(toScreen(end_x, end_y), sf::Color(255, 140, 0)),
            sf::Vertex(toScreen(arrow_end_x, arrow_end_y), sf::Color(255, 140, 0))
        };
        window.draw(arrow, 2, sf::Lines);

        double arrowhead_length = .15;
        double arrowhead_angle = .4;

        double left_x = arrow_end_x - arrowhead_length * cos(phi - arrowhead_angle);
        double left_y = arrow_end_y - arrowhead_length * sin(phi - arrowhead_angle);
        double right_x = arrow_end_x - arrowhead_length * cos(phi + arrowhead_angle);
        double right_y = arrow_end_y - arrowhead_length * sin(phi + arrowhead_angle);
        
        sf::Vertex arrowhead_left[] = {
            sf::Vertex(toScreen(arrow_end_x, arrow_end_y), sf::Color(255, 140, 0)),
            sf::Vertex(toScreen(left_x, left_y), sf::Color(255, 140, 0))
        };
        sf::Vertex arrowhead_right[] = {
            sf::Vertex(toScreen(arrow_end_x, arrow_end_y), sf::Color(255, 140, 0)),
            sf::Vertex(toScreen(right_x, right_y), sf::Color(255, 140, 0))
        };
    
        window.draw(arrowhead_left, 2, sf::Lines);
        window.draw(arrowhead_right, 2, sf::Lines);

        
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