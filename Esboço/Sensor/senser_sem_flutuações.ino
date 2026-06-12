int SENSOR_01 = A0;  

void setup() {
  Serial.begin(9600);       
}

void loop() {
  long soma = 0;
  int quantidadeLeituras = 10;

  // Faz 10 leituras rápidas
  for (int i = 0; i < quantidadeLeituras; i++) {
    soma = soma + analogRead(SENSOR_01);
    delay(5); // pequeno intervalo entre as leituras
  }

  // Calcula a média
  int media = soma / quantidadeLeituras;

  Serial.println(media); // Esse valor será muito mais estável!

  delay(100);
}
