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
                int signal;
                Callback<void()> callback;
                Callback<void(int)> callback_int;
                Callback<void(float)> callback_float;
                bool inUse;
                bool takesInt;
                bool takesFloat;
            };
                
        Serial bleModule;

        int input;

        callbacks callbacksArray[10]; // max of ten callbacks i guess

        void init();

        /* Decode 4 incoming chars into a single float                */
        float incoming_to_float();

    public:

        HM10(PinName rx, PinName tx);
        HM10(PinName rx, PinName tx, int baud);

        /*  Change the Baud Rate that the HM10 Module is running on 
            Can only be one of the specified Baud rates above
            Returns: 0 on successful change,
                    -1 if failed                                      */
        int setBaud(int baud);

        /*  Assign a callback to run when the signal number is sent
            through BLE. 
            Returns: 0 if the callback is successfully added
                    -1 if not (i.e. all callback slots used)          */
        int addCallback(int signal, Callback<void()> cb);
        int addCallback(int signal, Callback<void(int)> cb);
        int addCallback(int signal, Callback<void(float)> cb);

        /*  Remove a callback by passing in the signal that is supposed
            to trigger it.
            Returns: 0 if the callback is successfully removed
                    -1 if not, i.e. signal doesn't exist              */
        int removeCallback(int signal);

        /*  Transmit data through bluetooth. 
            ## This is a blocking call. ##
            Returns: 0 on successful addition  
                                                                      */
        int transmitData(const char* data, const int len);

        /*  Main Loop of BLE handling. Must be called every cycle, as this
            checks whether the BLE device is receiving data.          
            
            ## This is a blocking call. ##

            If a function that requires an argument to be called is
            received through bluetooth, then the main loop will be
            interrupted to wait for the bluetooth command.
            
            If the function requires an int, then we expect one value 
            through bluetooth (uint8_t).
            If the function requires a float, then we expect two values
            (uint8_t) x 2. See ints_to_half_float for details on decoding
                                                                     */
        void doBLE();

};


#endif
