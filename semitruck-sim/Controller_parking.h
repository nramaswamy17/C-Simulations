#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <cmath>
#include <iostream>
#include "SemiTruck.h"
#include "ParkingSpot.h"

enum ParkingState {
    APPROACH,
    ALIGN,
    BACK_IN,
    ADJUST,
    PARKED
};

class Controller {
    public:
        ParkingState currentState;
        bool isEnabled;

        // Target position
        float alignTargetX, alignTargetY;
        float alignTargetAngle;

        // Control parameters
        float approachSpeed;
        float alignSpeed;
        float backSpeed;
        float minSensorDistance;

        Controller() {
            currentState = APPROACH;
            isEnabled = false;
            approachSpeed = 100.0f;
            alignSpeed = 80.0f;
            backSpeed = 60.0f;
            minSensorDistance = 30.0f;
        }

        void enable() {
            isEnabled = true;
            currentState = APPROACH;
        }

        void disable() {
            isEnabled = false;
        }

        void toggle() {
            isEnabled = !isEnabled;
            if (isEnabled) {
                currentState = APPROACH;
            }
        }

        // Calc alignment position
        void calculateAlignmentTarget(const ParkingSpot& spot) {
            float radians = spot.targetAngle * M_PI / 180.0f;

            // Position ~150px in front of the parking spot
            float offsetDistance = 150.0f;
            alignTargetX = spot.x - std::cos(radians) * offsetDistance;
            alignTargetY = spot.y - std::sin(radians) * offsetDistance;
            alignTargetAngle = spot.targetAngle;
        }

        // Main control loop
        void update(SemiTruck& truck, const ParkingSpot& spot, float dt) {
            if (!isEnabled) return;
            
            // Check if already parked
            if (spot.isParked) {
                currentState = PARKED;
                return;
            }

            switch (currentState) {
                case APPROACH:
                    handleApproach(truck, spot, dt);
                    break;
                case ALIGN:
                    handleAlign(truck, spot, dt);
                    break;
                case BACK_IN:
                    handleBackIn(truck, spot, dt);
                    break;
                case ADJUST:
                    handleAdjust(truck, spot, dt);
                    break;
                case PARKED:
                    break;
            }
        }

    private:
        void handleApproach(SemiTruck& truck, const ParkingSpot spot, float dt) {
            calculateAlignmentTarget(spot);

            // calculate distance to alignment position
            float dx = alignTargetX - truck.cab_x;
            float dy = alignTargetY - truck.cab_y;
            float distance = std::sqrt(dx * dx + dy * dy);

            float desiredAngle = std::atan2(dy, dx) * 180.0f / M_PI;

            // Convert angles to correct range
            while (desiredAngle < 0) desiredAngle += 360.0f;
            while (desiredAngle >= 360.0f) desiredAngle -= 360.0f;

            float angleDiff = desiredAngle - truck.cab_angle;
            while (angleDiff > 180.0f) angleDiff -= 360.0f;
            while (angleDiff < -180.0f) angleDiff += 360.0f;

            // Control logic
            bool closeToWall = checkSensorProximity(truck);

            if (distance < 50.0f) {
                // Close distance so transition to align
                currentState = ALIGN;
                std::cout << "State: APPROACH -> ALIGN" << std::endl;
            } else if (closeToWall) {
                // To close to wall, slow down and adjust
                truck.cab_speed = approachSpeed * .3f;
                if (angleDiff > 5.0f) {
                    simulateKeyPress(truck, 'D', dt);
                } else if (angleDiff < -5.0f) {
                    simulateKeyPress(truck, 'A', dt);
                }
            } else {
                // Normal approach
                truck.cab_speed = approachSpeed;

                // Steer towards target
                if (angleDiff > 10.0f) {
                    simulateKeyPress(truck, 'D', dt);
                } else if (angleDiff < -10.0f) {
                    simulateKeyPress(truck, 'A', dt);
                }
            }
        }
        void handleAlign(SemiTruck& truck, const ParkingSpot spot, float dt) {
            // Get angle error
            float angleDiff = alignTargetAngle - truck.cab_angle;
            while (angleDiff > 180.0f) angleDiff -= 360.0f;
            while (angleDiff < -180.0f) angleDiff += 360.0f;

            // if alignment is close, start back in
            if (std::abs(angleDiff) < 15.0f) {
                currentState = BACK_IN;
                std::cout << "State: ALIGN -> BACK_IN" << std::endl;
                return;
            }

            // Rotate in place (very slow speed for turning)
            truck.cab_speed = 15.0f;
            if (angleDiff > 0) {
                simulateKeyPress(truck, 'D', dt);
            } else {
                simulateKeyPress(truck, 'A', dt);
            }
        }

        void handleBackIn(SemiTruck& truck, const ParkingSpot spot, float dt) {
            // Calculate position error
            float dx = spot.x - truck.cab_x;
            float dy = spot.y - truck.cab_y;
            float distance = std::sqrt(dx * dx + dy * dy);

            float cabAngleDiff = spot.targetAngle - truck.cab_angle;
            while (cabAngleDiff > 180.0f) cabAngleDiff -= 360.0f;
            while (cabAngleDiff < -180.0f) cabAngleDiff += 360.0f;

            float trailerAngleDiff = spot.targetAngle - truck.trailer_angle;
            while (trailerAngleDiff > 180.0f) trailerAngleDiff -= 360.0f;
            while (trailerAngleDiff < -180.0f) trailerAngleDiff += 360.0f;

            // Jackknife check
            float cabTrailerDiff = truck.cab_angle - truck.trailer_angle;
            while (cabTrailerDiff > 180.0f) cabTrailerDiff -= 360.0f;
            while (cabTrailerDiff < -180.0f) cabTrailerDiff += 360.0f;

            // back in slowly
            truck.cab_speed = -backSpeed;

            // Steering correction to prevent jackknife and guide into spot
            if (std:: abs(cabTrailerDiff) > 30.0f) {
                // Jackknife risk! Counter-steer!
                if (cabTrailerDiff > 0) {
                    simulateKeyPress(truck, 'A', dt);
                } else {
                    simulateKeyPress(truck, 'D', dt);
                }
            } else if (std::abs(cabAngleDiff) > 5.0f) {
                // Adjust angle toward the target
                if (cabAngleDiff > 0) {
                    simulateKeyPress(truck, 'D', dt);
                } else {
                    simulateKeyPress(truck, 'A', dt);
                }
            }

            // Check if close enough for fine adjustment
            if (distance < 30.0f && std::abs(cabAngleDiff) < 15.0f) {
                currentState = ADJUST;
                std::cout << "State: BACK_IN -> ADJUST" << std::endl;
            }

            // Safety check
            if (checkSensorProximity(truck, 25.0f)) {
                // Stop backing up if close to wall 
                truck.cab_speed = 0;
            }
        }

        void handleAdjust(SemiTruck& truck, const ParkingSpot spot, float dt) {
            // Very fine control for final positioning
            float dx = spot.x - truck.cab_x;
            float dy = spot.y - truck.cab_y;
            float distance = std::sqrt(dx * dx + dy * dy);

            float cabAngleDiff = spot.targetAngle - truck.cab_angle;
            while (cabAngleDiff > 180.0f) cabAngleDiff -= 360.0f;
            while (cabAngleDiff < -180.0f) cabAngleDiff += 360.0f;

            if (distance > 10.0f) {
                truck.cab_speed = -30.0f; // slowly move back
            } else if (std::abs(cabAngleDiff) > 3.0f) {
                truck.cab_speed = 10.0f; // slow move forward
                if (cabAngleDiff > 0) {
                    simulateKeyPress(truck, 'D', dt);
                } else {
                    simulateKeyPress(truck, 'A', dt);
                }
            } else {
                // stop
                truck.cab_speed;
            }
        }

        // Checks if any sensor is too close to an obstacle
        bool checkSensorProximity(const SemiTruck& truck, float threshold = -1.0f) {
            if (threshold < 0) threshold = minSensorDistance;

            for (float dist: truck.sensorDistances) {
                if (dist < threshold) {
                    return true;
                }
            }
            return false;
        }

        void simulateKeyPress(SemiTruck& truck, char key, float dt) {
            switch (key) {
                case 'W':
                    truck.cab_speed += truck.acceleration * dt;
                    break;
                case 'S':
                    truck.cab_speed -= truck.acceleration * dt;
                    break;
                case 'A':
                    if (std::abs(truck.cab_speed) > 10.0f) {
                        truck.cab_angle -= truck.turnRate * dt * (truck.cab_speed / truck.maxSpeed);
                        while (truck.cab_angle < 0.0f) truck.cab_angle += 360.0f;
                        while (truck.cab_angle >= 360.0f) truck.cab_angle -= 360.0f;
                    }
                    break;
                case 'D':
                    if (std::abs(truck.cab_speed) > 10.0f) {
                        truck.cab_angle += truck.turnRate * dt * (truck.cab_speed / truck.maxSpeed);
                        while (truck.cab_angle < 0.0f) truck.cab_angle += 360.0f;
                        while (truck.cab_angle >= 360.0f) truck.cab_angle -= 360.0f;
                    }
                    break;
            }

            // clamp speed
            if (truck.cab_speed > truck.maxSpeed) truck.cab_speed = truck.maxSpeed;
            if (truck.cab_speed < -truck.maxSpeed * 0.5f) truck.cab_speed = -truck.maxSpeed * 0.5f;
        }
    
    public:
        // Get current state from controller
        std::string getStateName() const {
            switch (currentState) {
                case APPROACH: return "APPROACH";
                case ALIGN: return "ALIGN";
                case BACK_IN: return "BACK_IN";
                case ADJUST: return "ADJUST";
                case PARKED: return "PARKED";
                default: return "UNKNOWN";
            }
        }
};


#endif