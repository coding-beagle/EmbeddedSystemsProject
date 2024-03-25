#include "mbed.h"
#include "PID.h"
#include "ENCODERS.h"
#include "config.h"
#include "LINESENSOR.h"
#include "BLEMODULE.h"

enum programState {normal, showdata, calculatePID,stop, reset_controllers, togglePrint, change_val};
programState state = normal;

enum valueToChange {speed1, speed2, pro1, int1, der1, pro2, int2, der2, pro3, int3, der3, pro4, int4, der4} value=speed1;
float changeTo = 0.0;

void printData(){
    state = showdata;
}

void calcPID(){
    state = calculatePID;
}

volatile double desired_speed_1 = 0.0;
volatile double desired_speed_2 = 0.0;
volatile double prev_desired_speed_1 = 0.0;
volatile double prev_desired_speed_2 = 0.0;
volatile int prev_dir_1 = 0, prev_dir_2 = 0;

DigitalOut direction1(DIRECTION1);
DigitalOut direction2(DIRECTION2);

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

Timeout revert;

bool enabledStopCondition;
bool useSensors;

void toggleStopCondition(){
    enabledStopCondition = !enabledStopCondition;
    printf("Toggling Stop Cond\n");
}

DigitalOut enable(ENABLE_PIN);
bool toggle_interp;

void return_speed(){
    desired_speed_1 = prev_desired_speed_1;
    desired_speed_2 = prev_desired_speed_2;
    toggle_interp = true;
    useSensors = true;
    state = normal;
}

void kill_speed(){
    desired_speed_1 = 0.0;
    desired_speed_2 = 0.0;
    direction1 = prev_dir_1;
    direction2 = prev_dir_2;
    state = normal;

    revert.attach(&return_speed, 0.3);
}

Timeout turn;

void reenable_sensors(){
    useSensors = true;
}

void do_turn(){
    enable = 1;
    desired_speed_1 = 1.5;
    desired_speed_2 = 1.5;
    turn.attach(&reenable_sensors, 1.0);
}

void uturn(){
    toggle_interp = false;
    useSensors = false;
    prev_desired_speed_1 = desired_speed_1;
    prev_desired_speed_2 = desired_speed_2;
    desired_speed_1 = 0.0;
    desired_speed_2 = 0.0;
    enable = 0;
    // scale stopping time based on current_speed
    turn.attach(&do_turn, (0.25 * desired_speed_1) + 0.5);

    prev_dir_1 = direction1;
    prev_dir_2 = direction2;

    direction1 = 0;
    direction2 = 0;

    revert.attach(&kill_speed, (0.25 * desired_speed_1) + 1.8);

    state = normal;
}

double error_scale = 1.0;

Ticker t;
Ticker t2;

void enable_tickers(){
    t.attach(&printData, DATA_MEASURE_FREQ);
    t2.attach(&calcPID, PID_ENCODER_FREQ);
}

void disable_tickers(){
    t.detach();
    t2.detach();
}

void stopStart(){
    enable = !enable;
    disable_tickers();
    state = reset_controllers;
}

void use_sensors(){
    useSensors = !useSensors;
}

void changeValueBeingSet(int val){
    value = (valueToChange)(val);
}

void setValueTo(float val){
    changeTo = val;
    state = change_val;
}

void setValueToQuick(int val){
    changeTo = (float)val/10;
    state = change_val;
}

void speedsTo(int val){         // quick speed setting shortcut
    desired_speed_1 = (float)(val)/10;
    desired_speed_2 = (float)(val)/10;
    prev_desired_speed_1 = (float)(val)/10;
    prev_desired_speed_2 = (float)(val)/10;
}

void disable_motors(){
    enable = 0;
}

void enable_motors(){
    enable = 1;
}

void change_interp(){
    toggle_interp = !toggle_interp;
}

#ifdef USE_RC_MODE
void stop_all_commands(){
    desired_speed_1 = 0.0;
    desired_speed_2 = 0.0;
}

Timeout stop_after_command;

void forward(){
    desired_speed_1 = 2.0;
    desired_speed_2 = 2.0;
    direction1 = 0;
    direction2 = 1;
    stop_after_command.attach(&stop_all_commands, 0.1);
}

void backward(){
    desired_speed_1 = 2.0;
    desired_speed_2 = 2.0;
    direction1 = 1;
    direction2 = 0;
    stop_after_command.attach(&stop_all_commands, 0.1);
}

void left(){
    desired_speed_1 = 2.0;
    desired_speed_2 = 2.0;
    direction1 = 1;
    direction2 = 1;
    stop_after_command.attach(&stop_all_commands, 0.1);
}

void right(){
    desired_speed_1 = 2.0;
    desired_speed_2 = 2.0;
    direction1 = 0;
    direction2 = 0;
    stop_after_command.attach(&stop_all_commands, 0.1);
}
#endif

int main() {
    enable = 1;

    Serial pc(USBTX, USBRX);

    led = 0;

    toggle_interp = true;

    useSensors = true;

    enabledStopCondition = false;

    HM10 ble(BLE_PINS_TUPLE);

    PwmOut pwm1(MOTOR1PWM);
    pwm1.period(Period);
    pwm1.write(1.0);

    PwmOut pwm2(MOTOR2PWM);
    pwm2.period(Period);
    pwm2.write(1.0);

    int ble_errors = 0;

    ble_errors += ble.addCallback(TOGGLE_LED_SIGNAL, &toggle_led);
    ble_errors += ble.addCallback(TOGGLE_PRINT_ENCODERS_SIGNAL, &toggle_print);
    ble_errors += ble.addCallback(TOGGLE_PRINT_SERIAL_SIGNAL, &toggle_print_serial);
    ble_errors += ble.addCallback(TOGGLE_STOP_COND_SIGNAL, &toggleStopCondition);       // Signal for turning on / off automatic stopping when no line is sensed
    ble_errors += ble.addCallback(TOGGLE_INTERPOLATION, &change_interp);
    ble_errors += ble.addCallback(TOGGLE_ENABLE_SIGNAL, &stopStart);
    ble_errors += ble.addCallback(TOGGLE_SENSOR_USAGE_SIGNAL, &use_sensors);

    ble_errors += ble.addCallback(CHANGE_VALUE_SIGNAL, &setValueTo);                    // uses FLOAT as arg for change value
    ble_errors += ble.addCallback(CHANGE_VALUE_QUICK_SIGNAL, &setValueToQuick);         // uses INT as arg for change value instead of float.
    ble_errors += ble.addCallback(CHANGE_VALUE_BEING_CHANGED_SIGNAL, &changeValueBeingSet);
    ble_errors += ble.addCallback(SET_SPEED_QUICK, &speedsTo);   

    ble_errors += ble.addCallback(ENABLE_MOTORS_SIGNAL, &disable_motors);
    ble_errors += ble.addCallback(DISABLE_MOTORS_SIGNAL, &enable_motors);

    ble_errors += ble.addCallback(ENABLE_TICKERS_SIGNAL, &enable_tickers);
    ble_errors += ble.addCallback(DISABLE_TICKERS_SIGNAL, &disable_tickers);

    ble_errors += ble.addCallback(ROTATE_180_SIGNAL, &uturn);

    // #ifndef USE_RC_MODE 
    // if(ble_errors != 0){
    //     pc.printf("Error Adding BLE Callbacks!");
    //     pc.printf("\nCBs Added = %d", ble_errors);
    //     for(;;){
    //         led = !led;
    //         wait(0.2);
    //     }
    // }
    // #endif

    #ifdef USE_RC_MODE
    ble_errors += ble.addCallback(FORWARD_SIGNAL, &forward);
    ble_errors += ble.addCallback(BACKWARD_SIGNAL, &backward);
    ble_errors += ble.addCallback(LEFT_SIGNAL, &left);
    ble_errors += ble.addCallback(RIGHT_SIGNAL, &right);

    if(ble_errors != 0){
        pc.printf("Error Adding BLE Callbacks!");
        for(;;){
            led = !led;
            wait(0.2);
        }
    }
    #endif

    direction1 = 0;
    direction2 = 1;

    t.attach(&printData, DATA_MEASURE_FREQ);
    t2.attach(&calcPID, PID_ENCODER_FREQ);

    DigitalOut bipolar1(BIPOLAR1);
    bipolar1 = 0;
    
    DigitalOut bipolar2(BIPOLAR2);
    bipolar2 = 0;



    Encoder encoder1(ENCODER1_CHA, ENCODER1_CHB, 256, 0.08, ENCODER_MEASURE_RATE);
    Encoder encoder2(ENCODER2_CHA, ENCODER2_CHB, 256, 0.08, ENCODER_MEASURE_RATE);

    PIDController first_motor_controller(0.15, 0.3, 0.001, PID_ENCODER_FREQ);
    PIDController second_motor_controller(0.13, 0.2, 0.001, PID_ENCODER_FREQ);
    PIDController navigation_controller(1.0, 0.0, 0.0, PID_ENCODER_FREQ);

    // pwm1.write(0.5);
    // pwm2.write(0.5);    

    double write_val1;
    double write_val2;

    DigitalIn button(USER_BUTTON);

    LineSensor lineSensorManager(SENSOR_BANK_1_PIN, SENSOR_BANK_2_PIN, LINE_SENSOR_1_PIN, LINE_SENSOR_2_PIN, LINE_SENSOR_3_PIN, LINE_SENSOR_4_PIN);

    lineSensorManager.enableBank1();
    lineSensorManager.enableBank2();

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
    bool decelerating = false;
    printSerial = false;

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
                        printf("DESIRED3 = %f\n", current_speed_1 + navigation_bias);
                        printf("DESIRED4 = %f\n", current_speed_2 - navigation_bias);
                    }
                    else{
                        printf("Sensor1 = %f\n", lineSensorManager.getAverageSensorValue(1));
                        printf("Sensor2 = %f\n", lineSensorManager.getAverageSensorValue(2));
                        // printf("Sensor3 = %f\n", lineSensorManager.getAverageSensorValue(3));
                        // printf("Sensor4 = %f\n", lineSensorManager.getAverageSensorValue(4));
                        // printf("Sensor5 = %f\n", navigation_bias);
                        // printf("P1 = %lf\n", navigation_controller.getP());
                        // printf("I2 = %lf\n", navigation_controller.getI());
                        // printf("D3 = %lf\n", navigation_controller.getD());
                    }
                }
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
                        if(stop_count > 100){
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

                // basic interpolation
                if(toggle_interp){
                    current_speed_1 = (desired_speed_1 * ACCELERATION_RATE_PER_TICK) + last_tick_current_speed_1*(1-ACCELERATION_RATE_PER_TICK);
                    last_tick_current_speed_1 = current_speed_1;

                    current_speed_2 = (desired_speed_2 * ACCELERATION_RATE_PER_TICK) + last_tick_current_speed_2*(1-ACCELERATION_RATE_PER_TICK);
                    last_tick_current_speed_2 = current_speed_2;
                }
                else{
                    current_speed_1 = desired_speed_1;
                    current_speed_2 = desired_speed_2;
                    last_tick_current_speed_1 = current_speed_1;
                    last_tick_current_speed_2 = current_speed_2;
                }

                // scale error logarithmically based on the current speeds
                error_scale = (current_speed_1 + current_speed_2)/2;
                error_scale = 20 * pow(0.01 * error_scale, 0.5);
                if(error_scale > 4.0){
                    error_scale = 4.0;
                }

                if(useSensors){
                    navigation_bias = navigation_controller.calculate(bank_1_difference * error_scale, 0.0);
                }
                else{
                    navigation_bias = 0.0;
                }

                if(abs(navigation_bias) > 0.3 && !decelerating){
                    toggle_interp = false;
                    decelerating = true;
                    prev_desired_speed_1 = desired_speed_1;
                    prev_desired_speed_2 = desired_speed_2;
                    desired_speed_1 = 0.5;
                    desired_speed_2 = 0.5;
                }
                else{
                    decelerating = false;
                    toggle_interp = true;
                    desired_speed_1 = prev_desired_speed_1;
                    desired_speed_2 = prev_desired_speed_2;
                }

                double first_motor_controller_set_point = current_speed_1 + navigation_bias;
                if(first_motor_controller_set_point < 0.0){
                    first_motor_controller_set_point = abs(first_motor_controller_set_point);
                    direction1 = 0;}
                else{
                    direction1 = 1;
                }

                write_val1 = - first_motor_controller.calculate(encoder1.getRPS(), first_motor_controller_set_point);        // 0.7 is needed because otherwise we spike to 0.0
                write_val1 = (write_val1 > 1.0) ? write_val1 = 1.0 : (write_val1 <= 0.0) ? write_val1 = 0.0 : write_val1 = write_val1;
                pwm1.write(write_val1);


                double second_motor_controller_set_point = current_speed_2 - navigation_bias;
                if(second_motor_controller_set_point < 0.0){
                    second_motor_controller_set_point = abs(second_motor_controller_set_point);
                    direction2 = 1;}
                else{
                    direction2 = 0;
                }
                write_val2 = - second_motor_controller.calculate(encoder2.getRPS(), second_motor_controller_set_point);
                write_val2 = (write_val2 > 1.0) ? write_val2 = 1.0 : (write_val2 <= 0.0) ? write_val2 = 0.0 : write_val2 = write_val2;
                pwm2.write(write_val2);

                state = normal;
                calculatingPID = false;
                break;

            case reset_controllers:
                first_motor_controller.reset();
                second_motor_controller.reset();
                navigation_controller.reset();
                state = normal;
                enable_tickers();
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
            case change_val:
                switch(value){
                    case speed1:
                        if(changeTo < 0.0){direction1 = 0;}else{direction1 = 1;}
                            desired_speed_1 = abs(changeTo);
                            prev_desired_speed_1 = abs(changeTo);
                        break;
                    case speed2:
                        if(changeTo < 0.0){direction2 = 0;}else{direction2 = 1;}
                            desired_speed_2 = abs(changeTo);
                            prev_desired_speed_2 = abs(changeTo);
                        break;
                    case pro1:
                        first_motor_controller.setP((double)changeTo);
                        break;
                    case int1:
                        first_motor_controller.setI((double)changeTo);
                        first_motor_controller.reset();
                        break;
                    case der1:
                        first_motor_controller.setD((double)changeTo);
                        first_motor_controller.reset();
                        break;
                    case pro2:
                        second_motor_controller.setP((double)changeTo);
                        break;
                    case int2:
                        second_motor_controller.setI((double)changeTo);
                        second_motor_controller.reset();
                        break;
                    case der2:
                        second_motor_controller.setD((double)changeTo);
                        second_motor_controller.reset();
                        break;
                    case pro3:
                        navigation_controller.setP((double)changeTo);
                        break;
                    case int3:
                        navigation_controller.setI((double)changeTo);
                        navigation_controller.reset();
                        break;
                    case der3:
                        navigation_controller.setD((double)changeTo);
                        navigation_controller.reset();
                        break;
                }
                state = normal;
                break;

            // case change_p1:
            //     first_motor_controller.setP(p1);
            //     state = normal;
            //     break;
            // case change_i1:
            //     first_motor_controller.setI(i1);
            //     first_motor_controller.reset();
            //     state = normal;
            //     break;
            // case change_d1:
            //     first_motor_controller.setD(d1);
            //     first_motor_controller.reset();
            //     state = normal;
            //     break;
            // case change_p2:
            //     second_motor_controller.setP(p2);
            //     state = normal;
            //     break;
            // case change_i2:
            //     second_motor_controller.setI(i2);
            //     second_motor_controller.reset();
            //     state = normal;
            //     break;
            // case change_d2:
            //     second_motor_controller.setD(d2);
            //     second_motor_controller.reset();
            //     state = normal;
            //     break;
            // case change_p3:
            //     navigation_controller.setP(p3);
            //     state = normal;
            //     break;
            // case change_i3:
            //     navigation_controller.setI(i3);
            //     navigation_controller.reset();
            //     state = normal;
            //     break;
            // case change_d3:
            //     navigation_controller.setD(d3);
            //     navigation_controller.reset();
            //     state = normal;
            //     break;
        }
    }
}