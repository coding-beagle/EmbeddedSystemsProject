#include "mbed.h"
#include "BLEMODULE.h"

/*
Example code of how to get BLE running on 
the nucleo board. Connects to the BLE module
through serial, and then turns on or off
when the respective signal is sent
*/

bool ledState;

void toggleLED(){
    ledState = !ledState;
    return;
}

int main() {
    DigitalOut led(LED2);
    led = 1;

    HM10 hm10(D10, D2, 9600);           // Creates a hm10 device with desired TX, RX and Baudrate
    hm10.addCallback('Z', &toggleLED);  // Assigns a callback that runs when the character 'Z' is received through BLE

    while(1){
        hm10.doBLE();                   // doBLE must be called through the loop
        led = ledState;
    }
}