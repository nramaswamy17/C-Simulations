#include <iostream>
#include <cmath>
#include "kinematics.h"
#include "controller.h"
#include "visualize.h"

int main() {
    // Setup
    Kinematics_single robot(1.0);  // 1 meter arm
    PIDController pid(5.0, 0.1, 0.5);  // P, I, D gains
    
    double angle = 0.0;
    double velocity = 0.0;
    double dt = 0.01;  // 10ms timestep
    
    // Get target from user
    Position target;
    target.x = .5;
    target.y = .5;
    std::cout << "Target: (" << target.x << ", " << target.y << ")\n"; 
    
    double target_angle = robot.inverse(target);
    std::cout << "Target angle: " << target_angle * (180/M_PI) << " deg\n\n";
    
    // Create visualizer
    RobotVisualizer viz(800, 3.0);
    
    // Simulation loop
    double t = 0.0;
    bool reached_target = false;
    
    while (viz.isOpen()) {
        // Handle window events
        viz.handleEvents();
        
        // Continue simulation if not reached target
        if (!reached_target && t < 5.0) {
            // PID control
            double control = pid.compute(angle, target_angle, dt);
            
            // Update motion
            velocity += control * dt;
            velocity *= 0.95;  // damping
            angle += velocity * dt;
            
            // Print every 0.5 seconds
            if (fmod(t, 0.5) < dt) {
                Position pos = robot.forward(angle);
                std::cout << "t=" << t 
                          << " angle=" << angle * (180/M_PI) 
                          << " pos=(" << pos.x << "," << pos.y << ")\n";
            }
            
            // Check if reached target
            if (fabs(target_angle - angle) < 0.001) {
                std::cout << "\nReached target! (Close window to exit)\n";
                reached_target = true;
            }
            
            t += dt;
        }

        if (t > 2) {
            target.x = -1; 
            target.y = 1;
            target_angle = robot.inverse(target);
            double control = pid.compute(angle, target_angle, dt);
        }
        
        // Draw current state (keep visualizing even after reaching target)
        viz.draw(angle, target, robot.link_length);
    }
    
    return 0;
}