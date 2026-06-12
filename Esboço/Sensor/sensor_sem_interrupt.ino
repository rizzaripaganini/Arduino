// O Pino 2 é obrigatório aqui pois suporta Interrupção (Interrupt 0 no Arduino Uno)
const int SENSOR_CATRACA = 2;  

// A variável que vai contar os movimentos precisa ser "volatile" 
// para funcionar bem com a interrupção
volatile int contadorPulsos = 0; 
int pulsosAnteriores = 0;

void setup() {
  pinMode(SENSOR_CATRACA, INPUT_PULLUP); 
  Serial.begin(9600);       

  // Configura a interrupção:
  // - digitalPinToInterrupt(2): Converte o pino 2 para o canal de interrupção
  // - registrarMovimento: É o nome da função que será chamada
  // - FALLING: Só aciona quando o sinal "cai" de HIGH para LOW (quando o dente do disco passa)
  attachInterrupt(digitalPinToInterrupt(SENSOR_CATRACA), registrarMovimento, FALLING);
}

void loop() {
  // Só imprime no Monitor Serial se o número de pulsos mudou
  // Isso evita que o Monitor fique rolando loucamente com a catraca parada
  if (contadorPulsos != pulsosAnteriores) {
    Serial.print("Movimentos detectados: ");
    Serial.println(contadorPulsos);
    pulsosAnteriores = contadorPulsos;
  }
}

// Essa é a função ativada automaticamente pela interrupção
void registrarMovimento() {
  contadorPulsos++;
}
