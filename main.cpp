#include "mbed.h"

// DigitalOut myled(LED1);
#define Frequency 25000.0
#define Period 1.0/Frequency
// #define maxCurrent 0.100 // mA
// #define threeQuartersPercentCurrent 0.065 // mA

void accelerateMotors(PwmOut *Motor1, PwmOut *Motor2, float max_speed1, float max_speed2, float duration){
    float speed1, speed2;
    int i = 100;
    int i2 = 100;
    speed1 = (1.0 - max_speed1) * 100;
    speed2 = (1.0 - max_speed2) * 100;
    while(i > (int)speed1 or i2 > (int)speed2){
            
            if(i > (int)speed1){
                Motor1->write(i*0.01);
                i--;
            }

            if(i2 > (int)speed2){
                Motor2->write(i2*0.01);
                i2--;
            }
            wait(0.01);
    }
    wait(duration);
}

int main() {
    DigitalOut enable(D9);
    enable = 1;

    DigitalOut direction1(D3);
    direction1 = 0;

    DigitalOut direction2(D6);
    direction1 = 0;

    DigitalOut bipolar1(D4);
    bipolar1 = 0;
    
    DigitalOut bipolar2(D7);
    bipolar2 = 0;

    PwmOut pwm(D5);
    pwm.period(Period);
    pwm.write(0.9);

    PwmOut pwm2(D10);
    pwm2.period(Period);
    pwm2.write(0.9);

    DigitalIn button(USER_BUTTON);

    // PwmOut pwm2(D9);
    // pwm2.period(Period);
    // pwm2.write(0.9);

    bool currentlyExecuting = false;

    while(1){
    if(button == 0 && !currentlyExecuting){
        currentlyExecuting = true;
    for(int i = 0; i<4; i++){
        //straight drive
        direction1 = 1;
        direction2 = 0;
        accelerateMotors(&pwm, &pwm2, 0.5, 0.4, 1.4);
    
        //turning
        direction1 = 1;
        direction2 = 1;
        accelerateMotors(&pwm, &pwm2, 0.5, 0.5, 0.15);
    }

    direction1 = 1;
    direction2 = 1;
    accelerateMotors(&pwm, &pwm2, 0.5, 0.5, 0.1);

    for(int i = 0; i<4; i++){
        //straight drive
        direction1 = 1;
        direction2 = 0;
        accelerateMotors(&pwm, &pwm2, 0.5, 0.4, 1.4);
    
        //turning
        direction1 = 0;
        direction2 = 0;
        accelerateMotors(&pwm, &pwm2, 0.5, 0.5, 0.12);
    }
    direction1 = 1;
    direction2 = 0;
    accelerateMotors(&pwm, &pwm2, 0.5, 0.4, 1.4);

    pwm.write(1.0);
    pwm2.write(1.0);
    }
    }
}

