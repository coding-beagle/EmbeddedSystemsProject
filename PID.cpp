#include "PID.h"
#include <mbed.h>

PIDController::PIDController(double prop, double integ, double deriv, double dt)
        : prop_const(prop), int_const(integ), deriv_const(deriv), dt(dt) {
            integral_sum = 0;
    }

double PIDController::calculate(double current_val, double desired_val) {
        // Proportional term
        error = current_val - desired_val;
        double proportional_error = prop_const * error;

        // Integral term
        integral_sum += error;
        double integral_error = int_const * 0.0; // Initialize to zero
        integral_error = integral_sum*dt;

        // Derivative term
        double derivative_error = deriv_const * (error - previous_error) / dt; 
        previous_error = error;

        // Calculate control output
        double output_speed = proportional_error + integral_error + derivative_error;

        return output_speed;
    }
