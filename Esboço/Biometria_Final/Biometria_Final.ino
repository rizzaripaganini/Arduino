
//------Catracas Arduino V1.0  - Biometria Suprema BioEntry W2------
// Ligação do tipo normalmente Aberto
//---------------------LIGAÇÕES DO ARDUINO--------------------------
//Pino BIOMETRIA_NA PORTA PIN-4  CABO BRANCO DA LEITORA BIOMÉTRICA
//Pino RELE_PD8 PORTA PIN-5 (ARDUINO)
//Pino RELE_PD8 (+) 5V     (ARDUINO)
//Pino RELE_PD8 (-) GDN    (ARDUINO)
//Pino BIOMETRIA_NA (+) 5V (ARDUINO)
//------------------------------------------------------------------

//----------------LIGAÇÕES DO SOLENOIDE E FONTE 12V-----------------
//SAIDA DO BORNE CENTRAL O RELE
//SAIDA 12V NORMALMENTE FECHADA 12(+)   (FONTE 12V)
//ALIMENTAÇÃO DA BIOMETRIA 12V (+) (FONTE 12V)
//NEGATIVO DA BIOMETRIA (-) (FONTE 12V)
//-------------------------------------------------------------------
// ==============================================================================
//           DIAGRAMA DE LIGAÇÃO - LEITOR BIOMÉTRICO (SUPREMA) E ARDUINO
// ==============================================================================
//
//        [ FONTE 12V ]
//         (+)     (-)
//          |       |
//          |       +-----------------------------------+
//          |                                           |
//          v                                           v
//   +-------------------------------------------------------------+
//   |             SUPREMA BIOENTRY W2 (LEITOR BIOMÉTRICO)         |
//   |                                                             |
//   |  [12V IN]   [GND]               [RELÉ COM]      [RELÉ NO]   |
//   +--------------------------------------|---------------|------+
//                                          |               |
//                                          |               |
//                                          |               |
//           O relé interno da Suprema      |               |
//           fecha o contato quando a       v               v
//           biometria é aceita.        [ GND AZ ]    [ PINO 4 BC]
//                                  +-------------------------------+
//                                  |           ARDUINO             |
//                                  +-------------------------------+
//
// ==============================================================================
// ==============================================================================
//                      DIAGRAMA DE LIGAÇÃO - CATRACA
// ==============================================================================
//
//       [ FONTE 12V ]
//       (+)       (-)
//        |         |
//        |         |----------------------------------------------------+
//        |                                                              |
//        |             +-----------------------------------------+      |
//        |             |                                         |      |
//      [COM]         [NO]                                        |      |
//   +----------------------+                                     |      |
//   |                      |                                     |      |
//   |     MÓDULO RELÉ      |                                     |      |
//   |                      |                                  +------------+
//   +----------------------+                                  |            |
//     [IN]  [GND]  [VCC]                                      | SOLENOIDE  |
//       |     |      |                                        | (Catraca)  |
//       |     |      |                                        |            |
//       |     |      |                                        +------------+
//      [D5] [GND]   [5V]
//   +----------------------+
//   |       ARDUINO        |
//   +----------------------+
//
// ==============================================================================
// -------------------- Definição dos Pinos -------------------------
const int BIOMETRIA_NA = 4;  // Pino que recebe o sinal NA do leitor
const int RELE_PD8 = 5;


// A logca da biometria
#define BIOMETRIA_LIBERADA LOW
#define BIOMETRIA_NAO_LIBERADA HIGH

// A lógica para o relay
#define RELAY_ARMADO LOW
#define RELAY_DESARRMADO HIGH

bool biometria_livre_para_leitura = true;

void setup() {
  Serial.begin(115200);

  // Configura o pino do leitor com resistor de pull-up interno.
  // Ficará em HIGH por padrão e irá para LOW quando o leitor atracar.
  pinMode(BIOMETRIA_NA, INPUT_PULLUP);

  // Configura o pino da catraca como saída
  pinMode(RELE_PD8, OUTPUT);

  // LÓGICA INVERTIDA: Garante que o relé inicie e permaneça ARMADO (HIGH)
  digitalWrite(RELE_PD8, RELAY_ARMADO);
  digitalWrite(BIOMETRIA_NA, BIOMETRIA_NAO_LIBERADA);

  Serial.println("Sistema Iniciado. Rele ARMADO (Travado). Aguardando biometria...");
}

void loop() {
  delay(1000);
  // Lê o estado atual do pino do leitor Suprema
  int ESTADO_DO_LEITOR = digitalRead(BIOMETRIA_NA);
  int ESTADO_DO_RELAY = digitalRead(RELE_PD8);

  if (biometria_livre_para_leitura == true && ESTADO_DO_LEITOR == BIOMETRIA_LIBERADA) {
    Serial.println("Biometria liberada!!!");
    biometria_livre_para_leitura = false;
    digitalWrite(BIOMETRIA_NA, BIOMETRIA_NAO_LIBERADA);
    digitalWrite(RELE_PD8, RELAY_DESARRMADO);

    delay(5000);
    digitalWrite(RELE_PD8, RELAY_ARMADO);
    biometria_livre_para_leitura = true;
  }
}
