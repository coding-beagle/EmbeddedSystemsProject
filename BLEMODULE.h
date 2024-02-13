#ifndef BLEMODULE_H
#define BLEMODULE_H
#include "mbed.h"

#define ACCEPTABLE_BAUDS 9600, 19200, 38400, 57600, 115200, 4800, 2400, 1200, 230400 // as per spec sheet
#define NUMBER_OF_BAUDS 9
#define COMMAND_LENGTH 2
#define OUTPUT_BUFFER_SIZE 50

class HM10{
    private:
        struct callbacks{
                char signal;
                Callback<void()> callback;
                bool inUse;
            };
        
        void init();
        
        Serial bleModule;

        char inputBuffer[COMMAND_LENGTH];   // incoming data
        int inputBufferInIndex;

        char outputBuffer[50];  // to send signals to bluetooth receiver
        int outputBufferOutIndex;
        int outputBufferInIndex;

        callbacks callbacksArray[5]; // max of five callbacks i guess

    public:

    HM10(PinName rx, PinName tx);
    HM10(PinName rx, PinName tx, int baud);

    /*  Change the Baud Rate that the HM10 Module is running on 
        Can only be one of the specified Baud rates above
        Returns: 0 on successful change,
                -1 if failed                                      */
    int setBaud(int baud);

    /*  Assign a callback to run when the signal character is sent
        through BLE. 
        Returns: 0 if the callback is successfully added
                -1 if not (i.e. all callback slots used)          */
    int addCallback(const char signal, Callback<void()> cb);
    
    /*  Remove a callback by passing in the signal that is supposed
        to trigger it.
        Returns: 0 if the callback is successfully removed
                -1 if not, i.e. signal doesn't exist              */
    int removeCallback(const char signal);

    /*  Add data to the output transmission buffer, which gets sent
        over the BLE connection.
        Returns: 0 on successful addition  
        NOT WORKING YET                                           */
    int transmitData(const char* data, const int len);

    /*  Main Loop of BLE handling. Must be called every cycle, as this
        checks whether the BLE device is receiving data.          */
    void doBLE();

};


#endif
