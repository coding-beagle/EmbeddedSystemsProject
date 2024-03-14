#include "mbed.h"
#include "PID.h"
#include "ENCODERS.h"
#include "config.h"
#include "LINESENSOR.h"
#include "BLEMODULE.h"

enum programState {normal, showdata, calculatePID,stop, togglePrint, change_p1, change_i1, change_d1, change_p2, change_i2, change_d2, change_p3, change_i3, change_d3};
programState state = normal;

void printData(){
    state = showdata;
}

void calcPID(){
    state = calculatePID;
}

volatile double p1 = 0.0, i1 = 0.0, d1 = 0.0;
volatile double p2 = 0.0, i2 = 0.0, d2 = 0.0;
volatile double p3 = 0.0, i3 = 0.0, d3 = 0.0;

volatile double desired_speed_1 = 0.0;
volatile double desired_speed_2 = 0.0;
volatile double prev_desired_speed_1 = 0.0;
volatile double prev_desired_speed_2 = 0.0;

volatile int prev_dir_1 = 0, prev_dir_2 = 0;

volatile bool tuningMotors;

DigitalOut direction1(DIRECTION1);
DigitalOut direction2(DIRECTION2);

void set_speed_1(float val){
    if(val < 0.0){direction1 = 0;}else{direction1 = 1;}
    desired_speed_1 = abs(val);
}

void set_speed_2(float val){
    if(val < 0.0){direction2 = 0;}else{direction2 = 1;}
    desired_speed_2 = abs(val);
}

void change_p1_val(float val){
    p1 = val;
    state = change_p1;
}

void change_i1_val(float val){
    i1 = val;
    state = change_i1;
}

void change_d1_val(float val){
    d1 = val;
    state = change_d1;
}

void change_p2_val(float val){
    p2 = val;
    state = change_p2;
}

void change_i2_val(float val){
    i2 = val;
    state = change_i2;
}

void change_d2_val(float val){
    d2 = val;
    state = change_d2;
}

void change_p3_val(float val){
    p3 = val;
    state = change_p3;
}

void change_i3_val(float val){
    i3 = val;
    state = change_i3;
}

void change_d3_val(float val){
    d3 = val;
    state = change_d3;
}

DigitalOut led(LED2);

bool printingEncoders = false;

void toggle_led(){
    led = !led;
}

void toggle_print(){
    printingEncoders = !printingEncoders;
}

bool printSerial;

void toggle_print_serial(){
    printSerial = !printSerial;
}

void return_speeds(){
    desired_speed_1 = prev_desired_speed_1;
    desired_speed_2 = prev_desired_speed_2;
    direction1 = prev_dir_1;
    direction2 = prev_dir_2;
    state = normal;
}

Timeout revert;

bool enabledStopCondition;

void toggleStopCondition(){
    enabledStopCondition = !enabledStopCondition;
    printf("Toggling Stop Cond\n");
}

DigitalOut enable(ENABLE_PIN);

void stopStart(){
    enable = !enable;
}

void uturn(){
    prev_desired_speed_1 = desired_speed_1;
    prev_desired_speed_2 = desired_speed_2;
    desired_speed_1 = 3.0;
    desired_speed_2 = 3.0;

    prev_dir_1 = direction1;
    prev_dir_2 = direction2;

    direction1 = 0;
    direction2 = 0;

    revert.attach(&return_speeds, 1.33);

    state = normal;
}

double error_scale = 1.0;

void change_error_scale(float val){
    error_scale = val;
}

int main() {
    enable = 1;

    Serial pc(USBTX, USBRX);

    led = 0;

    enabledStopCondition = false;

    HM10 ble(BLE_PINS_TUPLE);

    ble.addCallback(TOGGLE_LED_SIGNAL, &toggle_led);
    ble.addCallback(TOGGLE_PRINT_ENCODERS_SIGNAL, &toggle_print);
    ble.addCallback(TOGGLE_PRINT_SERIAL_SIGNAL, &toggle_print_serial);

    ble.addCallback(SET_MOTOR1_SPEED_SIGNAL, &set_speed_1);
    ble.addCallback(SET_MOTOR2_SPEED_SIGNAL, &set_speed_2);

    ble.addCallback(SET_MOTOR1_P_SIGNAL, &change_p1_val);
    ble.addCallback(SET_MOTOR1_I_SIGNAL, &change_i1_val);
    ble.addCallback(SET_MOTOR1_D_SIGNAL, &change_d1_val);
    ble.addCallback(SET_MOTOR2_P_SIGNAL, &change_p2_val);
    ble.addCallback(SET_MOTOR2_I_SIGNAL, &change_i2_val);
    ble.addCallback(SET_MOTOR2_D_SIGNAL, &change_d2_val);

    ble.addCallback(SET_NAVIGATION_P_SIGNAL, &change_p3_val);
    ble.addCallback(SET_NAVIGATION_I_SIGNAL, &change_i3_val);
    ble.addCallback(SET_NAVIGATION_D_SIGNAL, &change_d3_val);

    ble.addCallback(ROTATE_180_SIGNAL, &uturn);
    ble.addCallback(TOGGLE_STOP_COND_SIGNAL, &toggleStopCondition);

    ble.addCallback(TOGGLE_ENABLE_SIGNAL, &stopStart);
    ble.addCallback(CHANGE_ERROR_SCALE_SIGNAL, &change_error_scale);

    direction1 = 0;
    direction2 = 1;

    DigitalOut bipolar1(BIPOLAR1);
    bipolar1 = 0;
    
    DigitalOut bipolar2(BIPOLAR2);
    bipolar2 = 0;

    PwmOut pwm1(MOTOR1PWM);
    pwm1.period(Period);
    pwm1.write(1.0);

    PwmOut pwm2(MOTOR2PWM);
    pwm2.period(Period);
    pwm2.write(1.0);

    Encoder encoder1(ENCODER1_CHA, ENCODER1_CHB, 256, 0.08, ENCODER_MEASURE_RATE);
    Encoder encoder2(ENCODER2_CHA, ENCODER2_CHB, 256, 0.08, ENCODER_MEASURE_RATE);

    PIDController first_motor_controller(0.12, 1.0, 0.001, PID_ENCODER_FREQ);
    PIDController second_motor_controller(0.12, 0.4, 0.001, PID_ENCODER_FREQ);

    PIDController navigation_controller(1.0, 0.0, 0.0, PID_ENCODER_FREQ);

    // pwm1.write(0.5);
    // pwm2.write(0.5);
    
    Ticker t;
    t.attach(&printData, DATA_MEASURE_FREQ);

    Ticker t2;
    t2.attach(&calcPID, PID_ENCODER_FREQ);

    double write_val1;
    double write_val2;

    DigitalIn button(USER_BUTTON);

    LineSensor lineSensorManager(SENSOR_BANK_1_PIN, SENSOR_BANK_2_PIN, LINE_SENSOR_1_PIN, LINE_SENSOR_2_PIN, LINE_SENSOR_3_PIN, LINE_SENSOR_4_PIN);

    lineSensorManager.enableBank1();
    lineSensorManager.enableBank2();

    int polls = 0;

    double navigation_bias = 0.0;
    double bank_1_difference = 0.0;
    double bank_2_difference = 0.0;
    double avg_value = 0.0;
    double avg_difference = 0.0;

    // for interpolation
    double current_speed_1 = 0.0;
    double current_speed_2 = 0.0;
    double last_tick_current_speed_1 = 0.0;
    double last_tick_current_speed_2 = 0.0;

    bool calculatingPID = false;
    printSerial = true;

    int stop_count = 0;

    wait(1);// wait for everything to init
    
    while(1) {

        switch (state){
            case normal:
                if(!button){
                    // printf("Button Pressed");
                    // timer.start();
                }
                ble.doBLE();

                break;
            case showdata:
                if(!calculatingPID and printSerial){
                    if(printingEncoders){
                        printf("RPS1 = %f\n", encoder1.getRPS());
                        printf("RPS2 = %f\n", encoder2.getRPS());
                    }
                    else{
                        printf("Sensor1 = %f\n", lineSensorManager.getAverageSensorValue(1));
                        printf("Sensor2 = %f\n", lineSensorManager.getAverageSensorValue(2));
                        // printf("Sensor3 = %f\n", lineSensorManager.getAverageSensorValue(3));
                        // printf("Sensor4 = %f\n", lineSensorManager.getAverageSensorValue(4));
                        // printf("Sensor5 = %f\n", navigation_bias);
                        }
                }
                // printf("Write Val 1 = %lf Write Val 2 = %lf\n", (write_val1), write_val2);
                state = normal;
                break;

            case calculatePID:
                calculatingPID = true;

                // lineSensorManager.enableBank1();
                // lineSensorManager.disableBank2();
                lineSensorManager.readSensorValues(1);
                lineSensorManager.readSensorValues(2);
                // lineSensorManager.disableBank1();
                
                // lineSensorManager.readSensorValues(3);
                // lineSensorManager.readSensorValues(4);

                bank_1_difference = lineSensorManager.getAverageSensorValue(2) - lineSensorManager.getAverageSensorValue(1);
                // bank_2_difference = lineSensorManager.getAverageSensorValue(4) - lineSensorManager.getAverageSensorValue(3);
                avg_value = (lineSensorManager.getAverageSensorValue(1) + lineSensorManager.getAverageSensorValue(2)) / 2;

                if(enabledStopCondition){
                    if(avg_value < BANK_1_STOP_CONDITION){
                        stop_count++;
                        if(stop_count > 150){
                            state = stop;
                            break;
                        }
                    }
                    else{
                        if(stop_count > 0){
                            stop_count = 0;
                        }
                    }
                }

                // basic linear acceleration
                current_speed_1 = (desired_speed_1 * ACCELERATION_RATE_PER_TICK) + last_tick_current_speed_1*(1-ACCELERATION_RATE_PER_TICK);
                last_tick_current_speed_1 = current_speed_1;

                current_speed_2 = (desired_speed_2 * ACCELERATION_RATE_PER_TICK) + last_tick_current_speed_2*(1-ACCELERATION_RATE_PER_TICK);
                last_tick_current_speed_2 = current_speed_2;

                error_scale = (current_speed_1 + current_speed_2)/2;
                error_scale = 20 * pow(0.01 * error_scale, 0.5);
                if(error_scale > 4.0){
                    error_scale = 4.0;
                }

                navigation_bias = navigation_controller.calculate(bank_1_difference * error_scale, 0.0);

                write_val1 = - first_motor_controller.calculate(encoder1.getRPS(), abs(current_speed_1 + navigation_bias));        // 0.7 is needed because otherwise we spike to 0.0
                write_val1 = (write_val1 > 1.0) ? write_val1 = 1.0 : (write_val1 <= 0.0) ? write_val1 = 0.7 : write_val1 = write_val1;
                pwm1.write(write_val1);

                write_val2 = - second_motor_controller.calculate(encoder2.getRPS(), abs(current_speed_2 - navigation_bias));
                write_val2 = (write_val2 > 1.0) ? write_val2 = 1.0 : (write_val2 <= 0.0) ? write_val2 = 0.7 : write_val2 = write_val2;
                pwm2.write(write_val2);

                state = normal;
                calculatingPID = false;
                break;
            case stop:
                printf("Stopping\n");
                desired_speed_1 = 0.0;
                desired_speed_2 = 0.0;
                enable = 0;
                state = normal;
                break;                

            case togglePrint:
                printSerial = !printSerial;
                if(printSerial){
                    t.attach(&printData, DATA_MEASURE_FREQ);
                }
                else{
                    t.detach();
                }
                state = normal;
                break;

            case change_p1:
                first_motor_controller.setP(p1);
                state = normal;
                break;
            case change_i1:
                first_motor_controller.setI(i1);
                first_motor_controller.reset();
                state = normal;
                break;
            case change_d1:
                first_motor_controller.setD(d1);
                first_motor_controller.reset();
                state = normal;
                break;
            case change_p2:
                second_motor_controller.setP(p2);
                state = normal;
                break;
            case change_i2:
                second_motor_controller.setI(i2);
                second_motor_controller.reset();
                state = normal;
                break;
            case change_d2:
                second_motor_controller.setD(d2);
                second_motor_controller.reset();
                state = normal;
                break;
            case change_p3:
                navigation_controller.setP(p3);
                state = normal;
                break;
            case change_i3:
                navigation_controller.setI(i3);
                navigation_controller.reset();
                state = normal;
                break;
            case change_d3:
                navigation_controller.setD(d3);
                navigation_controller.reset();
                state = normal;
                break;
        }
    }
}