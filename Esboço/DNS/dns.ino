#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>

const char* ssid = "mpaganini.lan";
const char* password = "46580493";

const byte DNS_PORT = 53;
DNSServer dnsServer;
WebServer server(80);

// Configuração do IP Fixo na sua rede
IPAddress local_IP(192, 168, 1, 10);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress DNS1(192, 168, 1, 10);
IPAddress DNS2(8, 8, 8, 8);

// Variáveis para controlar o tempo de impressão no loop
unsigned long tempoAnterior = 0;
const long intervalo = 10000; // Tempo em milissegundos (10.000 ms = 10 segundos)

void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);

  if (!WiFi.config(local_IP, gateway, subnet, DNS1, DNS2)) {
    Serial.println("Falha ao configurar IP Fixo");
  }

  WiFi.begin(ssid, password);
  Serial.print("Conectando ao Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConectado ao Wi-Fi!");

  // Inicia o servidor DNS respondendo apenas por mpaganini.srv
  dnsServer.start(DNS_PORT, "mpaganini.srv", local_IP);

  server.on("/", []() {
    server.send(200, "text/html", "<h1>Servidor local de mpaganini rodando na rede!</h1>");
  });

  server.onNotFound([]() {
    server.sendHeader("Location", "/", true);
    server.send(302, "text/plain", "");
  });

  server.begin();
  Serial.println("Servidor Web e DNS iniciados.");
}

void loop() {
  // Mantém os servidores rodando e escutando a rede (Essencial não bloquear o código)
  dnsServer.processNextRequest();
  server.handleClient();

  // Captura o tempo atual de execução do ESP32
  unsigned long tempoAtual = millis();

  // Verifica se já passaram 10 segundos (10000 ms) desde a última impressão
  if (tempoAtual - tempoAnterior >= intervalo) {
    // Salva o tempo atual para o próximo ciclo
    tempoAnterior = tempoAtual;
    Serial.println("\n--- Status de Endereçamento Atual ---");
    Serial.print("IP Local:    ");
    Serial.println(WiFi.localIP());
    
    Serial.print("Máscara:     ");
    Serial.println(WiFi.subnetMask());
    
    Serial.print("Gateway:     ");
    Serial.println(WiFi.gatewayIP());
    
    // Correção na leitura do DNS primário (Índice 0)
    Serial.print("DNS 1:       ");
    Serial.println(WiFi.dnsIP(0));

    // Correção na leitura do DNS secundário (Índice 1)
    Serial.print("DNS 2:       ");
    Serial.println(WiFi.dnsIP(1));
    Serial.println("-------------------------------------");
    
   while(1);
  }
}