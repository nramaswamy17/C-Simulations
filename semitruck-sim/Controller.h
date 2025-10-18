#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <cmath>
#include <iostream>
#include "SemiTruck.h"
#include "Lane.h"

enum LaneKeepingState {
    CENTERED,           // Truck is well-centered in lane
    CORRECTING_LEFT,    // Drifting right, steering left
    CORRECTING_RIGHT,   // Drifting left, steering right
    EMERGENCY_LEFT,     // Far right, aggressive correction left
    EMERGENCY_RIGHT     // Far left, aggressive correction right
};

class Controller {
public:
    LaneKeepingState currentState;
    bool isEnabled;
    
    // Target lane
    int targetLaneIndex;
    
    // Control parameters
    float targetSpeed;
    float Kp_lateral;      // Proportional gain for lateral error
    float Kp_heading;      // Proportional gain for heading error
    float Kd_lateral;      // Derivative gain for lateral error
    
    // PID state
    float previousLateralError;
    
    // Thresholds
    float centeredThreshold;
    float emergencyThreshold;
    
    Controller() {
        currentState = CENTERED;
        isEnabled = false;
        targetLaneIndex = 1;  // Middle lane by default
        
        targetSpeed = 120.0f;   // Slower for curves
        Kp_lateral = 0.5f;     // Higher gain for curves
        Kp_heading = 2.0f;     // Higher gain for curves
        Kd_lateral = 0.2f;     // More damping
        
        previousLateralError = 0.0f;
        
        centeredThreshold = 15.0f;   // Within 15px = centered
        emergencyThreshold = 35.0f;  // Beyond 35px = emergency (tighter for curves)
    }
    
    void enable() {
        isEnabled = true;
        previousLateralError = 0.0f;
    }
    
    void disable() {
        isEnabled = false;
    }
    
    void toggle() {
        isEnabled = !isEnabled;
        if (isEnabled) {
            previousLateralError = 0.0f;
        }
    }
    
    // Main control loop
    void update(SemiTruck& truck, const Road& road, float dt) {
        if (!isEnabled) return;
        
        // Get the target lane
        const Lane& targetLane = road.lanes[targetLaneIndex];
        
        // Calculate errors
        float lateralError = targetLane.getLateralError(truck);
        float headingError = targetLane.getHeadingError(truck);
        
        // Update state based on error magnitude
        updateState(lateralError);
        
        // Apply lane keeping control
        applyLaneKeeping(truck, lateralError, headingError, dt);
    }
    
    void setTargetLane(int laneIndex) {
        targetLaneIndex = laneIndex;
    }
    
    std::string getStateName() const {
        switch (currentState) {
            case CENTERED: return "CENTERED";
            case CORRECTING_LEFT: return "CORRECTING_LEFT";
            case CORRECTING_RIGHT: return "CORRECTING_RIGHT";
            case EMERGENCY_LEFT: return "EMERGENCY_LEFT";
            case EMERGENCY_RIGHT: return "EMERGENCY_RIGHT";
            default: return "UNKNOWN";
        }
    }

    // Angle of target lane relative to truck position
    float getDesiredHeading(const SemiTruck& truck, const Road& road) const {
        if (!isEnabled) {
            return truck.cab_angle;
        }

        const Lane& targetLane = road.lanes[targetLaneIndex];

        // Find closest point on target lane
        int closestIdx = targetLane.findClosestPointIndex(truck);
        const RoadPoint& closestPoint  = targetLane.centerline[closestIdx];

        // The desired heading is the lane's tangent direction to this point
        return closestPoint.angle;
    }
    
private:
    void updateState(float lateralError) {
        float absError = std::abs(lateralError);
        
        if (absError < centeredThreshold) {
            currentState = CENTERED;
        } else if (absError > emergencyThreshold) {
            currentState = (lateralError > 0) ? EMERGENCY_RIGHT : EMERGENCY_LEFT;
        } else {
            currentState = (lateralError > 0) ? CORRECTING_LEFT : CORRECTING_RIGHT;
        }
    }
    
    void applyLaneKeeping(SemiTruck& truck, float lateralError, 
                         float headingError, float dt) {
        // Calculate derivative of lateral error
        float lateralErrorDerivative = (lateralError - previousLateralError) / dt;
        previousLateralError = lateralError;
        
        // PD control for steering correction
        // Negative lateral error = too far left, need to steer right (positive)
        // Positive lateral error = too far right, need to steer left (negative)
        float steeringCorrection = -Kp_lateral * lateralError 
                                  - Kp_heading * headingError
                                  - Kd_lateral * lateralErrorDerivative;
        
        // Emergency corrections get stronger response
        if (currentState == EMERGENCY_LEFT || currentState == EMERGENCY_RIGHT) {
            steeringCorrection *= 2.0f;
        }
        
        // Maintain target speed
        float speedError = targetSpeed - truck.cab_speed;
        if (std::abs(speedError) > 5.0f) {
            if (speedError > 0) {
                truck.cab_speed += truck.acceleration * dt * 0.5f;
            } else {
                truck.cab_speed -= truck.acceleration * dt * 0.5f;
            }
        }
        
        // Apply steering (simulate key presses based on correction)
        if (std::abs(truck.cab_speed) > 10.0f) {
            if (steeringCorrection > 0.5f) {
                // Steer right
                truck.cab_angle += truck.turnRate * dt * 
                                  (truck.cab_speed / truck.maxSpeed) * 
                                  std::min(steeringCorrection, 3.0f);
            } else if (steeringCorrection < -0.5f) {
                // Steer left
                truck.cab_angle -= truck.turnRate * dt * 
                                  (truck.cab_speed / truck.maxSpeed) * 
                                  std::min(std::abs(steeringCorrection), 3.0f);
            }
            
            // Normalize angle
            while (truck.cab_angle < 0.0f) truck.cab_angle += 360.0f;
            while (truck.cab_angle >= 360.0f) truck.cab_angle -= 360.0f;
        }
        
        // Clamp speed
        if (truck.cab_speed > truck.maxSpeed) truck.cab_speed = truck.maxSpeed;
        if (truck.cab_speed < -truck.maxSpeed * 0.5f) 
            truck.cab_speed = -truck.maxSpeed * 0.5f;
    }
};

#endif // CONTROLLER_H