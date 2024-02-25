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

Step steps[] = {
    {0.01, 2.0, 1, 0, 2.0}, // Straight
    {2.0, 2.4, 1, 1, 3.4},  // Left Turn
    {2.4, 4.4, 1, 0, 2.0},  
    {4.4, 4.8, 1, 1, 3.7},  
    {4.8, 6.8, 1, 0, 2.0},  
    {6.8, 7.2, 1, 1, 3.7},  
    {7.2, 9.2, 1, 0, 2.0},  
    {9.2, 10.0, 1, 1, 3.4},  // Double turn 
    {10.0, 12.0, 1, 0, 2.0},  
    {12.0, 12.4, 0, 0, 3.7},  
    {12.4, 14.4, 1, 0, 2.0}, 
    {14.4, 14.8, 0, 0, 3.7},
    {14.8, 16.8, 1, 0, 2.0}, 
    {16.8, 17.2, 0, 0, 3.7},
    {17.2, 19.2, 1, 0, 2.0},
    {19.2, -1.0, 0, 0, 0.0}  // -1 for endTime indicates the end.
};

int currentStep = 0;
double desired_speed = 0.0;

DigitalOut direction1(DIRECTION1);

DigitalOut direction2(DIRECTION2);

void updateMotorControl(float currentTime) {
    if (currentStep < (sizeof(steps) / sizeof(steps[0])) - 1) {
        // It's not the last step, proceed with normal logic
        if (currentTime >= steps[currentStep].startTime && 
            (currentTime < steps[currentStep].endTime || steps[currentStep].endTime == -1)) {
            // It's within the current step's time range, so set motor controls accordingly
            direction1 = (steps[currentStep].direction1);
            direction2 = (steps[currentStep].direction2);
            desired_speed = steps[currentStep].desiredSpeed;
        } 
        if (currentTime >= steps[currentStep].endTime && steps[currentStep].endTime != -1) {
            // Time to move to the next step
            currentStep++;
            // Check the next step immediately if within its start time
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

    PIDController first_motor_controller(0.08, 0.001, 0.00001, PID_FREQ);
    PIDController second_motor_controller(0.08, 0.002, 0.00001, PID_FREQ);

    // pwm1.write(0.5);
    // pwm2.write(0.5);
    
    Ticker t;
    t.attach(&printData, DATA_MEASURE_FREQ);

    Ticker t2;
    t2.attach(&calcPID, PID_FREQ);

    double write_val1;
    double write_val2;

    DigitalIn button(USER_BUTTON);

    // double desired_speed = 0.0;

    Timer timer;
    wait(1);// wait for everything to init
    
    while(1) {

        switch (state){
            case reset_timer_and_speed:
                timer.reset();
                desired_speed = 0.0;
                state = normal;
                break;
            case normal:
                if(!button){
                    printf("Button Pressed");
                    timer.start();
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
                write_val1 = first_motor_controller.calculate(encoder1.getRPS(), desired_speed);        // 0.7 is needed because otherwise we spike to 0.0
                write_val1 = (write_val1 > 1.0) ? write_val1 = 1.0 : (write_val1 <= 0.0) ? write_val1 = 0.7 : write_val1 = write_val1;
                pwm1.write(write_val1);

                write_val2 = second_motor_controller.calculate(encoder2.getRPS(), desired_speed);
                write_val2 = (write_val2 > 1.0) ? write_val2 = 1.0 : (write_val2 <= 0.0) ? write_val2 = 0.7 : write_val2 = write_val2;
                pwm2.write(write_val2);

                // printf("Write Val 1 = %lf\n Write Val 2 = %lf", write_val1, write_val2);

                state = normal;
                break;
        }
    }
}
