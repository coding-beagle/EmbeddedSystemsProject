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

#define ENCODER1_CHA PC_3

#define ENCODER2_CHA PC_2

// Settings

#define Frequency 25000.0
#define Period 1.0/Frequency
#define DelayPeriod 0.2

#define ENCODER_MEASURE_RATE 0.03
#define PID_FREQ 0.01
#define DATA_MEASURE_FREQ 0.05
