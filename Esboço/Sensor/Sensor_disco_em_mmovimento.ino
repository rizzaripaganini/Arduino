const int SENSOR_CATRACA = 2; // Pino do sensor óptico

// Variáveis de controle de tempo
volatile unsigned long tempoUltimoPulso = 0; 
unsigned long tempoLimiteParada = 500; // Tempo (em ms) sem pulso para considerar parado (0.5s)
unsigned long tempoDebounce = 35;      // Filtro (em ms) para ignorar tremores com a catraca parada

int estadoMovimento = 0;  // 0 = Parado, 1 = Em movimento
int estadoAnterior = -1;  // Controle para não repetir mensagens no terminal

void setup() {
  pinMode(SENSOR_CATRACA, INPUT);
  Serial.begin(9600);
  
  // Interrupção: avisa o Arduino sempre que o disco se mover
  attachInterrupt(digitalPinToInterrupt(SENSOR_CATRACA), sensorInterrupcao, FALLING);
  
  Serial.println("Sistema Pronto. Aguardando movimento...");
}

void loop() {
  // Captura o tempo atual do Arduino
  unsigned long tempoAtual = millis();

  // Se o tempo atual menos o tempo do último pulso for MAIOR que o limite, a catraca parou
  if (tempoAtual - tempoUltimoPulso > tempoLimiteParada) {
    estadoMovimento = 0; // Força o valor para 0 (Parado)
  } else {
    estadoMovimento = 1; // Mantém em 1 (Movimento)
  }

  // Só envia para o terminal se o estado realmente MUDOU
  // Evita que o terminal fique poluído quando a catraca estiver parada
  if (estadoMovimento != estadoAnterior) {
    Serial.print("Status do Disco: ");
    Serial.println(estadoMovimento);
    
    estadoAnterior = estadoMovimento; // Atualiza o estado anterior
  }
}

// Função rápida disparada pelo sensor quando o disco gira
void sensorInterrupcao() {
  unsigned long tempoAgora = millis();
  
  // FILTRO: Só aceita o pulso se o tempo desde o último movimento for maior que o debounce
  // Isso mata qualquer oscilação ou ruído mecânico da catraca parada
  if (tempoAgora - tempoUltimoPulso > tempoDebounce) {
    tempoUltimoPulso = tempoAgora; // Registra o momento exato do movimento
  }
}
