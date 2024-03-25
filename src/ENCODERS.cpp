#include "mbed.h"
#include "ENCODERS.h"

void Encoder::increment(){
    pulses++;
}

void Encoder::calculateRPS(){
    RPS = (pulses / (float)pulses_per_rev) * 1/(calc_period);
    previous_rps[rps_index] = RPS;
    rps_index++; if(rps_index >= ROLLING_AVERAGE_DATA_POINTS){rps_index = 0;}
    clearPulses();
}

Encoder::Encoder(PinName channel1, PinName channel2, int PPR, float wheel, float period): channelA(channel1) ,channelB(channel2), pulses_per_rev(PPR), wheel_diameter(wheel), calc_period(period){
    channelA.rise(callback(this, &Encoder::increment));
    channelA.fall(callback(this, &Encoder::increment));
    channelB.rise(callback(this, &Encoder::increment));
    channelB.fall(callback(this, &Encoder::increment));
    t.attach(callback(this, &Encoder::calculateRPS), calc_period);
    rps_index = 0;
    clearPulses();
    clearRPS();
}

int Encoder::getPulses(){
    return pulses;
}

void Encoder::clearPulses(){
    pulses = 0;
}

void Encoder::clearRPS(){
    for(int i = 0; i<ROLLING_AVERAGE_DATA_POINTS; i++){previous_rps[i] = 0.0;}
}

float Encoder::getRPS(){
    float sum = 0.0;
    for(int i = 0; i < ROLLING_AVERAGE_DATA_POINTS; i++){
        sum += previous_rps[i];
    }

    // divide by 10.0 because we have 4x
    return sum / ((float)ROLLING_AVERAGE_DATA_POINTS * 2);
}

float Encoder::getDistanceM(){
    return getRPS() * wheel_diameter * 3.14;
}