#ifndef KINEMATICS_H
#define KINEMATICS_H

#include <cmath>

struct Position {
    double x, y;
};

struct Result {
    Position pos1, pos2;
};

struct JointAngles {
    double theta1, theta2;
};

class Kinematics {
public:
    double L1;
    double L2;
    
    Kinematics(double length1, double length2) {
        L1 = length1;
        L2 = length2;
    }
    
    // Forward: angle -> position
    Result forward(double theta1, double theta2) {
        Result result;
        Position end_pos;
        Position elbow_pos = getElbow(theta1);

        end_pos.x = elbow_pos.x + L2 * cos(theta1 + theta2);
        end_pos.y = elbow_pos.y + L2 * sin(theta1 + theta2);
        
        result.pos1 = elbow_pos;
        result.pos2 = end_pos;

        return result;
    }

    Position getElbow(double theta1) {
        Position pos;
        pos.x = L1 * cos(theta1);
        pos.y = L1 * sin(theta1);

        return pos;
    }
    
    // Inverse: position -> angle
    JointAngles inverse(Position target) {
        JointAngles angles;

        double x = target.x;
        double y = target.y;
        
        // Euclidean distance to target
        double dist = sqrt(x*x + y*y);

        // Check if target is within max len
        if (dist > L1 + L2 || dist < fabs(L1-L2)) {
            angles.theta1 = atan2(y, x);
            angles.theta2 = 0;
            return angles;
        }

        // Law of cosines to find theta2
        double cos_theta2 = (dist*dist - L1*L1 - L2*L2) / (2*L1*L2);
        angles.theta2 = acos(cos_theta2); // Elbow down solution

        // Find theta 1
        double beta = atan2(y, x);
        double alpha = atan2(L2*sin(angles.theta2), L1 + L2*cos(angles.theta2));
        angles.theta1 = beta-alpha;

        return angles;
    }
};

#endif
