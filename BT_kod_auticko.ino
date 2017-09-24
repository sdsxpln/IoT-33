#define pred_zad 11
#define leva_prava 10
#define leva 7 //5-2
#define prava 6 //4-3
#define vpred 5 //3-4
#define vzad 4 //2-5



byte r = 200;


void setup() {
  Serial1.begin(9600);
  pinMode(leva, OUTPUT);
  pinMode(prava, OUTPUT);
  pinMode(vpred, OUTPUT);
  pinMode(vzad, OUTPUT);
  pinMode(leva_prava, OUTPUT);
  pinMode(pred_zad, OUTPUT);
  analogWrite(leva_prava, 100);
}

void loop() {
  if (Serial1.available() >= 1) {
      digitalWrite(vpred, 1);
    digitalWrite(vzad, 1);
    digitalWrite(leva, 1);
    digitalWrite(prava, 1);
    char i = Serial1.read();
    switch (i) {
      case 'w':
        digitalWrite(vpred, 0);
        analogWrite(pred_zad, r);
        break;
      case 's':
        digitalWrite(vzad, 0);
        analogWrite(pred_zad, r);
        break;
      case 'e':
        digitalWrite(vpred, 0);
        digitalWrite(prava, 0);
        analogWrite(pred_zad, r);
        break;
      case 'q':
        digitalWrite(vpred, 0);
        digitalWrite(leva, 0);
        analogWrite(pred_zad, r);
        break;
      case 'a':
        digitalWrite(vzad, 0);
        digitalWrite(leva, 0);
        analogWrite(pred_zad, r);
        break;
      case 'd':
        digitalWrite(vzad, 0);
        digitalWrite(prava, 0);
        analogWrite(pred_zad, r);
        break;
      case 'r':
        while (Serial1.available() < 3) {
          delay(100);
        }
        r = Serial1.parseInt();
        Serial1.println(r);
        break;
      default:
        Serial1.read();
        break;
    }
    delay(100);
  }
  else {
    if (digitalRead(A0) == 0) {
      Serial1.println("ok");
    }
  }
}
