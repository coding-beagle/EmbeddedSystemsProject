#ifndef ENCODERS_H
#define ENCODERS_H
#include "mbed.h"

class Encoder{
    private:
        InterruptIn channelA;
        int pulses, revolutions, pulses_per_rev;
        float RPS, wheel_diameter, calc_period;
        Ticker t;

        void increment();

        void calculateRPS();

    public:
        Encoder(PinName channel, int PPR, float wheel, float period);
        
        int getPulses();

        void clearPulses();

        void clearRPS();

        float getRPS();

        float getDistanceM();
};

#endif
