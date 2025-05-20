

#ifndef MotorController_h
#define MotorController_h
#include "Arduino.h"

class MotorController {
  public:
    byte dcMotor[4][3];
    int currentSpeed[4];

    String MOTOR_FORWARD[4];
    String MOTOR_BACK[4];

    MotorController(byte motor1[3], byte motor2[3], byte motor3[3], byte motor4[3]);

    void motorSetter(byte motor1[3], byte motor2[3], byte motor3[3], byte motor4[3]);
    void motor_direction_setter(byte motor_id, String forward, String back);
    void moveMotor(byte motor[3], String turn, byte motor_speed);
    void stopMotor(byte pwm);
    void stopAll();
    void printMotorDefinition();
    void moveForward(byte motor_speed);
    void moveBack(byte motor_speed);
    void rotate(String turnDirection, byte motor_speed);
    void moveParallel(String parallelDirection, byte motor_speed);
    void moveDiagonal(String turnDirection, byte motor_speed);
    void moveBias(byte motor_id, byte motor_speed, byte bias, byte move_weights);
    int smoothSpeed(int targetSpeed, int currentSpeed, int step = 5);
};

#endif
