#include <WiFi.h>
#include <WiFiUdp.h>
#include <ESP32Ping.h>
#include <Arduino_SNMP_Manager.h> 
#include <lwip/etharp.h> 
#include "BluetoothSerial.h"
#include <lwip/sockets.h>
#include <lwip/netdb.h>
#include <lwip/icmp.h>
#include <SD.h>
#include <HTTPClient.h>
#include <LittleFS.h>
#include "lwip/tcpip.h"
#include <WebServer.h>

// DECLARAÇÃO ANTECIPADA (Sempre DEPOIS das bibliotecas)
void descobrirMAC(IPAddress ipAlvo, bool silencioso = false);
void limparTela();
void printOut(String msg);
void printlnOut(String msg);



// Inicia o servidor web na porta 80
WebServer server(80);

// Função executada quando o diretório raiz ("/") for acessado
void handleRoot() {
  long rssi = WiFi.RSSI();
  String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1'><title>Status ESP32</title>";
  html += "<style>body{font-family:Arial,sans-serif; margin:20px; background-color:#f4f4f4;} .card{background:#fff; border:1px solid #ccc; padding:20px; border-radius:8px; max-width:400px;}</style></head>";
  html += "<body><div class='card'><h2>Dados do ESP32</h2>";
  html += "<hr>";
  html += "<p><b>SSID:</b> " + WiFi.SSID() + "</p>";
  html += "<p><b>IP Local:</b> " + WiFi.localIP().toString() + "</p>";
  html += "<p><b>Máscara:</b> " + WiFi.subnetMask().toString() + "</p>";
  html += "<p><b>Gateway:</b> " + WiFi.gatewayIP().toString() + "</p>";
  html += "<p><b>Sinal (RSSI):</b> " + String(rssi) + " dBm</p>";
  html += "<p><b>MAC do ESP32:</b> " + WiFi.macAddress() + "</p>";
  html += "<hr><p>Terminal de Diagnóstico Operante.</p></div></body></html>";
  
  server.send(300, "text/html", html);
}

int redeSelecionada = -1;
String ssidStr = "";
String passStr = "";
String diretorioAtual = "C:\\ESP32\\Rede";
String filtroHostname = "";
String filtroSerie = "";

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error O Bluetooth não está habilitado na sua placa!
#endif

BluetoothSerial SerialBT;

const char* const HTML_PARAGRAPH_START = "<p><b>";
const char* const HTML_PARAGRAPH_END = "</p>";
const char* const HTML_BOLD_END = "</b>";
const char* const HTML_LABEL_GETMAC = "GetMac:"; // Rótulo para o endereço IP
const char* const IP_NOT_CONNECTED_MSG = "N/A (Não Conectado)";
const char* const IP_INVALID_MSG = "N/A (IP Inválido)";
const size_t MAX_IP_STRING_LEN = 16;

// ====================================================================
// FUNÇÕES DE EXIBIÇÃO
// ====================================================================
void limparTela() {
    printOut("\x1B[2J\x1B[H"); 
}

void printOut(String msg) {
    Serial.print(msg);
    SerialBT.print(msg);
}

void printlnOut(String msg) {
    Serial.println(msg);
    SerialBT.println(msg);
}

void mudarDiretorio(String destino) {
    if (destino == "") {
        printlnOut(diretorioAtual);
        return;
    }
    if (destino == "\\" || destino == "/" || destino == "c:" || destino == "c:\\") {
        diretorioAtual = "C:\\";
        return;
    }
    if (destino == "..") {
        if (diretorioAtual == "C:\\" || diretorioAtual == "C:") return; 
        
        int ultimaBarra = diretorioAtual.lastIndexOf('\\');
        if (ultimaBarra > 2) { 
            diretorioAtual = diretorioAtual.substring(0, ultimaBarra);
        } else {
            diretorioAtual = "C:\\";
        }
        return;
    }
    destino.replace("/", "\\"); 
    if (destino.startsWith("\\")) {
        destino = destino.substring(1);
    }
    if (diretorioAtual.endsWith("\\")) {
        diretorioAtual += destino;
    } else {
        diretorioAtual += "\\" + destino;
    }
}

void imprimirPrompt() {
    printOut("\r\n" + diretorioAtual + "> ");
}

// ====================================================================
// FUNÇÕES DE LOG E DISCO
// ====================================================================
const char* urlServidorLog = "http://192.168.0.80:5000/log/esp"; 

void registrarLogNaRede(String comando, String resultado) {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin(urlServidorLog);
        http.addHeader("Content-Type", "text/plain");
        String payload = "Comando: " + comando + "\nResultado:\n" + resultado + "\n-------------------";
        int httpResponseCode = http.POST(payload);
        if (httpResponseCode > 0) {
            printlnOut("[LOG] Salvo no diretorio da rede com sucesso.");
        } else {
            printlnOut("[LOG] Erro ao salvar no diretorio: " + String(httpResponseCode));
        }
        http.end();
    }
}

void mostrarInfoDisco() {
    printlnOut("\nO volume na unidade C e ESP32_FLASH");
    printlnOut("O Numero de Serie do Volume e 1A2B-3C4D\n");
    size_t total = LittleFS.totalBytes();
    size_t usado = LittleFS.usedBytes();
    size_t livre = total - usado;
    printlnOut("    " + String(total) + " bytes de espaco total em disco.");
    printlnOut("    " + String(usado) + " bytes usados.");
    printlnOut("    " + String(livre) + " bytes disponiveis no disco.\n");
}

void gravarLog(String comando, String resultado) {
    File file = LittleFS.open("/rede.log", "a");
    if (file) {
        file.println("CMD: " + comando);
        file.println(resultado);
        file.println("----------------------------------------");
        file.close();
    } else {
        printlnOut("[ERRO] Falha ao abrir o sistema de arquivos.");
    }
}

void lerLogs() {
    File file = LittleFS.open("/rede.log", "r");
    if (!file || file.isDirectory()) {
        printlnOut("O sistema nao pode encontrar o arquivo especificado (C:\\ESP32\\Rede\\rede.log).");
        return;
    }
    printlnOut("\n--- INICIO DO ARQUIVO DE LOG ---");
    while (file.available()) {
        printOut(file.readStringUntil('\n') + "\n");
    }
    printlnOut("--- FIM DO ARQUIVO DE LOG ---\n");
    file.close();
}

void apagarLogs() {
    if (LittleFS.remove("/rede.log")) {
        printlnOut("Arquivo apagado com sucesso. Espaco liberado.");
    } else {
        printlnOut("Nao foi possivel encontrar C:\\ESP32\\Rede\\rede.log");
    }
}

void configurarCartaoSD() {
    if (!SD.begin(5)) {
        printlnOut("Falha ao montar o Cartao SD");
        return;
    }
    if (!SD.exists("/Logs_de_Rede")) {
        SD.mkdir("/Logs_de_Rede");
        printlnOut("Diretorio '/Logs_de_Rede' criado com sucesso no SD.");
    }
}

void listarDiretorio() {
    printlnOut("\n O volume na unidade C e ESP32_FLASH");
    printlnOut(" O Numero de Serie do Volume e 1A2B-3C4D\n");
    printlnOut(" Pasta de C:\\ESP32\\Rede\n");

    File root = LittleFS.open("/");
    if (!root || !root.isDirectory()) {
        printlnOut("Falha ao ler a unidade C:");
        return;
    }

    int totalArquivos = 0;
    int totalPastas = 0;
    size_t tamanhoTotal = 0;

    printlnOut("    <DIR>          .");
    printlnOut("    <DIR>          ..");
    totalPastas += 2;

    File file = root.openNextFile();
    while (file) {
        if (file.isDirectory()) {
            printlnOut("    <DIR>          " + String(file.name()));
            totalPastas++;
        } else {
            String tamanho = String(file.size());
            int espacos = 18 - tamanho.length();
            String espacamento = "";
            for(int i = 0; i < espacos; i++) espacamento += " ";
            printlnOut(espacamento + tamanho + " " + String(file.name()));
            tamanhoTotal += file.size();
            totalArquivos++;
        }
        file = root.openNextFile();
    }
    size_t discoLivre = LittleFS.totalBytes() - LittleFS.usedBytes();
    printlnOut("               " + String(totalArquivos) + " arquivo(s)      " + String(tamanhoTotal) + " bytes");
    printlnOut("               " + String(totalPastas) + " pasta(s)        " + String(discoLivre) + " bytes livres\n");
}

// ====================================================================
// FUNÇÕES DE REDE
// ====================================================================
void executarPing(String ipStr) {
  IPAddress ipAlvo;
  if (ipAlvo.fromString(ipStr)) {
    printlnOut("\nDisparando " + ipStr + " com 32 bytes de dados:");
    if (Ping.ping(ipAlvo, 4)) { 
      String tempo = String(Ping.averageTime());
      String mensagemSucesso = "Resposta de " + ipStr + ": tempo=" + tempo + "ms";
      printlnOut(mensagemSucesso);
      gravarLog("ping " + ipStr, mensagemSucesso); 
    } else {
      String mensagemFalha = "Esgotado o tempo limite do pedido para " + ipStr;
      printlnOut(mensagemFalha);
      gravarLog("ping " + ipStr, mensagemFalha);
    }
  } else {
    printlnOut("\nA solicitacao ping nao pode encontrar o host " + ipStr + ". Verifique o nome e tente novamente.");
  }
}

void arpNet(String redeStr) {
    printlnOut("\n[ARP SWEEP] Iniciando varredura na sub-rede: " + redeStr + ".X");
    for (int i = 1; i <= 254; i++) {
        IPAddress ipAlvo;
        String ipCompleto = redeStr + "." + String(i);
        if (ipAlvo.fromString(ipCompleto)) {
            descobrirMAC(ipAlvo, true); 
            yield(); 
        }
    }
    printlnOut("[ARP SWEEP] Varredura concluida!\n");
}

void mostrarHostname() {
    printlnOut(WiFi.getHostname());
}

void mostrarRotas() {
    printlnOut("\n===========================================================================");
    printlnOut("Rotas Ativas:");
    printlnOut("Destino de Rede        Mascara de Rede          Gateway       Interface");
    printOut("          0.0.0.0          0.0.0.0     ");
    printOut(WiFi.gatewayIP().toString());
    printOut("     ");
    printlnOut(WiFi.localIP().toString());
    printOut("     ");
    printOut(WiFi.localIP().toString());
    printOut("    ");
    printOut(WiFi.subnetMask().toString());
    printlnOut("         No-link       127.0.0.1");
    printlnOut("===========================================================================");
}

void testarConexaoTCP(String ipStr, String portaStr) {
    IPAddress ipAlvo;
    int porta = portaStr.toInt();
    if (porta == 0) {
        printlnOut("Porta invalida.");
        return;
    }
    if (ipAlvo.fromString(ipStr)) {
        printlnOut("\nTestando conexao TCP com " + ipStr + " na porta " + String(porta) + "...");
        WiFiClient client;
        client.setTimeout(3); 
        uint32_t tempoInicio = millis();
        bool conectado = client.connect(ipAlvo, porta);
        uint32_t tempoFim = millis() - tempoInicio;
        if (conectado) {
            printlnOut("TcpTestSucceeded : True (Tempo: " + String(tempoFim) + "ms)");
            client.stop();
        } else {
            printlnOut("TcpTestSucceeded : False (Timeout ou Recusado)");
        }
    } else {
        printlnOut("IP invalido.");
    }
}

void descobrirMAC(IPAddress ipAlvo, bool silencioso) {
  ip4_addr_t ip_lwip;
  ip_lwip.addr = static_cast<uint32_t>(ipAlvo);
  
  if (!silencioso) {
    printlnOut("\nEnviando requisicao ARP para " + ipAlvo.toString() + "...");
  }
  
  LOCK_TCPIP_CORE();
  etharp_request(netif_default, &ip_lwip);
  UNLOCK_TCPIP_CORE();
  
  unsigned long tempoInicio = millis();
  bool macEncontrado = false;
  char macStr[18] = "";
  
  while (millis() - tempoInicio < 150) {
      eth_addr *mac_retornado = nullptr;
      const ip4_addr_t *ip_retornado = nullptr;
      
      LOCK_TCPIP_CORE();
      if (etharp_find_addr(netif_default, &ip_lwip, &mac_retornado, &ip_retornado) != -1 && mac_retornado != nullptr) {
          sprintf(macStr, "%02X-%02X-%02X-%02X-%02X-%02X", 
                  mac_retornado->addr[0], mac_retornado->addr[1], mac_retornado->addr[2], 
                  mac_retornado->addr[3], mac_retornado->addr[4], mac_retornado->addr[5]);
          macEncontrado = true;
      }
      UNLOCK_TCPIP_CORE();
      if (macEncontrado) break; 
      delay(5); 
  }
  
  if (macEncontrado) {
      printOut("  " + ipAlvo.toString());
      int espacos = 22 - ipAlvo.toString().length();
      for(int s = 0; s < espacos; s++) printOut(" ");
      printlnOut(String(macStr) + "      Dinamico");
      gravarLog(ipAlvo.toString(), "MAC Encontrado: " + String(macStr));
  } else {
      if (!silencioso) printlnOut("Falha na resolucao ARP.");
  }
}

void arpSweep() {
  IPAddress meuIP = WiFi.localIP();
  IPAddress gateway = WiFi.gatewayIP();
  
  printlnOut("\nInterface: " + meuIP.toString() + " --- 0x1");
  printlnOut("  Endereco IP           Endereco Fisico       Tipo");
  printlnOut("  ------------------------------------------------------");
  
  for (int i = 1; i <= 254; i++) {
    IPAddress ipAlvo(meuIP[0], meuIP[1], meuIP[2], i);
    if (ipAlvo == meuIP) continue; 
    descobrirMAC(ipAlvo, true); 
    delay(10); 
    yield(); 
  }

  ip4_addr_t gw_lwip;
  gw_lwip.addr = static_cast<uint32_t>(gateway);
  LOCK_TCPIP_CORE();
  etharp_request(netif_default, &gw_lwip);
  UNLOCK_TCPIP_CORE();

  printlnOut("  ------------------------------------------------------");
  printlnOut("Varredura ARP concluida.\n");
}

void exibirIpconfig() {
    printlnOut("\nConfiguracao de IP do Windows\n");
    printlnOut("Adaptador de rede sem fio Wi-Fi:");
    printlnOut("   Sufixo DNS específico de conexão . : local");
    printlnOut("   Endereço IPv4. . . . . . . . . . . : " + WiFi.localIP().toString());
    printlnOut("   Máscara de Sub-rede. . . . . . . . : " + WiFi.subnetMask().toString());
    printlnOut("   Gateway Padrão. . . . . . . . . . : " + WiFi.gatewayIP().toString());
    printlnOut("   Servidor DNS. . . . . . . . . . . : " + WiFi.dnsIP().toString() + "\n");
}

void mostrarHelp() {
  printlnOut("===============================================================================");
  printlnOut("Digite HELP."); // Corrigido o erro do \ que impedia compilar
  printlnOut("GETMAC         Para MAC individual getmac 192.68.1.1");
  printlnOut("HELP           Fornece informacoes de Ajuda para os comandos da rede.");
  printlnOut("SCANARP        Exibe e modifica as tabelas de conversao de enderecos IP em MAC.");
  printlnOut("PING           Verifica a conectividade no nivel IP para outro computador.");
  printlnOut("HOSTNAME       Exibe o nome do host da maquina conectada.");
  printlnOut("ROUTE PRINT    Manipula tabelas de roteamento da rede.");
  printlnOut("TESTNET        Testa portas TCP de um endereco IP. Ex: testnet 192.168.1.1 80");
  printlnOut("CHKDSK         Verifica o disco e exibe um relatorio de status.");
  printlnOut("TYPE LOGS      Exibe o conteudo do arquivo de logs.");
  printlnOut("DEL LOGS       Exclui o arquivo de logs.");
  printlnOut("===============================================================================");
}

void executarSet(String argumento) {
    if (argumento == "") {
        printlnOut("Filtros atuais:");
        printlnOut("  HOSTNAME=" + (filtroHostname == "" ? "NENHUM" : filtroHostname));
        printlnOut("  SERIE=" + (filtroSerie == "" ? "NENHUM" : filtroSerie));
        return;
    }
    if (argumento.startsWith("hostname=")) {
        filtroHostname = argumento.substring(9);
        printlnOut("Filtro Hostname definido para: " + filtroHostname);
    } else if (argumento.startsWith("serie=")) {
        filtroSerie = argumento.substring(6);
        printlnOut("Filtro Serie definido para: " + filtroSerie);
    } else {
        printlnOut("Uso: set [hostname=valor | serie=valor]");
    }
}

// ====================================================================
// SETUP PRINCIPAL
// ====================================================================
void setup() {
  Serial.begin(115200);
  SerialBT.begin("ESP32_BT_46");

  if (!LittleFS.begin(true)) {
      printlnOut("Falha ao montar o sistema de arquivos interno (LittleFS).");
  }

  printlnOut("\nMarco Paganini- Diagnostico de rede [versao ESP32.Rede.1.0]");
  printlnOut("(c) Marco Paganini Todos os direitos reservados.\n");
  
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  int totalRedes = WiFi.scanNetworks();
  if (totalRedes == 0) {
    printlnOut("Nenhuma rede encontrada. Reinicie o ESP32.");
    while (true);
  } 

  for (int i = 0; i < totalRedes; ++i) {
    printOut("[");
    printOut(String(i + 1));
    printOut("] ");
    printlnOut(WiFi.SSID(i));
  }

  // >>> BLOCO DE CÓDIGO ORIGINAL DO SEU SCANNER <<<
  Serial.println("\nDigite o NUMERO da rede que deseja conectar e pressione ENTER:");
  while (redeSelecionada == -1) {
    if (Serial.available()) {
      String entrada = Serial.readStringUntil('\n');
      entrada.trim();
      int opcao = entrada.toInt(); // Converte texto para número inteiro

      if (opcao > 0 && opcao <= totalRedes) {
        redeSelecionada = opcao - 1; // Subtrai 1 porque o índice da matriz (array) começa em 0
      } else {
        Serial.println("Numero invalido. Tente novamente:");
      }
    }
  }

  ssidStr = WiFi.SSID(redeSelecionada);
  Serial.print("Rede selecionada: ");
  Serial.println(ssidStr);

  // Verifica se tem senha e pede ao usuário
  if (WiFi.encryptionType(redeSelecionada) != WIFI_AUTH_OPEN) {
    Serial.println("Rede protegida. Digite a PASSWORD e pressione ENTER:");
    while (passStr.length() == 0) {
      if (Serial.available()) {
        passStr = Serial.readStringUntil('\n');
        passStr.trim();
      }
    }
  } else {
    Serial.println("Rede Aberta. Pulando etapa de senha.");
  }

  // Inicia a conexão
  Serial.println("\nConectando...");
  WiFi.begin(ssidStr.c_str(), passStr.c_str());

  int tentativas = 0;
  while (WiFi.status() != WL_CONNECTED && tentativas < 20) { 
    delay(500);
    Serial.print(".");
    tentativas++;
  }
  // >>> FIM DO BLOCO ORIGINAL <<<

  if (WiFi.status() == WL_CONNECTED) {
    WiFi.scanDelete();
    
    Serial.println("\n\nConectado com sucesso!");
    Serial.print("Endereço IP para acesso Web: ");
    Serial.println(WiFi.localIP()); 
    
    // Inicia o servidor Web na raiz
    server.on("/", handleRoot);
    server.begin();
    Serial.println("Servidor HTTP iniciado");
    
    imprimirPrompt();
  } else {
    printlnOut("\nFalha ao conectar.");
  }
}

// ====================================================================
// LOOP PRINCIPAL
// ====================================================================
void loop() {
  // Mantém o servidor web escutando requisições constantemente
  server.handleClient();

  if (WiFi.status() == WL_CONNECTED) {
    
    static String bufferEntrada = ""; 
    
    while (Serial.available() || SerialBT.available()) {
      char c;
      if (Serial.available()) { c = Serial.read(); }
      else { c = SerialBT.read(); }

      if (c == '\b' || c == 127) {
        if (bufferEntrada.length() > 0) {
          bufferEntrada.remove(bufferEntrada.length() - 1);
          printOut("\b \b"); 
        }
        continue;
      }

      if (c == '\r' || c == '\n') {
        if (c == '\n' && bufferEntrada.length() == 0) continue;
        
        printlnOut(""); 

        if (bufferEntrada.length() > 0) {
          String entradaBruta = bufferEntrada;
          bufferEntrada = ""; 
          
          entradaBruta.trim(); 
          entradaBruta.replace('\t', ' '); 
          
          while(entradaBruta.indexOf("  ") >= 0) {
            entradaBruta.replace("  ", " ");
          }

          String comando = entradaBruta;
          String argumento = "";
          String subArgumento = ""; 
          
          int espacoIndex = entradaBruta.indexOf(" ");
          if (espacoIndex != -1) {
            comando = entradaBruta.substring(0, espacoIndex);
            argumento = entradaBruta.substring(espacoIndex + 1);
            
            int espaco2Index = argumento.indexOf(" ");
            if (espaco2Index != -1) {
              subArgumento = argumento.substring(espaco2Index + 1);
              argumento = argumento.substring(0, espaco2Index);
            }
          }
          
          comando.toLowerCase();
          comando.trim();  
          argumento.toLowerCase(); 
          argumento.trim(); 
          subArgumento.toLowerCase();
          subArgumento.trim();

          IPAddress ipAlvo;
          
          if (comando == "help") {
            mostrarHelp();
          } else if (comando == "ipconfig") {
            exibirIpconfig();
          } else if (comando == "cd") {
            mudarDiretorio(argumento);
          } else if (comando == "dir" || comando == "ls") { 
            listarDiretorio();
          } else if (comando == "scanarp" || comando == "arp") { 
            arpSweep();
          } else if (comando == "hostname") {
            mostrarHostname();
          } else if (comando == "route") {
            mostrarRotas();
          } else if (comando == "ping") {
            if (argumento != "") { executarPing(argumento); } 
            else { printlnOut("Uso: ping <ip_destino>"); }
          } else if (comando == "testnet") {
            if (argumento != "" && subArgumento != "") { 
              testarConexaoTCP(argumento, subArgumento); 
            } else { 
              printlnOut("Sintaxe: testnet <ip> <porta>"); 
            }
          } else if (comando == "chkdsk") {
            mostrarInfoDisco();
          } else if (comando == "logs") {
            lerLogs();
          } else if (comando == "del") {
            apagarLogs();
          } else if (comando == "getmac") {
            IPAddress ipBusca;
            if (ipBusca.fromString(argumento)) {
              descobrirMAC(ipBusca, false);
            } else {
              printlnOut("Sintaxe: getmac <ip_destino> (Ex: getmac 192.168.0.15)");
            }
          } else if (comando == "arpnet") {
            if (argumento != "") { arpNet(argumento); } 
            else { printlnOut("Sintaxe: arpnet <prefixo_da_rede> (Ex: arpnet 192.168.10)"); }
          } else if (ipAlvo.fromString(comando)) {
            descobrirMAC(ipAlvo, false);
          } else if (comando == "cmd" || comando == "powershell") {
            // Apenas simula que não faz nada
          } else if (comando == "cls" || comando == "clear") {
            limparTela();
          } else if (comando == "set") {
            executarSet(argumento);  
          } else {
            printlnOut("'" + comando + "' nao e reconhecido como comando interno.");
          }
        }
        imprimirPrompt();
        
      } else {
        bufferEntrada += c;
        printOut(String(c)); 
      }
    }
  } else {
    printlnOut("Aviso: Conexao Wi-Fi perdida.");
    server.handleClient();
    delay(5000); 
  }
}