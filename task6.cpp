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
    {0.000, 0.010, 0,0, 0.0}, // start at 0.0
    {0.010, 2.010, 1,0, 3.0}, // 
    {2.010, 2.510, 0,0, 0.0},
    {2.510, 2.910, 1,1, 3.0},
    {2.910, 3.410, 0,0, 0.0},
    {3.410, 5.410, 1,0, 3.0},
    {5.410, 5.910, 0,0, 0.0},
    {5.910, 6.310, 1,1, 3.0},
    {6.310, 6.810, 0,0, 0.0},
    {6.810, 8.810, 1,0, 3.0},
    {8.810, 9.310, 0,0, 0.0},
    {9.310, 9.710, 1,1, 3.0},
    {9.710, 10.210, 0,0, 0.0},
    {10.210, 12.210, 1,0, 3.0},
    {12.210, 12.710, 0,0, 0.0},
    {12.710, 13.110, 1,1, 3.0},
    {13.110, 13.610, 0,0, 0.0},
    {13.610, 14.010, 1,1, 3.0},
    {14.010, 14.510, 0,0, 0.0},
    {14.510, 16.510, 1,0, 3.0},
    {16.510, 17.010, 0,0, 0.0},
    {17.010, 17.410, 0,0, 3.0},
    {17.410, 17.910, 0,0, 0.0},
    {17.910, 19.910, 1,0, 3.0},
    {19.910, 20.410, 0,0, 0.0},
    {20.410, 20.810, 0,0, 3.0},
    {20.810, 21.310, 0,0, 0.0},
    {21.310, 23.310, 1,0, 3.0},
    {23.310, 23.810, 0,0, 0.0},
    {23.810, 24.210, 0,0, 3.0},
    {24.210, 24.710, 0,0, 0.0},
    {24.710, 26.710, 1,0, 3.0},
    {26.710, 27.210, 0,0, 0.0},
    {27.210, 27.610, 0,0, 3.0},
    {27.610, 28.110, 0,0, 0.0},
    {28.110, -1, 1, 0, 0.0}
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
            // desired_speed = steps[currentStep].desiredSpeed;

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

    PIDController first_motor_controller(0.1, 0.4, 0.0, PID_FREQ);
    PIDController second_motor_controller(0.1, 0.3, 0.0, PID_FREQ);

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
