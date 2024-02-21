#include "mbed.h"

// DigitalOut myled(LED1);
#define Frequency 25000.0
#define Period 1.0/Frequency
// #define maxCurrent 0.100 // mA
// #define threeQuartersPercentCurrent 0.065 // mA

void accelerateMotors(PwmOut *Motor1, PwmOut *Motor2, float max_speed1, float max_speed2, float duration){
    float speed1, speed2;
    int i = 1000;
    int i2 = 1000;
    speed1 = (1.0 - max_speed1) * 1000;
    speed2 = (1.0 - max_speed2) * 1000;
    while(i > (int)speed1 or i2 > (int)speed2){
            
            if(i > (int)speed1){
                Motor1->write(i*0.001);
                i--;
            }

            if(i2 > (int)speed2){
                Motor2->write(i2*0.001);
                i2--;
            }
            wait(0.001);
    }
    wait(duration);
}

DigitalOut led(LED2);

void turnOnLed(){
    led = 1;
    wait(0.1);
    led = 0;
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
    pwm.write(1.0);

    PwmOut pwm2(D10);
    pwm2.period(Period);
    pwm2.write(1.0);

    DigitalIn button(USER_BUTTON);
    
    InterruptIn encoder(D15);
    encoder.rise(&turnOnLed);

    bool currentlyExecuting = false;

    while(1){
    if(button == 0 && !currentlyExecuting){
        currentlyExecuting = true;
    for(int i = 0; i<4; i++){
        //straight drive
        direction1 = 1;
        direction2 = 0;
        accelerateMotors(&pwm, &pwm2, 0.5, 0.401, 1.43);
    
        //turning
        direction1 = 1;
        direction2 = 1;
        accelerateMotors(&pwm, &pwm2, 0.5, 0.5, 0.165);
    }

    direction1 = 1;
    direction2 = 1;
    accelerateMotors(&pwm, &pwm2, 0.5, 0.5, 0.14);

    for(int i = 0; i<4; i++){
        //straight drive
        direction1 = 1;
        direction2 = 0;
        accelerateMotors(&pwm, &pwm2, 0.5, 0.4, 1.45);
    
        //turning
        direction1 = 0;
        direction2 = 0;
        accelerateMotors(&pwm, &pwm2, 0.5, 0.5, 0.145);
    }
    // direction1 = 1;
    // direction2 = 0;
    // accelerateMotors(&pwm, &pwm2, 0.5, 0.4, 1.5);

    pwm.write(1.0);
    pwm2.write(1.0);
    }
    }
}

