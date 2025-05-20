void setup() {
  // put your setup code here, to run once:
    Serial.begin(9600);
    pinMode(10, OUTPUT);
    pinMode(11, OUTPUT);

}

char recv;

void loop() {
  // put your main code here, to run repeatedly:
    if (Serial.available() > 0) {
        Serial.println("Reading:");
        recv = Serial.read();
        if (recv == '5') {
            digitalWrite(11, LOW);
            digitalWrite(10, LOW);
        } else if (recv == '6') {
            digitalWrite(11, HIGH);
            digitalWrite(10, LOW);
        } else if (recv == '7') {
            digitalWrite(11, LOW);
            digitalWrite(10, HIGH);
        } else {

        }
        Serial.println(recv);
    }
}
