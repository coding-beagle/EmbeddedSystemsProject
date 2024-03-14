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

    int lastcounts;

    double sensor1_values[5];
    int sensor1_index;
    double sensor2_values[5];
    int sensor2_index;
    double sensor3_values[5];
    int sensor3_index;
    double sensor4_values[20];
    int sensor4_index;

    double averageSensorValue1;
    double averageSensorValue2;
    double averageSensorValue3;
    double averageSensorValue4;

    double maxSensorAverage1;
    double maxSensorAverage2;
    double maxSensorAverage3;
    double maxSensorAverage4;

    // adjust values as needed in lab
    const double sensor1Max = 1.0;
    const double sensor1Min = 0.0;
    
    const double sensor2Max = 1.0;
    const double sensor2Min = 0.0;

    const double sensor3Max = 1.0;
    const double sensor3Min = 0.0;

    const double sensor4Max = 1.0;
    const double sensor4Min = 0.0;

    double normalize(int sensorNum);

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
    void readSensorValues(int sensorNumber);
};

#endif