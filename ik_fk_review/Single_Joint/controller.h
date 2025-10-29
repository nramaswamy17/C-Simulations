#ifndef CONTROLLER_H
#define CONTROLLER_H

class PIDController {
public:
    double kp, ki, kd;
    double integral;
    double prev_error;
    
    PIDController(double p, double i, double d) 
        : kp(p), ki(i), kd(d), integral(0), prev_error(0) {}
    
    double compute(double current, double target, double dt) {
        double error = target - current;
        
        integral += error * dt;
        double derivative = (error - prev_error) / dt;
        prev_error = error;
        
        return kp * error + ki * integral + kd * derivative;
    }
    
    void reset() {
        integral = 0;
        prev_error = 0;
    }
};

#endif
