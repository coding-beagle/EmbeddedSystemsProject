#ifndef ENCODERS_H
#define ENCODERS_H
#include "mbed.h"

#define ROLLING_AVERAGE_DATA_POINTS 10

class Encoder{
    private:
        InterruptIn channelA, channelB;
        int pulses, revolutions, pulses_per_rev, rps_index;
        float RPS, wheel_diameter, calc_period;
        Ticker t;
        float previous_rps[30];

        void increment();

        void calculateRPS();

    public:
        Encoder(PinName channel1, PinName channel2, int PPR, float wheel, float period);
        
        int getPulses();

        void clearPulses();

        void clearRPS();

        float getRPS();

        float getDistanceM();
};

#endif