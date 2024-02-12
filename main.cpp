#include "mbed.h"

class PWMControl {

private:
    PwmOut motorPin;
    float pwmFrequency;
    
public:
    PWMControl(PinName pin, float frequency) : motorPin(pin), pwmFrequency(frequency) {
        motorPin.period(1.0 / pwmFrequency);
    }

    void setSpeed(float speed) {
        // Ensure speed is within valid range (0.0 to 1.0)
        speed = (speed < 0.0) ? 0.0 : ((speed > 1.0) ? 1.0 : speed);

        // Set the duty cycle to control speed
        motorPin.write(speed);
    }

};

class HBridgeControl {

public:
    HBridgeControl(PinName dirPin1, PinName dirPin2) 
        : direction1(dirPin1), direction2(dirPin2) {
        
        // Set initial direction (stop)
        stop();
    }
// Set motor direction to forward
    void forward() {
        direction1 = 1;
        direction2 = 0;
    }

// Set motor direction to backward
    void backward() {
        direction1 = 0;
        direction2 = 1;
    }

// Stop the motor
    void stop() {
        direction1 = 0;
        direction2 = 0;
    }

private:
    DigitalOut direction1;  // H-bridge input 1
    DigitalOut direction2;  // H-bridge input 2

    
};


int main() {
    DigitalOut bipolar1(D2);
    DigitalOut bipolar2(D7);
    DigitalOut enable(D15);
    DigitalOut led2(LED2);

    DigitalOut direction1(D3);
    DigitalOut direction2(D9);

    direction1 = 0;
    direction2 = 0;
    enable =1;
    bipolar1 = 0;
    bipolar2 = 0;
  
    PWMControl motorPin1(D5, 25000.0);  // Adjust pin and frequency
    PWMControl motorPin2(D9, 25000.0);  // Adjust pin and frequency
    HBridgeControl motordir(D5, D12);
    motorPin1.setSpeed(0.4);
    motorPin2.setSpeed(0.4);
   
        while(1){
                  motordir.forward();
                  wait(5);
                  motordir.backward();
                  wait(5);
               };
    
        // Gradually increase the speed
        // for (float speed = 0.0; speed <= 1.0; speed += 0.01) {
        //     motorPin.setSpeed(speed);
        
        // }

        // // Gradually decrease the speed
        // for (float speed = 1.0; speed >= 0.0; speed -= 0.01) {
        //     motorPin.setSpeed(speed);1
        // }
        // motorPin.setSpeed(0.5);
    }


