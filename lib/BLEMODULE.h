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

        callbacks callbacksArray[20]; // max of twenty callbacks i guess

        /*  Clears internal callback buffers.
         */
        void init();

        /*  Decode 4 incoming chars into a single float
         *  ## This is a blocking call ##
         *
         *  This specific implementation requires the user to send 4
         *  chars in succession, starting from the least significant byte.
         *  
         *  Failure to send 4 chars will halt the program, which can be
         *  found if the bluetooth device does not echo the received float
         *  back to the sender. E.g. 'Called with xx.xxxx' will be echoed
         *  once all bytes are received.               
         */
        float incoming_to_float();

    public:

        HM10(PinName rx, PinName tx);
        HM10(PinName rx, PinName tx, int baud);

        /*  Change the Baud Rate that the HM10 Module is running on 
         *  Can only be one of the specified Baud rates above
         *
         *  @baud => The baud rate to change the HM10 communications to.
         *
         *  Returns: 0 on successful change,
         *          -1 if failed (i.e. Baud rate is invalid)                                      
         */
        int setBaud(int baud);

        /*  Assign a callback to run when the signal number is sent
         *  through BLE. 
         *
         *  @signal => The value that must be received by the HM10 module to trigger the callback.
         *  @cb     => The callback that gets executed when 'signal' is received.
         *
         *  Returns: 0 if the callback is successfully added
         *          -1 if not (i.e. all callback slots used)          
         */
        int addCallback(int signal, Callback<void()> cb);
        int addCallback(int signal, Callback<void(int)> cb);
        int addCallback(int signal, Callback<void(float)> cb);

        /*  Remove a callback by passing in the signal that is supposed
         *  to trigger it.
         *
         *  @signal => Which signal to remove from the internal callback buffer.
         *
         *  Returns: 0 if the callback is successfully removed
         *          -1 if not, i.e. signal doesn't exist              
         */
        int removeCallback(int signal);

        /*  Transmit data through bluetooth. 
         *  ## This is a blocking call. ##
         *
         *  @data => Pointer to string buffer to be sent.
         *  @len  => length of the string buffer. 
         *
         *  Returns: 0 on successful addition  
         */                                                            
        int transmitData(const char* data, const int len);

        /*  Main Loop of BLE handling. Must be called every cycle, as this
         *  checks whether the BLE device is receiving data.          
         *  
         *  ## This is a blocking call. ##
         *
         *  If a function that requires an argument to be called is
         *  received through bluetooth, then the main loop will be
         *  interrupted to wait for the bluetooth command.
         *  
         *  If the function requires an int, then we expect one value 
         *  through bluetooth (uint8_t).
         *  If the function requires a float, then we expect two values
         *  (uint8_t) x 2. See ints_to_half_float for details on decoding
         */
        void doBLE();

};


#endif
