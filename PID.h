#ifndef PID_H
#define PID_H

#include <mbed.h>

class PIDController {
private:
    double prop_const;       // Proportional gain
    double int_const;        // Integral gain
    double deriv_const;      // Derivative gain
    double dt;              // Time between sensor polls
    double previous_error;  // error previous delta t (for deriv)
    double error;
    double integral_sum;

public:
    PIDController(double prop, double integ, double deriv, double dt);

    double calculate(double current_val, double desired_val);
};

#endif
