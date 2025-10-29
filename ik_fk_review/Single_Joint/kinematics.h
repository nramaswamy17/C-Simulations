#ifndef KINEMATICS_H
#define KINEMATICS_H

#include <cmath>

struct Position {
    double x, y;
};

class Kinematics {
public:
    double link_length;
    
    Kinematics(double length) : link_length(length) {}
    
    // Forward: angle -> position
    Position forward(double angle) {
        Position pos;
        pos.x = link_length * cos(angle);
        pos.y = link_length * sin(angle);
        return pos;
    }
    
    // Inverse: position -> angle
    double inverse(Position target) {
        return atan2(target.y, target.x);
    }
};

#endif
