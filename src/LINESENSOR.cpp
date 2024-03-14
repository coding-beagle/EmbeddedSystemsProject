#include "mbed.h"
#include "LINESENSOR.h"

LineSensor::LineSensor(PinName enablePin1, PinName enablePin2,
               PinName pin1, PinName pin2, PinName pin3, PinName pin4): 
            enablePin1(enablePin1), enablePin2(enablePin2),
            sensor1(pin1), sensor2(pin2), sensor3(pin3), sensor4(pin4) {
                for(int i = 0; i<5; i++){
                    sensor1_values[i] = 0.0;
                    sensor2_values[i] = 0.0;
                    sensor3_values[i] = 0.0;
                    sensor4_values[i] = 0.0;
                }
                sensor1_index = 0;
                sensor2_index = 0;
                sensor3_index = 0;
                sensor4_index = 0;

                maxSensorAverage1 = 0.0;
                maxSensorAverage2 = 0.0;
                maxSensorAverage3 = 0.0;
                maxSensorAverage4 = 0.0;
                
                lastcounts = 0;
            }

double LineSensor::normalize(int sensorNumber){
    switch(sensorNumber){
        case 1:
            return (averageSensorValue1 - sensor1Min) / (sensor1Max - sensor1Min);
        case 2:
            return (averageSensorValue2 - sensor2Min) / (sensor2Max - sensor2Min);
        case 3:
            return (averageSensorValue3 - sensor3Min) / (sensor3Max - sensor3Min);
        case 4:
            return (averageSensorValue4 - sensor4Min) / (sensor4Max - sensor4Min);
    }
}

void LineSensor::readSensorValues(int sensorNumber){
    // lastcounts++;
    // if(lastcounts > 1000){
    //     maxSensorAverage1 = 0.0;
    //     maxSensorAverage2 = 0.0;
    //     maxSensorAverage3 = 0.0;
    //     maxSensorAverage4 = 0.0;
    //     lastcounts = 0;
    // }
    switch (sensorNumber){
        case 1:
            sensor1_values[sensor1_index] = sensor1.read();
            sensor1_index++; sensor1_index %= 5;
        case 2:
            sensor2_values[sensor2_index] = sensor2.read();
            sensor2_index++; sensor2_index %= 5;
        case 3:
            sensor3_values[sensor3_index] = sensor3.read();
            sensor3_index++; sensor3_index %= 5;
        case 4:
            sensor4_values[sensor4_index] = sensor4.read();
            sensor4_index++; sensor4_index %= 5;
    }
}

void LineSensor::enableBank1() { enablePin1 = 1; }
void LineSensor::disableBank1() { enablePin1 = 0; }

void LineSensor::enableBank2() { enablePin2 = 1; }
void LineSensor::disableBank2() { enablePin2 = 0; }

float LineSensor::getAverageSensorValue(int sensorNumber){
    double sum = 0.0;
    switch (sensorNumber){
        case 1:
            for(int i = 0; i<5; i++){
                sum += sensor1_values[i];
            }
            averageSensorValue1 = sum / 5.0;
            return normalize(1);
        case 2:
            for(int i = 0; i<5; i++){
                sum += sensor2_values[i];
            }
            averageSensorValue2 = sum / 5.0;
            return normalize(2);
        case 3:
            for(int i = 0; i<5; i++){
                sum += sensor3_values[i];
            }
            averageSensorValue3 = sum / 5.0;
            return normalize(3);
        case 4:
            for(int i = 0; i<5; i++){
                sum += sensor4_values[i];
            }
            averageSensorValue4 = sum / 5.0;
            return normalize(4);
        default:
            return -1.0;
    }
}