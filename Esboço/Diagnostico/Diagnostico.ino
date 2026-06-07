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

// DECLARAÇÃO ANTECIPADA (Sempre DEPOIS das bibliotecas)
void descobrirMAC(IPAddress ipAlvo, bool silencioso = false);
void descobrirMAC(IPAddress ipAlvo, bool silencioso);
void limparTela();



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

// ====================================================================
// INÍCIO DA FUNÇÃO: limparTela
// DESCRIÇÃO: Envia códigos ANSI para limpar o terminal e voltar ao topo.
// ====================================================================
void limparTela() {
    // \x1B[2J limpa a tela inteira
    // \x1B[H move o cursor para a linha 1, coluna 1 (Home)
    printOut("\x1B[2J\x1B[H"); 
}
// ====================================================================
// FIM DA FUNÇÃO: limparTela
// ====================================================================


// ====================================================================
// INÍCIO DA FUNÇÃO: printOut
// DESCRIÇÃO: Imprime texto sem quebra de linha simultaneamente na 
// porta Serial (USB) e no Bluetooth.
// ====================================================================
void printOut(String msg) {
    Serial.print(msg);
    SerialBT.print(msg);
}
// ====================================================================
// FIM DA FUNÇÃO: printOut
// ====================================================================


// ====================================================================
// INÍCIO DA FUNÇÃO: printlnOut
// DESCRIÇÃO: Imprime texto com quebra de linha simultaneamente na 
// porta Serial (USB) e no Bluetooth.
// ====================================================================
void printlnOut(String msg) {
    Serial.println(msg);
    SerialBT.println(msg);
}
// ====================================================================
// FIM DA FUNÇÃO: printlnOut
// ====================================================================

// ====================================================================
// INÍCIO DA FUNÇÃO: mudarDiretorio
// DESCRIÇÃO: Simula o comando CD (Change Directory) atualizando o
// caminho visual do prompt. Aceita navegação relativa (..) e absoluta.
// ====================================================================
void mudarDiretorio(String destino) {
    // Se digitar apenas "cd", mostra a pasta atual (comportamento do DOS)
    if (destino == "") {
        printlnOut(diretorioAtual);
        return;
    }

    // Voltar para a raiz (cd \ ou cd /)
    if (destino == "\\" || destino == "/" || destino == "c:" || destino == "c:\\") {
        diretorioAtual = "C:\\";
        return;
    }

    // Voltar um nível (cd ..)
    if (destino == "..") {
        if (diretorioAtual == "C:\\" || diretorioAtual == "C:") return; // Já está na raiz
        
        int ultimaBarra = diretorioAtual.lastIndexOf('\\');
        // Preserva o C:\ base
        if (ultimaBarra > 2) { 
            diretorioAtual = diretorioAtual.substring(0, ultimaBarra);
        } else {
            diretorioAtual = "C:\\";
        }
        return;
    }

    // Entrar em uma subpasta (Ex: cd logs)
    destino.replace("/", "\\"); // Padroniza as barras para o padrão Windows
    
    if (destino.startsWith("\\")) {
        destino = destino.substring(1);
    }
    
    if (diretorioAtual.endsWith("\\")) {
        diretorioAtual += destino;
    } else {
        diretorioAtual += "\\" + destino;
    }
}
// ====================================================================
// FIM DA FUNÇÃO: mudarDiretorio
// ====================================================================




// ====================================================================
// INÍCIO DA FUNÇÃO: imprimirPrompt
// DESCRIÇÃO: Imprime o prompt visual simulando o CMD do Windows para
// manter a imersão e indicar que o sistema aguarda um comando.
// ====================================================================
void imprimirPrompt() {
printOut("\r\n" + diretorioAtual + "> ");
}
// ====================================================================
// FIM DA FUNÇÃO: imprimirPrompt
// ====================================================================


// ====================================================================
// INÍCIO DA FUNÇÃO: registrarLogNaRede
// DESCRIÇÃO: Envia os logs formatados para um servidor HTTP externo 
// via método POST.
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
// ====================================================================
// FIM DA FUNÇÃO: registrarLogNaRede
// ====================================================================


// ====================================================================
// INÍCIO DA FUNÇÃO: mostrarInfoDisco
// DESCRIÇÃO: Simula o comando CHKDSK, exibindo o espaço total, usado
// e livre do sistema de arquivos interno LittleFS (Memória Flash).
// ====================================================================
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
// ====================================================================
// FIM DA FUNÇÃO: mostrarInfoDisco
// ====================================================================


// ====================================================================
// INÍCIO DA FUNÇÃO: gravarLog
// DESCRIÇÃO: Abre o arquivo local no LittleFS em modo "Append" (adição)
// e salva o comando executado e seu resultado para histórico.
// ====================================================================
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
// ====================================================================
// FIM DA FUNÇÃO: gravarLog
// ====================================================================


// ====================================================================
// INÍCIO DA FUNÇÃO: lerLogs
// DESCRIÇÃO: Simula o comando TYPE, abrindo o arquivo de log no modo 
// leitura e imprimindo todo o seu conteúdo no terminal.
// ====================================================================
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
// ====================================================================
// FIM DA FUNÇÃO: lerLogs
// ====================================================================


// ====================================================================
// INÍCIO DA FUNÇÃO: apagarLogs
// DESCRIÇÃO: Simula o comando DEL, removendo fisicamente o arquivo 
// de log da memória Flash para liberar espaço.
// ====================================================================
void apagarLogs() {
    if (LittleFS.remove("/rede.log")) {
        printlnOut("Arquivo apagado com sucesso. Espaco liberado.");
    } else {
        printlnOut("Nao foi possivel encontrar C:\\ESP32\\Rede\\rede.log");
    }
}
// ====================================================================
// FIM DA FUNÇÃO: apagarLogs
// ====================================================================


// ====================================================================
// INÍCIO DA FUNÇÃO: configurarCartaoSD
// DESCRIÇÃO: Inicializa a comunicação SPI com o módulo de Cartão SD.
// (Preparada para implementações futuras de hardware).
// ====================================================================
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
// ====================================================================
// FIM DA FUNÇÃO: configurarCartaoSD
// ====================================================================


// ====================================================================
// INÍCIO DA FUNÇÃO: executarPing
// DESCRIÇÃO: Dispara pacotes ICMP para um IP. Exibe o IP alvo 
// claramente e grava o resultado no arquivo de log.
// ====================================================================
void executarPing(String ipStr) {
  IPAddress ipAlvo;
  if (ipAlvo.fromString(ipStr)) {
    // Exibição mais fiel ao CMD do Windows
    printlnOut("\nDisparando " + ipStr + " com 32 bytes de dados:");
    
    if (Ping.ping(ipAlvo, 4)) { 
      // Calculando tempo médio
      String tempo = String(Ping.averageTime());
      String mensagemSucesso = "Resposta de " + ipStr + ": tempo=" + tempo + "ms";
      printlnOut(mensagemSucesso);
      
      // Grava no log
      gravarLog("ping " + ipStr, mensagemSucesso); 
    } else {
      String mensagemFalha = "Esgotado o tempo limite do pedido para " + ipStr;
      printlnOut(mensagemFalha);
      
      // Grava falha no log
      gravarLog("ping " + ipStr, mensagemFalha);
    }
  } else {
    printlnOut("\nA solicitacao ping nao pode encontrar o host " + ipStr + ". Verifique o nome e tente novamente.");
  }
}
// ====================================================================
// FIM DA FUNÇÃO: executarPing
// ====================================================================

// ====================================================================
// INÍCIO DA FUNÇÃO: arpNet
// DESCRIÇÃO: Realiza um ARP Sweep em uma sub-rede personalizada.
// Exemplo de uso: arpnet 192.168.10
// ====================================================================
void arpNet(String redeStr) {
    // Validação simples: espera algo no formato XXX.XXX.XXX
    // (Isso é uma simplificação para o parser)
    printlnOut("\n[ARP SWEEP] Iniciando varredura na sub-rede: " + redeStr + ".X");
    
    for (int i = 1; i <= 254; i++) {
        // Esta é uma forma rudimentar de montar o IP, 
        // em sistemas complexos usaríamos sscanf para separar os octetos
        IPAddress ipAlvo;
        String ipCompleto = redeStr + "." + String(i);
        
        if (ipAlvo.fromString(ipCompleto)) {
            descobrirMAC(ipAlvo, true); // modo silencioso
            yield(); 
        }
    }
    printlnOut("[ARP SWEEP] Varredura concluida!\n");
}
// ====================================================================
// FIM DA FUNÇÃO: arpNet
// ====================================================================




// ====================================================================
// INÍCIO DA FUNÇÃO: mostrarHostname
// DESCRIÇÃO: Retorna o nome da placa (Hostname) registrado no 
// servidor DHCP local.
// ====================================================================
void mostrarHostname() {
    printlnOut(WiFi.getHostname());
}
// ====================================================================
// FIM DA FUNÇÃO: mostrarHostname
// ====================================================================


// ====================================================================
// INÍCIO DA FUNÇÃO: mostrarRotas
// DESCRIÇÃO: Simula o comando ROUTE PRINT exibindo uma tabela padrão 
// de rotas com base nas configurações de IP e Gateway do Wi-Fi.
// ====================================================================
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
// ====================================================================
// FIM DA FUNÇÃO: mostrarRotas
// ====================================================================


// ====================================================================
// INÍCIO DA FUNÇÃO: testarConexaoTCP
// DESCRIÇÃO: Simula o PowerShell Test-NetConnection tentando abrir um 
// socket TCP em um IP e porta específicos.
// ====================================================================
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
// ====================================================================
// FIM DA FUNÇÃO: testarConexaoTCP
// ====================================================================


// ====================================================================
// INÍCIO DA FUNÇÃO: descobrirMAC
// ====================================================================
void descobrirMAC(IPAddress ipAlvo, bool silencioso) {
  ip4_addr_t ip_lwip;
  ip_lwip.addr = static_cast<uint32_t>(ipAlvo);
  
  if (!silencioso) {
    printlnOut("\nEnviando requisicao ARP para " + ipAlvo.toString() + "...");
  }
  
  // Tranca a pilha TCP/IP (MACRO EM MAIÚSCULAS)
  LOCK_TCPIP_CORE();
  etharp_request(netif_default, &ip_lwip);
  UNLOCK_TCPIP_CORE();
  
  unsigned long tempoInicio = millis();
  bool macEncontrado = false;
  char macStr[18] = "";
  
  while (millis() - tempoInicio < 150) {
      eth_addr *mac_retornado = nullptr;
      const ip4_addr_t *ip_retornado = nullptr;
      
      // Tranca a memória para procurar e copiar o MAC com segurança
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
      if (!silencioso) {
        printlnOut("Falha na resolucao ARP.");
      }
  }
}
// ====================================================================
// FIM DA FUNÇÃO: descobrirMAC
// ====================================================================


// ====================================================================
// INÍCIO DA FUNÇÃO: arpSweep
// ====================================================================
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

  // Restaura o MAC do Gateway com segurança de Thread
  ip4_addr_t gw_lwip;
  gw_lwip.addr = static_cast<uint32_t>(gateway);
  
  LOCK_TCPIP_CORE();
  etharp_request(netif_default, &gw_lwip);
  UNLOCK_TCPIP_CORE();

  printlnOut("  ------------------------------------------------------");
  printlnOut("Varredura ARP concluida.\n");
}
// ====================================================================
// FIM DA FUNÇÃO: arpSweep
// ====================================================================


// ====================================================================
// INÍCIO DA FUNÇÃO: mostrarHelp
// DESCRIÇÃO: Imprime o menu de ajuda com a lista de todos os 
// comandos disponíveis no sistema.
// ====================================================================
void mostrarHelp() {
  printlnOut("===============================================================================");
  printlnOut("\Digite HELP.");
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
// ====================================================================
// FIM DA FUNÇÃO: mostrarHelp
// ====================================================================


// ====================================================================
// INÍCIO DA FUNÇÃO: setup
// DESCRIÇÃO: Inicializa barramentos, monta o disco interno (LittleFS),
// estabelece a comunicação serial/Bluetooth e gerencia a conexão Wi-Fi.
// ====================================================================


// ====================================================================
// INÍCIO DA FUNÇÃO: listarDiretorio
// DESCRIÇÃO: Simula o comando DIR do MS-DOS listando os arquivos
// contidos na memória Flash (LittleFS), seus tamanhos e o espaço livre.
// ====================================================================
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

    // Imprime pastas padrão do sistema
    printlnOut("    <DIR>          .");
    printlnOut("    <DIR>          ..");
    totalPastas += 2;

    // Varre todos os arquivos do disco
    File file = root.openNextFile();
    while (file) {
        if (file.isDirectory()) {
            printlnOut("    <DIR>          " + String(file.name()));
            totalPastas++;
        } else {
            String tamanho = String(file.size());
            
            // Calcula espaços para alinhar a coluna de bytes à direita
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
// FIM DA FUNÇÃO: listarDiretorio
// ====================================================================

// ====================================================================
// INÍCIO DA FUNÇÃO: exibirIpconfig
// DESCRIÇÃO: Simula o comando IPCONFIG, exibindo as configurações 
// de rede atuais do adaptador Wi-Fi do ESP32.
// ====================================================================
void exibirIpconfig() {
    printlnOut("\nConfiguracao de IP do Windows\n");
    printlnOut("Adaptador de rede sem fio Wi-Fi:");
    printlnOut("   Sufixo DNS específico de conexão . : local");
    printlnOut("   Endereço IPv4. . . . . . . . . . . : " + WiFi.localIP().toString());
    printlnOut("   Máscara de Sub-rede. . . . . . . . : " + WiFi.subnetMask().toString());
    printlnOut("   Gateway Padrão. . . . . . . . . . : " + WiFi.gatewayIP().toString());
    printlnOut("   Servidor DNS. . . . . . . . . . . : " + WiFi.dnsIP().toString() + "\n");
}
// ====================================================================
// FIM DA FUNÇÃO: exibirIpconfig
// ====================================================================




// ====================================================================
// INICIO DA FUNÇÃO SET
// ====================================================================
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
// FIM DA FUNÇÃO SET
// ====================================================================


void setup() {
  Serial.begin(115200);
  //Serial.setTimeout(3000); // Dá 60 segundos para você terminar de digitar
  SerialBT.begin("ESP32_BT_46");

  if (!LittleFS.begin(true)) {
      printlnOut("Falha ao montar o sistema de arquivos interno (LittleFS).");
  }

  printlnOut("\nMarco Paganini- Diagnostico de rede [versao ESP32.Rede.1.0]");
  printlnOut("(c) Marco Paganini Todos os direitos reservados.\n");
  printlnOut("Pressione ENTER para configurar o Wi-Fi...");
  
  while (!Serial.available() && !SerialBT.available()) {
    delay(100); 
  }

  if (Serial.available()) { Serial.readString(); } 
  else if (SerialBT.available()) { SerialBT.readString(); }

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

  printlnOut("\nDigite o NUMERO da rede:");
  while (redeSelecionada == -1) {
    String entrada = "";
    if (Serial.available()) { entrada = Serial.readStringUntil('\n'); } 
    else if (SerialBT.available()) { entrada = SerialBT.readStringUntil('\n'); }

    if (entrada.length() > 0) {
      entrada.trim();
      int opcao = entrada.toInt(); 
      if (opcao > 0 && opcao <= totalRedes) { redeSelecionada = opcao - 1; } 
    }
  }

  ssidStr = WiFi.SSID(redeSelecionada);

  if (WiFi.encryptionType(redeSelecionada) != WIFI_AUTH_OPEN) {
    printlnOut("Senha da rede:");
    while (passStr.length() == 0) {
      if (Serial.available()) { passStr = Serial.readStringUntil('\n'); } 
      else if (SerialBT.available()) { passStr = SerialBT.readStringUntil('\n'); }
      if (passStr.length() > 0) { passStr.trim(); }
    }
  }

  printlnOut("\nAutenticando...");
  WiFi.begin(ssidStr.c_str(), passStr.c_str());

  int tentativas = 0;
  while (WiFi.status() != WL_CONNECTED && tentativas < 20) { 
    delay(500);
    tentativas++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    WiFi.scanDelete();
    imprimirPrompt();
  } else {
    printlnOut("\nFalha ao conectar.");
  }
}

// ====================================================================
// FIM DA FUNÇÃO: setup
// ====================================================================



// ====================================================================
// INÍCIO DA FUNÇÃO: loop
// DESCRIÇÃO: Monitora ativamente as portas seriais usando um Buffer
// Acumulador sem bloqueio (Non-Blocking). Permite digitação lenta
// e uso da tecla Backspace.
// ====================================================================
void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    
    // Esta variável guarda as letras (Buffer) e não reseta a cada ciclo
    static String bufferEntrada = ""; 
    
    while (Serial.available() || SerialBT.available()) {
      char c;
      if (Serial.available()) { c = Serial.read(); }
      else { c = SerialBT.read(); }

      // 1. Trata a tecla Backspace para poder apagar erros
      if (c == '\b' || c == 127) {
        if (bufferEntrada.length() > 0) {
          bufferEntrada.remove(bufferEntrada.length() - 1);
          printOut("\b \b"); // Apaga a letra visualmente na tela
        }
        continue;
      }

      // 2. Trata a tecla Enter (Sinal de Executar Comando)
      if (c == '\r' || c == '\n') {
        
        // Ignora a quebra de linha dupla que terminais enviam (\r\n)
        if (c == '\n' && bufferEntrada.length() == 0) continue;
        
        printlnOut(""); // Pula de linha no terminal após dar Enter

        if (bufferEntrada.length() > 0) {
          String entradaBruta = bufferEntrada;
          bufferEntrada = ""; // Esvazia a memória para o próximo comando
          
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
          
          // ==========================================================
          // ÁRVORE DE DECISÃO DOS COMANDOS
          // ==========================================================
          if (comando == "help") {
            mostrarHelp();
          } else if (comando == "ipconfig") {
            exibirIpconfig();
          } else if (comando == "cd") {
            mudarDiretorio(argumento);
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
            
          // ---> GETMAC DE VOLTA AQUI <---
          } else if (comando == "getmac") {
            IPAddress ipBusca;
            if (ipBusca.fromString(argumento)) {
              descobrirMAC(ipBusca, false);
            } else {
              printlnOut("Sintaxe: getmac <ip_destino> (Ex: getmac 192.168.0.15)");
            }
          // ------------------------------
          
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
        // Imprime o prompt novamente, pronto para receber algo
        imprimirPrompt();
        
      } else {
        // 3. Acumula a digitação pacientemente e exibe (Eco) na tela
        bufferEntrada += c;
        printOut(String(c)); 
      }
    }
  } else {
    printlnOut("Aviso: Conexao Wi-Fi perdida.");
    delay(5000); 
  }
}
// ====================================================================
// FIM DA FUNÇÃO: loop
// ====================================================================