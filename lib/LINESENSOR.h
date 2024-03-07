#ifndef LINESENSOR_H
#define LINESENSOR_H
#include "mbed.h"

class LineSensor {
private:
    AnalogIn sensor1;
    AnalogIn sensor2;
    AnalogIn sensor3;
    AnalogIn sensor4;
    DigitalOut enablePin1; // Pin connected to Darlington array for sensor 1
    DigitalOut enablePin2; // Pin connected to Darlington array for sensor 2
    Timer timer;

    double averageSensorValue1;
    double averageSensorValue2;
    double averageSensorValue3;
    double averageSensorValue4;

public:
    // Constructor to initialize sensors and enable pins
    LineSensor(PinName enablePin1, PinName enablePin2,
               PinName pin1, PinName pin2, PinName pin3, PinName pin4);

    // Enable/disable sensors 1, 2
    void enableBank1();
    void disableBank1();

    // Enable/disable sensors 3, 4
    void enableBank2();
    void disableBank2();

    float getAverageSensorValue(int sensorNumber);

    // Read values from sensors with sampling
    void readSensorValues(float samplingDuration);
};

#endif