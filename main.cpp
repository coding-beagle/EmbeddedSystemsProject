#include "LINESENSOR.h"
#include "mbed.h"
#include "PID.h"
#include "ENCODERS.h"
#include "config.h"
#include "BLEMODULE.h"
#include "LINESENSOR.h"

enum programState {normal, showdata, poll_sensor_data, calculatePID, revert_speed, rotate180, change_p1, change_p2, change_p3, change_i1, change_i2, change_i3, change_d1, change_d2, change_d3};
programState state = normal;

void printData(){
    state = showdata;
}

void state_normal(){
    state = revert_speed;
}

void calcPID(){
    state = calculatePID;
}          

float p1 = 1.0, i1 = 0.0, d1 = 10.0;
float p2 = 1.0, i2 = 0.0, d2 = 15.0;
float p3 = 1.0, i3 = 0.0, d3 = 0.0;

volatile double desired_speed_1 = 0.0, desired_speed_2 = 0.0;

DigitalOut direction1(DIRECTION1);
DigitalOut direction2(DIRECTION2);
DigitalOut led(LED2);

void change_p1_val(float val){
    p1 = val;
    state = change_p1;
}

void change_p2_val(float val){
    p2 = val;
    state = change_p2;
}

void change_p3_val(float val){
    p3 = val;
    state = change_p3;
}

void change_i1_val(float val){
    i1 = val;
    state = change_i1;
}

void change_i2_val(float val){
    i2 = val;
    state = change_i2;
}

void change_i3_val(float val){
    i3 = val;
    state = change_i3;
}

void change_d1_val(float val){
    d1 = val;
    state = change_d1;
}

void change_d2_val(float val){
    d2 = val;
    state = change_d2;
}

void change_d3_val(float val){
    d3 = val;
    state = change_d3;
}

void rotate(){
    state = rotate180;
}

void change_desired_speed_1(float val){
    if(val < 0.0){
        direction1 = 0;
    }
    else{
        direction1 = 1;
    }
    desired_speed_1 = abs(val);
}

void change_desired_speed_2(float val){
    if(val < 0.0){
        direction2 = 0;
    }
    else{
        direction2 = 1;
    }
    desired_speed_2 = abs(val);
}

void toggle_led(){
    led = !led;
}

void get_sensor_data(){
    state = poll_sensor_data;
}

int main() {
    DigitalOut enable(ENABLE_PIN);
    enable = 1;

    HM10 ble(BLE_PINS_TUPLE);

    ble.addCallback(SET_MOTOR1_P_SIGNAL, &change_p1_val);
    ble.addCallback(SET_MOTOR1_I_SIGNAL, &change_i1_val);
    ble.addCallback(SET_MOTOR1_D_SIGNAL, &change_d1_val);

    ble.addCallback(SET_MOTOR2_P_SIGNAL, &change_p2_val);
    ble.addCallback(SET_MOTOR2_I_SIGNAL, &change_i2_val);
    ble.addCallback(SET_MOTOR2_D_SIGNAL, &change_d2_val);

    ble.addCallback(SET_NAVIGATION_P_SIGNAL, &change_p3_val);
    ble.addCallback(SET_NAVIGATION_I_SIGNAL, &change_i3_val);
    ble.addCallback(SET_NAVIGATION_D_SIGNAL, &change_d3_val);

    ble.addCallback(ROTATE_180_SIGNAL, &rotate);
        
    ble.addCallback(SET_MOTOR1_SPEED_SIGNAL, &change_desired_speed_1);
    ble.addCallback(SET_MOTOR2_SPEED_SIGNAL, &change_desired_speed_2);
    
    ble.addCallback(TOGGLE_LED_SIGNAL, &toggle_led);

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

    Encoder encoder1(ENCODER1_CHA, 256, 0.08, ENCODER_MEASURE_RATE);
    Encoder encoder2(ENCODER2_CHA, 256, 0.08, ENCODER_MEASURE_RATE);

    PIDController first_motor_controller(1.0, 0.4, 15.0, PID_ENCODER_FREQ);
    PIDController second_motor_controller(1.0, 0.4, 10.0, PID_ENCODER_FREQ);

    PIDController navigation_controller(0.12, 0.4, 0.001, PID_NAVIGATION_FREQ);
    
    LineSensor line_sensor_manager(SENSOR_BANK_1_PIN, SENSOR_BANK_2_PIN, LINE_SENSOR_1_PIN, LINE_SENSOR_2_PIN, LINE_SENSOR_3_PIN, LINE_SENSOR_4_PIN);

    Ticker t;
    t.attach(&printData, DATA_MEASURE_FREQ);

    Ticker t2;
    t2.attach(&calcPID, PID_ENCODER_FREQ);

    Ticker t3;
    t3.attach(&get_sensor_data, LINESENSOR_POLL_RATE);
    
    Timeout timeout_turn;

    double write_val1;
    double write_val2;

    double temp_desired_speed_1, temp_desired_speed_2;

    DigitalIn button(USER_BUTTON);

    bool isTurning = false;

    Timer timer;
    wait(1);// wait for everything to init
    
    while(1) {

        switch (state){
            case normal:
                if(!button){
                    // printf("Button Pressed");
                    // timer.start();
                    // currentStep = 0;
                }
                ble.doBLE();
                // updateMotorControl(timer);
                break;

            case poll_sensor_data:
                line_sensor_manager.enableBank1();
                line_sensor_manager.enableBank2();

                line_sensor_manager.readSensorValues(0.003);

                line_sensor_manager.disableBank1();
                line_sensor_manager.disableBank2();
                break;
            
            case showdata:
                // uncomment out these lines for checking the RPS with encoder_monitoring.py
                // printf("%f\n", encoder1.getRPS());          
                // printf("%f\n", encoder2.getRPS());

                // uncomment out these lines for checking line sensors with line_sensor_monitoring.py
                printf("%f\n", line_sensor_manager.getAverageSensorValue(1)); 
                printf("%f\n", line_sensor_manager.getAverageSensorValue(2));
                printf("%f\n", line_sensor_manager.getAverageSensorValue(3)); 
                printf("%f\n", line_sensor_manager.getAverageSensorValue(4));
                
                state = normal;
                break;
            case calculatePID:
                double navigation_bias = 0.0;
                if(!isTurning){
                    double sensor_bank_1_error = line_sensor_manager.getAverageSensorValue(1) - line_sensor_manager.getAverageSensorValue(2);
                    double sensor_bank_2_error = line_sensor_manager.getAverageSensorValue(3) - line_sensor_manager.getAverageSensorValue(4);
                    double average_error = (sensor_bank_1_error + sensor_bank_2_error) / 2;

                    double navigation_bias = navigation_controller.calculate(average_error, 0.0);
                }                

                write_val1 = - first_motor_controller.calculate(encoder1.getRPS(), desired_speed_1 + navigation_bias);        // 0.7 is needed because otherwise we spike to 0.0
                write_val1 = (write_val1 > 1.0) ? write_val1 = 1.0 : (write_val1 <= 0.0) ? write_val1 = 0.7 : write_val1 = write_val1;
                pwm1.write(write_val1);

                write_val2 = - second_motor_controller.calculate(encoder2.getRPS(), desired_speed_2 - navigation_bias);
                write_val2 = (write_val2 > 1.0) ? write_val2 = 1.0 : (write_val2 <= 0.0) ? write_val2 = 0.7 : write_val2 = write_val2;
                pwm2.write(write_val2);

                // printf("Write Val 1 = %lf\n Write Val 2 = %lf", write_val1, write_val2);

                state = normal;
                break;
            case revert_speed:
                desired_speed_1 = temp_desired_speed_1;
                desired_speed_2 = temp_desired_speed_2;

                direction1 = 0;
                direction2 = 1;
                isTurning = false;
                state = normal;
                break;

            case rotate180:
                temp_desired_speed_1 = desired_speed_1;
                temp_desired_speed_2 = desired_speed_2;
                isTurning = true;

                desired_speed_1 = 4.0;
                desired_speed_2 = 4.0;

                direction1 = 0;
                direction2 = 0;

                timeout_turn.attach(&state_normal, 0.5);

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
                navigation_controller.setP(p2);
                state = normal;
                break;
            case change_i3:
                navigation_controller.setI(i2);
                navigation_controller.reset();
                state = normal;
                break;
            case change_d3:
                navigation_controller.setD(d2);
                navigation_controller.reset();
                state = normal;
                break;
        }
    }
}