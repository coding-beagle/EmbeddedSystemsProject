#include "mbed.h"
#include "ENCODERS.h"

void Encoder::increment(){
    pulses++;
}

void Encoder::calculateRPS(){
    RPS = (pulses / (float)pulses_per_rev) * 1/(calc_period);
    clearPulses();
}

Encoder::Encoder(PinName channel, int PPR, float wheel, float period): channelA(channel), pulses_per_rev(PPR), wheel_diameter(wheel), calc_period(period){
    channelA.rise(callback(this, &Encoder::increment));
    channelA.fall(callback(this, &Encoder::increment));
    t.attach(callback(this, &Encoder::calculateRPS), calc_period);
}

int Encoder::getPulses(){
    return pulses;
}

void Encoder::clearPulses(){
    pulses = 0;
}

void Encoder::clearRPS(){
    RPS = 0;
}

float Encoder::getRPS(){
    return RPS;
}

float Encoder::getDistanceM(){
    return RPS * wheel_diameter * 3.14;
}
