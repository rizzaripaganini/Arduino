int SENSOR_01 = A0;  

void setup() {
  Serial.begin(115200);       
}

void loop() {
  long MAX = 0;
  int QTD = 10;

  // Faz 10 leituras rápidas
  for (int i = 0; i < QTD; i++) {
    MAX = MAX + analogRead(SENSOR_01);
    delay(5); // pequeno intervalo entre as leituras
  }

  // Calcula a média
  int MED = MAX / QTD;

  Serial.println(MED); // Esse valor será muito mais estável!

  delay(10);
}
