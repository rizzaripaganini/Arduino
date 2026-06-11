const int SENSOR_01 = 6;
const int SENSOR_02 = 7;
const int LED_01 = 5;
const int LED_02 = 4;


#define SENSOR_ESTADO_TRAVADO LOW
#define SENSOR_ESTADO_LIBERADO HIGH
#define LED_01 HIGH
#define LED_02 HIGH

int ULTIMO_ESTADO_S1;
int ULTIMO_ESTADO_S2;
int APAGADO;
int LIGADO;

void setup() {
  Serial.begin(115200);
  pinMode(SENSOR_01, INPUT_PULLUP);
  pinMode(SENSOR_02, INPUT_PULLUP);
  pinMode(LED_01, OUTPUT);
  pinMode(LED_02, OUTPUT);

  ULTIMO_ESTADO_S1 = digitalRead(SENSOR_01);
  ULTIMO_ESTADO_S2 = digitalRead(SENSOR_02);
}

void loop() {
  int ESTADO_S1 = digitalRead(SENSOR_01);
  int ESTADO_S2 = digitalRead(SENSOR_02);

  // Switch para o Sensor 01
  switch (ESTADO_S1) {
    case HIGH: // Equivalente a > 0
      Serial.print(" S1: ");
      Serial.print(ESTADO_S1 );

      break;
      
    case LOW:  // Equivalente a == 0
      Serial.print(" S1: ");
      Serial.print(ESTADO_S1 );
      Serial.println("   ");
      break;
  }

  // Switch para o Sensor 02
  switch (ESTADO_S2) {
    case HIGH: // Equivalente a > 0
      Serial.print(" S2: ");
      Serial.print(ESTADO_S2 );
      Serial.println("   ");
      break;
      
    case LOW:  // Equivalente a == 0
      Serial.print(" S2: ");
      Serial.print(ESTADO_S2 );
      
      break;
  }
}
