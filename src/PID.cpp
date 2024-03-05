#include "PID.h"
#include <mbed.h>


PIDController::PIDController(double prop, double integ, double deriv, double dt)
        : prop_const(prop), int_const(integ), deriv_const(deriv), dt(dt) {
            previous_error = 0.0;
            integral_sum = 0.0;
    }

void PIDController::setP(double value){
    prop_const = value;
}

void PIDController::setI(double value){
    int_const = value;
}

void PIDController::setD(double value){
    deriv_const = value;
}

void PIDController::reset(){
    integral_sum = 0.0;
    previous_error = 0.0;
}

double PIDController::calculate(double current_val, double desired_val) {
        // Proportional term
        error = desired_val - current_val;
        double proportional_error = prop_const * error;

        // Integral term
        
        integral_sum += error*dt;
        double integral_error = integral_sum * int_const; // Initialize to zero

        // Derivative term
        double derivative_error = (error - previous_error) / dt; 
        previous_error = error;
        derivative_error *= deriv_const;

        // Calculate control output
        return proportional_error + integral_error + derivative_error;
    }
