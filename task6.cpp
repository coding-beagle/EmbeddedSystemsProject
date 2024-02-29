#include "mbed.h"
#include "PID.h"
#include "ENCODERS.h"
#include "config.h"

enum programState {normal, showdata, calculatePID, reset_timer_and_speed};
programState state = normal;

void printData(){
    state = showdata;
}

void calcPID(){
    state = calculatePID;
}

typedef struct {
    float startTime;
    float endTime;
    int direction1;
    int direction2;
    float desiredSpeed;
} Step;

//  The command structure reads as follows:
//
//  Time Start | Time End | Direction1 | Direction2 | Desired Motor Speed 
//
//  This is generated via command_maker.py in the repository            

Step steps[] = {
    {0.00, 0.010, 0,0, 0.0},
    {0.01, 2.410, 1,0, 3.0},
    {2.41, 2.610, 0,0, 0.0},
    {2.61, 3.140, 1,1, 3.0},
    {3.14, 3.340, 0,0, 0.0},
    {3.34, 5.340, 1,0, 3.0},
    {5.34, 5.540, 0,0, 0.0},
    {5.54, 6.070, 1,1, 3.0},
    {6.07, 6.270, 0,0, 0.0},
    {6.27, 8.270, 1,0, 3.0},
    {8.27, 8.470, 0,0, 0.0},
    {8.47, 9.000, 1,1, 3.0},
    {9.00, 9.200, 0,0, 0.0},
    {9.20, 11.200, 1,0, 3.0},
    {11.20, 11.400, 0,0, 0.0},
    {11.40, 11.930, 1,1, 3.0},
    {11.93, 12.130, 0,0, 0.0},
    {12.13, 12.660, 1,1, 3.0},
    {12.66, 12.860, 0,0, 0.0},
    {12.86, 14.860, 1,0, 3.0},
    {14.86, 15.060, 0,0, 0.0},
    {15.06, 15.460, 0,0, 3.0},
    {15.46, 15.660, 0,0, 0.0},
    {15.66, 17.660, 1,0, 3.0},
    {17.66, 17.860, 0,0, 0.0},
    {17.86, 18.260, 0,0, 3.0},
    {18.26, 18.460, 0,0, 0.0},
    {18.46, 20.460, 1,0, 3.0},
    {20.46, 20.660, 0,0, 0.0},
    {20.66, 21.060, 0,0, 3.0},
    {21.06, 21.260, 0,0, 0.0},
    {21.26, 23.260, 1,0, 3.0},
    {23.26, 23.460, 0,0, 0.0},
    {23.46, 23.860, 0,0, 3.0},
    {23.86, 24.060, 0,0, 0.0},
    {24.06, -1, 1, 0, 0.0}
};

volatile int currentStep = 0;
volatile double desired_speed = 0.0;

DigitalOut direction1(DIRECTION1);
DigitalOut direction2(DIRECTION2);

void updateMotorControl(float currentTime) {
    if (currentStep < (sizeof(steps) / sizeof(steps[0])) - 1) {
        // it's not the last step, proceed with normal logic
        if (currentTime >= steps[currentStep].startTime && 
            (currentTime < steps[currentStep].endTime || steps[currentStep].endTime == -1)) {
            // it's within the current step's time range, so set motor controls accordingly
            direction1 = (steps[currentStep].direction1);
            direction2 = (steps[currentStep].direction2);
            
            // basic acceleration
            // calculate the difference between the current and target speeds
            double speed_difference = steps[currentStep].desiredSpeed - desired_speed;

            // determine the speed adjustment, ensuring it does not exceed the speed difference magnitude
            double speed_adjustment = (abs(speed_difference) < ACCELERATION_RATE_PER_TICK) ? speed_difference : ACCELERATION_RATE_PER_TICK * (speed_difference > 0 ? 1 : -1);
            

            desired_speed += speed_adjustment;
        } 
        if (currentTime >= steps[currentStep].endTime && steps[currentStep].endTime != -1) {
            // time to move to the next step
            currentStep++;
            // check the next step immediately if within its start time
            if (currentTime < steps[currentStep].startTime) {
                updateMotorControl(currentTime);
            }
        }
    } else {
        // For the last step, ensure the motor stops
        if (steps[currentStep].endTime == -1 && currentTime >= steps[currentStep].startTime) {
            state = reset_timer_and_speed;
        }
    }
}

int main() {
    DigitalOut enable(ENABLE_PIN);
    enable = 1;

    direction1 = 0;
    direction2 = 1;

    DigitalOut bipolar1(BIPOLAR1);
    bipolar1 = 0;
    
    DigitalOut bipolar2(BIPOLAR2);
    bipolar2 = 0;

    PwmOut pwm1(MOTOR1PWM);
    pwm1.period(Period);
    pwm1.write(1.0);

    PwmOut pwm2(MOTOR2PWM);
    pwm2.period(Period);
    pwm2.write(1.0);

    Encoder encoder1(ENCODER1_CHA, 256, 0.08, ENCODER_MEASURE_RATE);
    Encoder encoder2(ENCODER2_CHA, 256, 0.08, ENCODER_MEASURE_RATE);

    PIDController first_motor_controller(0.12, 0.4, 0.001, PID_FREQ);
    PIDController second_motor_controller(0.12, 0.3, 0.001, PID_FREQ);

    // pwm1.write(0.5);
    // pwm2.write(0.5);
    
    Ticker t;
    t.attach(&printData, DATA_MEASURE_FREQ);

    Ticker t2;
    t2.attach(&calcPID, PID_FREQ);

    double write_val1;
    double write_val2;

    DigitalIn button(USER_BUTTON);

    desired_speed = 0.0;

    Timer timer;
    wait(1);// wait for everything to init
    
    while(1) {

        switch (state){
            case reset_timer_and_speed:
                timer.stop();
                timer.reset();
                desired_speed = 0.0;
                state = normal;
                break;
            case normal:
                if(!button){
                    // printf("Button Pressed");
                    timer.start();
                    currentStep = 0;
                }
                
                updateMotorControl(timer);
                break;
            case showdata:
                printf("%f\n", encoder1.getRPS());
                printf("%f\n", encoder2.getRPS());
                // printf("Write Val 1 = %lf Write Val 2 = %lf\n", (write_val1), write_val2);
                state = normal;
                break;
            case calculatePID:
                write_val1 = - first_motor_controller.calculate(encoder1.getRPS(), desired_speed);        // 0.7 is needed because otherwise we spike to 0.0
                write_val1 = (write_val1 > 1.0) ? write_val1 = 1.0 : (write_val1 <= 0.0) ? write_val1 = 0.7 : write_val1 = write_val1;
                pwm1.write(write_val1);

                write_val2 = - second_motor_controller.calculate(encoder2.getRPS(), desired_speed);
                write_val2 = (write_val2 > 1.0) ? write_val2 = 1.0 : (write_val2 <= 0.0) ? write_val2 = 0.7 : write_val2 = write_val2;
                pwm2.write(write_val2);

                // printf("Write Val 1 = %lf\n Write Val 2 = %lf", write_val1, write_val2);

                state = normal;
                break;
        }
    }
}
