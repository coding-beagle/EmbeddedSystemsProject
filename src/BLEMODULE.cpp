#include "mbed.h"
#include "BLEMODULE.h"
#include <cstdint>

void HM10::init(){
    for(int i = 0; i<NUMBER_OF_COMMANDS; i++){
        callbacksArray[i].signal = -1;
        callbacksArray[i].inUse = false;
        callbacksArray[i].takesInt = false;
        callbacksArray[i].takesFloat = false;
    }
}

HM10::HM10(PinName rx, PinName tx) : bleModule(rx, tx){
            init();
    }

HM10::HM10(PinName rx, PinName tx, int baud) : bleModule(rx, tx){
            HM10::setBaud(baud);
            init();
}

float HM10::incoming_to_float(){
    int index = 0;
    uint32_t output = 0;
    char data[5] = "SFLT";  // "Send FLoaT", tells receiver to initiate float transfer
    transmitData(data, 5);
    while(index < 4){
        uint32_t c = bleModule.getc() << (index * 8);
        output |= c;
        index++;
        char data[5] = "RCVD";
        transmitData(data, 5);
    }

    union {
            uint32_t asInt;
            float asFloat;
        } u;

    u.asInt = output;
    return u.asFloat;
}

int HM10::setBaud(int baud){
    int acceptable_bauds[] = { ACCEPTABLE_BAUDS };
    int i = 0;
    for(i=0; i < NUMBER_OF_BAUDS; i++){
        if(baud == acceptable_bauds[i]){
            break;
        }
    }
    if(i == NUMBER_OF_BAUDS){
        printf("Invalid Baudrate!");
        return -1;
    }
    else{
        bleModule.baud(baud);
        return 0;
    }
}
    
int HM10::addCallback(int signal, Callback<void()> cb){
    for(int i = 0; i < NUMBER_OF_COMMANDS; i++){
        if(callbacksArray[i].inUse == false){
            callbacksArray[i].signal = signal;
            callbacksArray[i].callback = cb;
            callbacksArray[i].inUse = true;
            return 0;
        }
    }
    printf("All Callbacks In Use!");
    return -1;
}

int HM10::addCallback(int signal, Callback<void(int)> cb){
    for(int i = 0; i < NUMBER_OF_COMMANDS; i++){
        if(callbacksArray[i].inUse == false){
            callbacksArray[i].signal = signal;
            callbacksArray[i].callback_int = cb;
            callbacksArray[i].inUse = true;
            callbacksArray[i].takesInt = true;
            callbacksArray[i].takesFloat = false;
            return 0;
        }
    }
    printf("All Callbacks In Use!");
    return -1;
}

int HM10::addCallback(int signal, Callback<void(float)> cb){
    for(int i = 0; i < NUMBER_OF_COMMANDS; i++){
        if(callbacksArray[i].inUse == false){
            callbacksArray[i].signal = signal;
            callbacksArray[i].callback_float = cb;
            callbacksArray[i].inUse = true;
            callbacksArray[i].takesInt = false;
            callbacksArray[i].takesFloat = true;
            return 0;
        }
    }
    printf("All Callbacks In Use!");
    return -1;
}

int HM10::removeCallback(int signal){
    int i = 0;
    for(i; i<NUMBER_OF_COMMANDS;i++){
        if(signal == callbacksArray[i].signal){
                callbacksArray[i].inUse = false;
                callbacksArray[i].signal = 0;
                callbacksArray[i].takesInt = false; 
                callbacksArray[i].takesFloat = false;  
                break;
        }
    }
    if(i == sizeof(callbacksArray)/sizeof(callbacks)){
        printf("Callback with signal %c could not be found!", signal);
        return -1;
    }
    return 0;
}

int HM10::transmitData(const char* data, const int len){
    printf("Called with %s\n", data);
    for(int i = 0; i<len+1; i++){
        bleModule.putc(data[i]);
    }
    return 0;
}

/* when a ble signal is received, check if the signal matches any signal that has been
previously declared. if it has, then execute that callback. this approach allows us to
dynamically create and declare callbacks via bluetooth*/
void HM10::doBLE(){
    if(bleModule.readable()){
        int c = bleModule.getc();
        
        for(int i = 0; i<NUMBER_OF_COMMANDS;i++){     // find if the signal matches an existing callback in array
            
            if(c == callbacksArray[i].signal){
                if(callbacksArray[i].takesInt){
                    char data[5] = "SINT"; // Tell Receiver to send int
                    transmitData(data, 5);
                    int arg = bleModule.getc();
                    callbacksArray[i].callback_int(arg);
                    char toTransmit[30];
                    sprintf(toTransmit, "Executed with %d", arg);
                    transmitData(toTransmit, 30);
                }
                else if(callbacksArray[i].takesFloat){
                    float float_arg = incoming_to_float();
                    callbacksArray[i].callback_float(float_arg);
                    char toTransmit[30];
                    // printf("(From Class) Executed with %f\n", float_arg);
                    sprintf(toTransmit, "Executed with %.3f", float_arg);
                    transmitData(toTransmit, 30);
                }
                else{
                    callbacksArray[i].callback();
                    transmitData("Executed", 9);
                    break;
                }
            }
        }
    }
}
