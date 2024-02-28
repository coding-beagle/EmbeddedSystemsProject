// pin configs
#define MOTOR1PWM D5
#define BIPOLAR1 D4 
#define DIRECTION1 PB_12

#define MOTOR2PWM D3
#define BIPOLAR2 D7
#define DIRECTION2 PC_4

#define ENABLE_PIN D9

#define BLE_RX D10
#define BLE_TX D2
#define BLE_PINS_TUPLE BLE_RX, BLE_TX, BLE_BAUD

#define ENCODER1_CHA PC_3
#define ENCODER2_CHA PC_2

// Settings

#define Frequency 25000.0
#define Period 1.0/Frequency

#define ENCODER_MEASURE_RATE 0.01
#define PID_FREQ 0.01
#define DATA_MEASURE_FREQ 0.1
#define BLE_POLL_FREQ 0.01

#define BLE_BAUD 9600

#define ACCELERATION_RATE_PER_TICK 0.001

// CALLBACK SIGNALS FOR STANDARDISING THINGS
#define TOGGLE_LED_SIGNAL 99

#define SET_MOTOR1_P_SIGNAL 44
#define SET_MOTOR1_I_SIGNAL 45
#define SET_MOTOR1_D_SIGNAL 46

#define SET_MOTOR2_P_SIGNAL 54
#define SET_MOTOR2_I_SIGNAL 55
#define SET_MOTOR2_D_SIGNAL 56

#define SET_MOTOR1_SPEED_SIGNAL 40
#define SET_MOTOR2_SPEED_SIGNAL 50