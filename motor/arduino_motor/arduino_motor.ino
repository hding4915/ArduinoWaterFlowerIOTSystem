#include "MotorController.h"





// Define Motor Move Direction
#define L298N1_FORWARD "forward"
#define L298N1_BACK "back"
#define L298N2_FORWARD "forward"
#define L298N2_BACK "back"
#define L298N3_FORWARD "forward"
#define L298N3_BACK "back"
#define L298N4_FORWARD "forward"
#define L298N4_BACK "back"


// Define Motor Speed
#define SPEED_FORWARD 100
#define SPEED_BACKWARD 255
#define SPEED_TURN 100

// Configure Motor Pins
byte motor1[3] = { 31, 30, 2 };
byte motor2[3] = { 32, 33, 3 };
byte motor3[3] = { 35, 34, 6 };
byte motor4[3] = { 36, 37, 5 };


// byte motor1[3] = { 4, 2, 3 };
// byte motor2[3] = { 7, 8, 5 };
// byte motor3[3] = { 13, 12, 6 };
// byte motor4[3] = { 9, 10, 11 };

MotorController controller = MotorController(motor1, motor2, motor3, motor4);



void setup() {
    TCCR3B = (TCCR3B & 0b11111000) | 0x03;  // 分頻 64
    TCCR4B = (TCCR4B & 0b11111000) | 0x03;  // 分頻 64

    pinMode(A0, INPUT);
    pinMode(A1, INPUT);
    pinMode(42, OUTPUT);
    pinMode(43, OUTPUT);


    digitalWrite(42, LOW);
    digitalWrite(43, LOW);

    Serial.begin(9600);
    controller.motor_direction_setter(1, L298N1_FORWARD, L298N1_BACK);
    controller.motor_direction_setter(2, L298N2_FORWARD, L298N2_BACK);
    controller.motor_direction_setter(3, L298N3_FORWARD, L298N3_BACK);
    controller.motor_direction_setter(4, L298N4_FORWARD, L298N4_BACK);


    //   Serial.println("Motor Pins:");
    //   for (int j = 0; j < 4; j++) {
    //     Serial.print("\tMotor " + String(j + 1) + "\n\t\t");
    //     for (int i = 0; i < 3; i++) {
    //       Serial.print(String(controller.dcMotor[j][i]) + " ");
    //     }
    //     Serial.println("");
    //   }

    //   controller.printMotorDefinition();
}

int left, right;
int fast = 80;
int slow = 30;
bool is_flood = false;
bool timer_start = false;
bool to_go = true;
bool need_flood = false;

unsigned long timer;
String recv;
int humidity = -1;
int counterH = 0;


void loop() {
    if (!need_flood && Serial.available()) {
        recv = Serial.readStringUntil('>');
        if (recv.startsWith("<H:")) {
            String humStr = recv.substring(3);
            humidity = humStr.toInt();
            Serial.print("Humidity: ");
            Serial.println(humidity);
        }
    }

    if (humidity > 250) {
        need_flood = true;
    }

    if (need_flood) {

        right = digitalRead(A0);
        left = digitalRead(A1);
        Serial.println("left: " + String(left) + ", ana: " + String(analogRead(A1)) + ", right: " + String(right) + ", ana: " + String(analogRead(A0)));

        if (left && !right && to_go) {
            is_flood = false;
            timer_start = false;
            controller.moveMotor(motor1, "forward", slow);
            controller.moveMotor(motor2, "forward", fast);
            controller.moveMotor(motor3, "forward", slow);
            controller.moveMotor(motor4, "forward", fast);
        } else if (!left && right && to_go) {
            is_flood = false;
            timer_start = false;
            controller.moveMotor(motor1, "forward", fast);
            controller.moveMotor(motor2, "forward", slow);
            controller.moveMotor(motor3, "forward", fast);
            controller.moveMotor(motor4, "forward", slow);
        } else if (left && right) {
            controller.stopAll();
            if (!timer_start) {
                timer = millis();
                timer_start = true;
            }

            if (!is_flood && (millis() - timer) > 800) {
                to_go = false;
                Serial.println("flood!!!!");
                delay(500);
                digitalWrite(42, HIGH);
                digitalWrite(43, LOW);
                delay(5000);
                digitalWrite(42, LOW);
                digitalWrite(43, LOW);
                delay(1000);
                is_flood = true;
                to_go = true;
                counterH++;
                if (counterH == 4) {
                    need_flood = false;
                    counterH = 0;
                }
                controller.moveForward(50);
                delay(600);
            }



        } else {
            is_flood = false;
            timer_start = false;
        }
    }
}
