    #include <Keyboard.h>
const int IR_PIN=2, RED_PIN=5, GREEN_PIN=9, BLUE_PIN=10, WHITE_PIN=11;

void setup() {  
  Keyboard.begin();
  pinMode(IR_PIN, OUTPUT); pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT); pinMode(BLUE_PIN, OUTPUT); pinMode(WHITE_PIN, OUTPUT); 

}
void loop() {
  Keyboard.write('A');
  digitalWrite(IR_PIN, HIGH);
  delay(1000);
  digitalWrite(IR_PIN, LOW);
  delay(1000);
  Keyboard.write('A');
  analogWrite(RED_PIN, floor(255*2/3));
  delay(1000);
  analogWrite(RED_PIN, floor(255*2/3));
  delay(1000);
  Keyboard.write('A');
  digitalWrite(GREEN_PIN, HIGH);
  delay(1000);
  digitalWrite(GREEN_PIN, LOW);
  delay(1000);
Keyboard.write('A');
  digitalWrite(BLUE_PIN, HIGH);
  delay(1000);
  digitalWrite(BLUE_PIN, LOW);
  delay(1000);
Keyboard.write('A');
  digitalWrite(WHITE_PIN, HIGH);
  delay(1000);
  digitalWrite(WHITE_PIN, LOW);  
}
