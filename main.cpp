#include <mbed.h>

Serial hm10(D1, D0); //UART6 TX,RX

DigitalOut led(D5);
DigitalOut led2(D9);

char c; //the character we want to receive

int main() {

    hm10.baud(9600);

    led=1;
    led2=1;

    while(1) {
        if(hm10.readable()){
            led2=0;
            c = hm10.getc(); //read a single character
            if(c == 'A'){
                led = 1;
            }
            else if(c == 'B'){
                led = 0;
            }
        }
        else{
            led2=1;
        }
    }
}