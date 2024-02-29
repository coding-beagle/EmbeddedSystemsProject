#include "mbed.h"
#include "BLEMODULE.h"
#include "ENCODERS.h"
#include "config.h"

#define Frequency 25000.0
#define Period 1.0/Frequency
#define DelayPeriod 0.2

bool ledState;
bool togglePrintRPS = true;

DigitalOut enable(ENABLE_PIN);

enum programState {normal, sweepMotor1, sweepMotor2, sweepMotors, checkpulse, changedir1, changedir2, checkEncoders};
programState state = normal;

void toggleLED(){
    ledState = !ledState;
    state = normal;
}
    
void Motor1Drive(){
    state = sweepMotor1;
}

void Motor2Drive(){
    state = sweepMotor2;
}

DigitalOut direction2(DIRECTION2);
DigitalOut direction1(DIRECTION1);

void changeDir1(int dir){
    direction1 = dir;
}

void changeDir2(int dir){
    direction2 = dir;
}

void enableDisable(){
    enable = !enable;
}

void checkPulses(){
    state = checkpulse;
}

void bothmotors(){
    state = sweepMotors;
}

void encodermode(){
    state = checkEncoders;
}

long map(long x, long in_min, long in_max, long out_min, long out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

PwmOut pwm1(MOTOR1PWM);
PwmOut pwm2(MOTOR2PWM);


void driveMotor1(int dutyCycle){
    pwm1.write((float)(dutyCycle) * 0.01);
}

void driveMotor2(int dutyCycle){
    pwm2.write((float)(dutyCycle) * 0.01);
}

void transmit_float(float float_received){
    printf("%f\n", float_received);
}

void allowedPrint(){
    togglePrintRPS = !togglePrintRPS;
}

DigitalOut led(LED2);

int main() {

    HM10 ble(D10, D2, 9600);
    ble.addCallback(99, &toggleLED);  // Assigns a callback that runs when the integer 1111 is received through BLE

    ble.addCallback(13, &enableDisable);
    ble.addCallback(14, &changeDir1);  
    ble.addCallback(15, &changeDir2);

    ble.addCallback(19, &driveMotor1);
    ble.addCallback(20, &driveMotor2);

    ble.addCallback(50, &allowedPrint);

    ble.addCallback(33, &transmit_float);

    enable = 1;

    double rps1 = 0;
    double rps2 = 0;
    
    direction1 = 0;   
    direction2 = 0;

    DigitalOut bipolar1(BIPOLAR1);
    bipolar1 = 0;
    
    DigitalOut bipolar2(BIPOLAR2);
    bipolar2 = 0;

    pwm1.period(Period);
    pwm1.write(1.0);
    
    pwm2.period(Period);
    pwm2.write(1.0);

    Encoder encoder1(PC_3, 256, 0.08, 0.01);
    Encoder encoder2(PC_2, 256, 0.08, 0.01);

    Ticker t;
    t.attach(&checkPulses, 0.3);

    while(1) {
        ble.doBLE();
        
        switch(state){

            case normal:
                led = ledState;
                break;

            case checkpulse:
                if(togglePrintRPS){
                    char data[14];
                    rps1 = encoder1.getRPS();
                    rps2 = encoder2.getRPS();

                    sprintf(data, "RPS1 = %1.4lf", rps1);
                    ble.transmitData(data, sizeof(data)/sizeof(char));
                    sprintf(data, "RPS2 = %1.4lf", rps2);
                    ble.transmitData(data, sizeof(data)/sizeof(char));
                }
                
                state = normal;
                break;

        }
    }
}
