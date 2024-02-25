#include "mbed.h"
#include "BLEMODULE.h"

void HM10::init(){
    outputBufferOutIndex = 0;
    outputBufferInIndex = 0;

    for(int i = 0; i<10; i++){
        callbacksArray[i].inUse = false;
        callbacksArray[i].takesArg = false;
    }
}

HM10::HM10(PinName rx, PinName tx) : bleModule(rx, tx){
            init();
    }

HM10::HM10(PinName rx, PinName tx, int baud) : bleModule(rx, tx){
            HM10::setBaud(baud);
            init();
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
    for(int i = 0; i < 10; i++){
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
    for(int i = 0; i < 10; i++){
        if(callbacksArray[i].inUse == false){
            callbacksArray[i].signal = signal;
            callbacksArray[i].callback_int = cb;
            callbacksArray[i].inUse = true;
            callbacksArray[i].takesArg = true;
            return 0;
        }
    }
    printf("All Callbacks In Use!");
    return -1;
}

int HM10::removeCallback(int signal){
    int i = 0;
    for(i; i<sizeof(callbacksArray)/sizeof(callbacks);i++){
        if(signal == callbacksArray[i].signal){
                callbacksArray[i].inUse = false;
                callbacksArray[i].signal = 0;
                callbacksArray[i].takesArg = false;   
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
        
        for(int i = 0; i<sizeof(callbacksArray)/sizeof(callbacks);i++){     // find if the signal matches an existing callback in array
            
            if(c == callbacksArray[i].signal){
                if(callbacksArray[i].takesArg){
                    int arg = bleModule.getc();
                    callbacksArray[i].callback_int(arg);
                    char toTransmit[30];
                    sprintf(toTransmit, "Executed with %d", arg);
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
