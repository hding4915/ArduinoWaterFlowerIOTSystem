

#include "MotorController.h"
#include "Arduino.h"


MotorController::MotorController(byte motor1[3], byte motor2[3], byte motor3[3], byte motor4[3]) {
  motorSetter(motor1, motor2, motor3, motor4);
}

void MotorController::motorSetter(byte motor1[3], byte motor2[3], byte motor3[3], byte motor4[3]) {

  for (byte i = 0; i < 3; i++) {
    dcMotor[0][i] = motor1[i];
    dcMotor[1][i] = motor2[i];
    dcMotor[2][i] = motor3[i];
    dcMotor[3][i] = motor4[i];
  }
  for (byte j = 0; j < 4; j++) {
    for (byte i = 0; i < 2; i++) {
      pinMode(dcMotor[j][i], OUTPUT);
      digitalWrite(dcMotor[j][i], LOW);
    }
  }
  for (int i = 0; i < 4; i++) {
    currentSpeed[i] = 0;
  }
}

void MotorController::printMotorDefinition() {
  for (byte j = 0; j < 4; j++) {
    Serial.print("$: ");
    for (byte s = 0; s < 3; s++) {
        if (s == 0) Serial.print("[");
        Serial.print(dcMotor[j][s]);
        if (s != 2) Serial.print(", ");
        else Serial.print("]");
    }
    if (j % 2 != 0) Serial.println('\n');
    else Serial.print("  ");
  }
  for (byte j = 0; j < 4; j++) {
    String details = "MOTOR" + String(j + 1) + ": " + String(MOTOR_FORWARD[j] + " " + String(MOTOR_BACK[j]));
    Serial.println(details);
  }
}

void MotorController::motor_direction_setter(byte motor_id, String forward, String back) {
  MOTOR_FORWARD[motor_id - 1] = forward;
  MOTOR_BACK[motor_id - 1] = back;
}

void MotorController::moveMotor(byte motor[3], String turn, byte motor_speed) {
  int motorIndex = 0;
  
  // 根據給定的馬達引腳陣列選擇對應的馬達
  if (motor == dcMotor[0]) motorIndex = 0;
  else if (motor == dcMotor[1]) motorIndex = 1;
  else if (motor == dcMotor[2]) motorIndex = 2;
  else if (motor == dcMotor[3]) motorIndex = 3;

  // 平滑速度調整
  currentSpeed[motorIndex] = smoothSpeed(motor_speed, currentSpeed[motorIndex]);

  // 設定 PWM 信號（根據方向）
  if (turn == "forward") {
    digitalWrite(motor[0], HIGH);
    digitalWrite(motor[1], LOW);
  } else if (turn == "back") {
    digitalWrite(motor[0], LOW);
    digitalWrite(motor[1], HIGH); 
  }
  analogWrite(motor[2], motor_speed);  // ENA (PWM)
}

void MotorController::stopMotor(byte pwm) {
  analogWrite(pwm, 0);
}

void MotorController::stopAll() {
  stopMotor((byte)dcMotor[0][2]);
  stopMotor((byte)dcMotor[1][2]);
  stopMotor((byte)dcMotor[2][2]);
  stopMotor((byte)dcMotor[3][2]);
}


int MotorController::smoothSpeed(int targetSpeed, int currentSpeed, int step) {
  if (currentSpeed < targetSpeed) {
    return min(currentSpeed + step, targetSpeed); // 增加速度
  } else if (currentSpeed > targetSpeed) {
    return max(currentSpeed - step, targetSpeed); // 降低速度
  }
  return targetSpeed; // 已經達到目標速度
}



void MotorController::moveForward(byte motor_speed) {
  for (byte i = 0; i < 4; i++) {
    moveMotor(dcMotor[i], MOTOR_FORWARD[i], motor_speed);
  }
//  Serial.println("Move Forward!!");
}

void MotorController::moveBack(byte motor_speed) {
  for (byte i = 0; i < 4; i++) {
    moveMotor(dcMotor[i], MOTOR_BACK[i], motor_speed);
  }
//  Serial.println("Move Forward!!");
}

void MotorController::rotate(String turnDirection, byte motor_speed) {
  String direction1 = "";
  String direction2 = "";
  if (turnDirection == "right") {
    moveMotor(dcMotor[0], MOTOR_BACK[0], motor_speed);
    moveMotor(dcMotor[1], MOTOR_BACK[1], motor_speed);
    moveMotor(dcMotor[2], MOTOR_FORWARD[2], motor_speed);
    moveMotor(dcMotor[3], MOTOR_FORWARD[3], motor_speed);
  } else if (turnDirection == "left") {
    moveMotor(dcMotor[0], MOTOR_FORWARD[0], motor_speed);
    moveMotor(dcMotor[1], MOTOR_FORWARD[1], motor_speed);
    moveMotor(dcMotor[2], MOTOR_BACK[2], motor_speed);
    moveMotor(dcMotor[3], MOTOR_BACK[3], motor_speed);
  }
}

void MotorController::moveBias(byte motor_id, byte motor_speed, byte bias, byte move_weights) {
  for (byte i = 0; i < 4; i++) {
    if (i == motor_id) {
      moveMotor(dcMotor[i], MOTOR_FORWARD[i], motor_speed + (bias * move_weights));
    }
    else {
      moveMotor(dcMotor[i], MOTOR_FORWARD[i], motor_speed);
    }
  }
}


void MotorController::moveParallel(String parallelDirection, byte motor_speed) {
  if (parallelDirection == "right") {
    moveMotor(dcMotor[0], MOTOR_BACK[0], motor_speed);
    moveMotor(dcMotor[1], MOTOR_FORWARD[1], motor_speed + 20);
    moveMotor(dcMotor[2], MOTOR_FORWARD[2], motor_speed + 20);
    moveMotor(dcMotor[3], MOTOR_BACK[3], motor_speed);
  } else if (parallelDirection == "left") {
    moveMotor(dcMotor[0], MOTOR_FORWARD[0], motor_speed);
    moveMotor(dcMotor[1], MOTOR_BACK[1], motor_speed + 20);
    moveMotor(dcMotor[2], MOTOR_BACK[2], motor_speed + 20);
    moveMotor(dcMotor[3], MOTOR_FORWARD[3], motor_speed);
  }
}


void MotorController::moveDiagonal(String turnDirection, byte motor_speed) {
    if (turnDirection == "right") {
        stopMotor(dcMotor[0][2]);
        stopMotor(dcMotor[2][2]);
        moveMotor(dcMotor[1], MOTOR_FORWARD[1], motor_speed);
        moveMotor(dcMotor[3], MOTOR_FORWARD[3], motor_speed);
    } else if (turnDirection == "left") {
        stopMotor(dcMotor[1][2]);
        stopMotor(dcMotor[3][2]);
        moveMotor(dcMotor[0], MOTOR_FORWARD[0], motor_speed);
        moveMotor(dcMotor[2], MOTOR_FORWARD[2], motor_speed);
    }
}



