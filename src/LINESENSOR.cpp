#include "mbed.h"
#include "LINESENSOR.h"

LineSensor::LineSensor(PinName enablePin1, PinName enablePin2,
               PinName pin1, PinName pin2, PinName pin3, PinName pin4): 
            enablePin1(enablePin1), enablePin2(enablePin2),
            sensor1(pin1), sensor2(pin2), sensor3(pin3), sensor4(pin4) {}

void LineSensor::readSensorValues(float samplingDuration){
        Timer timer;
        timer.start();

        double sumSensorValue1 = 0.0;
        double sumSensorValue2 = 0.0;
        double sumSensorValue3 = 0.0;
        double sumSensorValue4 = 0.0;
        int numSamples = 0;

        // Sample sensor readings over the specified duration
        while (timer.read() < samplingDuration) {
            sumSensorValue1 += (double)(sensor1.read());
            sumSensorValue2 += (double)(sensor2.read());
            sumSensorValue3 += (double)(sensor3.read());
            sumSensorValue4 += (double)(sensor4.read());
            numSamples++;
        }

        // Calculate the average sensor values
        averageSensorValue1 = sumSensorValue1 / numSamples;
        averageSensorValue2 = sumSensorValue2 / numSamples;
        averageSensorValue3 = sumSensorValue3 / numSamples;
        averageSensorValue4 = sumSensorValue4 / numSamples;

        // Use average values for further processing
}

void LineSensor::enableBank1() { enablePin1 = 1; }
void LineSensor::disableBank1() { enablePin1 = 0; }

void LineSensor::enableBank2() { enablePin2 = 1; }
void LineSensor::disableBank2() { enablePin2 = 0; }

float LineSensor::getAverageSensorValue(int sensorNumber){
    switch (sensorNumber){
        case 1:
            return averageSensorValue1;
        case 2:
            return averageSensorValue2;
        case 3:
            return averageSensorValue3;
        case 4:
            return averageSensorValue4;
        default:
            return -1.0;
    }
}