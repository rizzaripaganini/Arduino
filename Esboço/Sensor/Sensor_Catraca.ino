
int SENSOR_01 = 0;  // Sensor 1 
int SENSOR_02 = 1;  // Sensor 2
const int RELE = 8;    // Rele

void setup() {
  pinMode(SENSOR_01, INPUT_PULLUP); 
  pinMode(SENSOR_02, INPUT_PULLUP); 
  pinMode(RELE, OUTPUT); 
  Serial.begin(115200); 

  digitalWrite(RELE, LOW);        
}

void loop() {
  double ESTADO_S1 = analogRead(SENSOR_01); 
  double ESTADO_S2 = analogRead(SENSOR_02);

  //Serial.println(ESTADO_S1);
  //Serial.println("NÂO ACIONADO ...");
   // Lógica para acionar o relé
  if (ESTADO_S1 == 20.0) { // Se qualquer sensor detectar um obstáculo (LOW)
    Serial.println("S1 ACIONADO....");
    Serial.println(ESTADO_S1);
    digitalWrite(RELE, HIGH);
    delay(500); 
  }

   // Lógica para acionar o relé
  if (ESTADO_S2 == 20.0) { // Se qualquer sensor detectar um obstáculo (LOW)
    Serial.println("S2 ACIONADO....");
    Serial.println(ESTADO_S2);
    digitalWrite(RELE, HIGH);
    delay(500); 
  }
  //delay(2000);
}
