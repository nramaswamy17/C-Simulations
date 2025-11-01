#ifndef KINEMATICS_H
#define KINEMATICS_H

#include <cmath>

struct Position {
    double x, y;
};

struct Result {
    Position pos1, pos2, pos3;
};

struct JointAngles {
    double theta1, theta2, theta3;
};

class Kinematics {
public:
    double L1;
    double L2;
    double L3;
    
    Kinematics(double length1, double length2, double length3) {
        L1 = length1;
        L2 = length2;
        L3 = length3;
    }
    
    // Forward: angle -> position
    Result forward(double theta1, double theta2, double theta3) {
        Result result;
        
        // End of base (Elbow)
        result.pos1.x = L1 * cos(theta1);
        result.pos1.y = L1 * sin(theta1);
        
        // End of elbow (Wrist)
        result.pos2.x = result.pos1.x + L2 * cos(theta1 + theta2);
        result.pos2.y = result.pos1.y + L2 * sin(theta1 + theta2);

        // End of wrist (end effector)
        result.pos3.x = result.pos2.x + L3 * cos(theta1 + theta2 + theta3);
        result.pos3.y = result.pos2.y + L3 * sin(theta1 + theta2 + theta3);

        return result;
    }
    
    // Inverse: position -> angle
    JointAngles inverse(Position target, double phi) {
        JointAngles angles;

        double x = target.x;
        double y = target.y;
        
        double x_wrist = x - L3 * cos(phi);
        double y_wrist = y - L3 * sin(phi);


        // Euclidean distance to target
        double dist = sqrt(x_wrist*x_wrist + y_wrist*y_wrist);

        // Check if target is within max len
        if (dist > L1 + L2 + L3 || dist < fabs(L1-L2-L3)) {
            angles.theta1 = atan2(y_wrist, x_wrist);
            angles.theta2 = 0;
            angles.theta3 = 0;
            return angles;
        }

        // Law of cosines to find theta2
        double cos_theta2 = (dist*dist - L1*L1 - L2*L2) / (2*L1*L2);
        angles.theta2 = acos(cos_theta2); // Elbow down solution since +/-

        // Find theta1
        double beta = atan2(y_wrist, x_wrist);
        double alpha = atan2(L2*sin(angles.theta2), L1 + L2*cos(angles.theta2));
        angles.theta1 = beta-alpha;

        // Calculate wrist angle based on current orientation
        angles.theta3 = phi - angles.theta1 - angles.theta2;

        return angles;
    }
};

#endif
