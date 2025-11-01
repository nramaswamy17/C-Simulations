#include <iostream>
#include <cmath>
#include "kinematics.h"
#include "controller.h"
#include "visualize.h"

int main() {
    // Setup
    Kinematics robot(2.0, 1.0, 0.5);  // 1 meter arm

    // 2 PID controllers for each joint
    PIDController pid1(5.0, 0.1, 0.5);
    PIDController pid2(5.0, 0.1, 0.5);
    PIDController pid3(5.0, 0.1, 0.5);
    
    double theta1 = 0.0, theta2 = 0.0, theta3 = 0.0;
    double vel1 = 0.0, vel2 = 0.0, vel3 = 0.0;
    double dt = 0.01;  // 10ms timestep
    
    // Get target from user
    Position target;
    target.x = -2;
    target.y = 1;
    double phi = 110; // Absolute target angle of end effector
    phi *= (M_PI / 180);

    std::cout << "Target: (" << target.x << ", " << target.y << ")\n"; 
    
    JointAngles target_angles = robot.inverse(target, phi);
    std::cout << "Target angles: theta1=" << target_angles.theta1 * (180/M_PI) 
              << " deg, theta2=" << target_angles.theta2 * (180/M_PI)
              << " deg, theta3=" << target_angles.theta3 * (180/M_PI) << " deg\n\n";
    
    // DEBUG
    std::cout << "Checking wrist position:\n";
    double x_w = target.x - robot.L3 * cos(phi);
    double y_w = target.y - robot.L3 * sin(phi);
    std::cout << "  Wrist should be at: (" << x_w << ", " << y_w << ")\n";
    double dist = sqrt(x_w*x_w + y_w*y_w);
    std::cout << "  Distance to wrist: " << dist << " (max reach: " << robot.L1 + robot.L2 << ")\n";

    // Create visualizer
    float space_size = 10.0;
    RobotVisualizer viz(600, space_size);
    
    // Simulation loop
    double t = 0.0;
    bool reached_target = false;
    
    while (viz.isOpen()) {
        // Handle window events
        viz.handleEvents();
        
        // Continue simulation if not reached target
        if (!reached_target && t < 5) {

            // PID control
            double control1 = pid1.compute(theta1, target_angles.theta1, dt);
            double control2 = pid2.compute(theta2, target_angles.theta2, dt);
            double control3 = pid3.compute(theta3, target_angles.theta3, dt);

            // Update motion
            vel1 += control1 * dt;
            vel2 += control2 * dt;
            vel3 += control3 * dt;
            
            // Damping
            vel1 *= .95;
            vel2 *= .95;
            vel3 *= .95;

            // Update angles using velocity
            theta1 += vel1 * dt;
            theta2 += vel2 * dt;
            theta3 += vel3 * dt;
            
            /*
            std::cout << "Control1: " << control1 
                      << "Control2: " << control2
                      << "\nVel1: " << vel1
                      << "Vel2: " << vel2;
            */
            // Print every 0.5 seconds
            if (fmod(t, 0.5) < dt) {
                Result result = robot.forward(theta1, theta2, theta3);
                std::cout << "t=" << t 
                          << " theta1=" << theta1 * (180/M_PI) 
                          << " theta2=" << theta2 * (180/M_PI) 
                          << " theta3=" << theta3 * (180/M_PI) 
                          << " pos=(" << result.pos2.x << "," << result.pos2.y << ")\n";
            }
            
            // Check if reached target
            if (fabs(target_angles.theta1 - theta1) < 0.001 && 
                fabs(target_angles.theta2 - theta2) < .001 &&
                fabs(target_angles.theta3 - theta3) < .001) {
                std::cout << "\nReached target! (Close window to exit)\n";
                reached_target = true;
            }
            
            t += dt;
        }
        
        // Draw current state (keep visualizing even after reaching target)
        viz.draw(theta1, theta2, theta3, target, robot.L1, robot.L2, robot.L3);
    }
    
    return 0;
}