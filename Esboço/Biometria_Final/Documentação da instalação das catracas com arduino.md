

# **📖 Documentação do Projeto: Controle de Catraca com Biometria**

**Versão:** 1.0  
**Equipamentos:** Arduino, Leitor Biométrico Suprema BioEntry W2, Módulo Relé 5V, Solenoide 12V.

## **1\. Visão Geral**

Este projeto tem como objetivo controlar o destravamento de uma catraca (solenoide 12V) através da validação biométrica de um leitor **Suprema BioEntry W2**. O Arduino atua como o microcontrolador central: ele detecta o acionamento do relé interno da biometria e, em seguida, aciona um módulo relé externo para fornecer energia à catraca por um período de tempo pré-determinado (5 segundos).

## 

## 

## **2\. Requisitos de Hardware**

* **1x** Placa Arduino (Uno, Nano, Mega, etc.)  
* **1x** Leitor Biométrico Suprema BioEntry W2  
* **1x** Módulo de Relé 5V (1 Canal, acionamento em nível baixo / *Active LOW*)  
* **1x** Solenoide/Eletroímã de Catraca (12V DC)  
* **1x** Fonte de Alimentação 12V DC  
* **1x** Diodo Retificador 1N4007 (Para proteção contra força contra-eletromotriz)  
* Cabos de conexão (Jumpers e fios de bitola adequada para a carga de 12V)

## 

## 

## 

## **3\. Diagramas de Ligação Eletromecânica**

### **3.1. Ligação do Leitor Biométrico (Suprema BioEntry W2)**

Este diagrama ilustra a leitura em **contato seco**, utilizando o resistor de pull-up interno do Arduino para maior segurança.

Plaintext

\=======================================================================  
//           DIAGRAMA DE LIGAÇÃO \- LEITOR BIOMÉTRICO (SUPREMA) E ARDUINO  
// \=======================================================================  
//  
//        \[ FONTE 12V \]  
//         (+)     (-)  
//          |       |  
//          |       \+-----------------------------------+  
//          |                                           |  
//          v                                           v  
//   \+-------------------------------------------------------------+  
//   |             SUPREMA BIOENTRY W2 (LEITOR BIOMÉTRICO)         |  
//   |                                                             |  
//   |  \[12V IN\]   \[GND\]               \[RELÉ COM\]      \[RELÉ NO\]   |  
//   \+--------------------------------------|---------------|------+  
//                                          |               |  
//                                          |               |  
//           O relé interno da Suprema      |               |  
//           fecha o contato quando a       v               v  
//           biometria é aceita.        \[ GND AZ \]    \[ PINO 4 BC\]  
//                                  \+-------------------------------+  
//                                  |           ARDUINO             |  
//                                  \+-------------------------------+  
//  
// \=======================================================================

### 

### 

### 

### 

### 

### 

### 

### **3.2. Ligação do Acionamento da Catraca (Módulo Relé e Solenoide)**

Este diagrama ilustra o seccionamento do cabo positivo (12V) utilizando um módulo relé de 5V controlado pelo Arduino. Note a presença obrigatória do Diodo 1N4007.

\=======================================================================  
//                      DIAGRAMA DE LIGAÇÃO \- CATRACA  
// \=======================================================================  
//  
//       \[ FONTE 12V \]  
//       (+)       (-)  
//        |         |  
//        |         |----------------------------------------------------+  
//        |                                                              |  
//        |             \+-----------------------------------------+      |  
//        |             |                                         |      |  
//      \[COM\]         \[NO\]                                       (+)    (-)  
//   \+----------------------+                                     |      |  
//   |                      |     Diodo 1N4007 (Proteção) \---\>    \+--|\>|-+  
//   |     MÓDULO RELÉ      |    (A faixinha cinza do diodo vai no positivo)  
//   |                      |                                  \+------------+  
//   \+----------------------+                                  |            |  
//     \[IN\]  \[GND\]  \[VCC\]                                      | SOLENOIDE  |  
//       |     |      |                                        | (Catraca)  |  
//       |     |      |                                        |            |  
//       |     |      |                                        \+------------+  
//      \[D5\] \[GND\]   \[5V\]  
//   \+----------------------+  
//   |       ARDUINO        |  
//   \+----------------------+  
//  
// \=======================================================================

## 

## 

## 

## **4\. Mapeamento de Pinos e Ligações**

A divisão elétrica do projeto consiste em dois circuitos isolados: o circuito lógico (5V) e o circuito de potência (12V).

### **4.1. Arduino x Leitor Suprema (Circuito Lógico)**

| Suprema BioEntry W2 | Ligação no Arduino | Função |
| :---- | :---- | :---- |
| Pino RELÉ NO (Normalmente Aberto) | **Pino Digital 4** | Envia o sinal de liberação (LOW) |
| Pino RELÉ COM (Comum) | **GND** | Fecha o circuito para o terra |

### 

### **4.2. Arduino x Módulo Relé (Controle de Potência)**

| Pino Módulo Relé | Ligação no Arduino | Função |
| :---- | :---- | :---- |
| VCC / (+) | **5V** | Alimentação lógica do módulo |
| GND / (-) | **GND** | Referência (Terra) |
| IN / Sinal | **Pino Digital 5** | Recebe o comando de acionamento |

### **4.3. Relé x Solenoide x Fonte (Circuito de Potência 12V)**

* **Negativo da Fonte (-):** Ligado diretamente ao cabo negativo do solenoide.  
* **Positivo da Fonte (+):** Conectado ao borne **COM** (Comum) do Módulo Relé.  
* **Cabo Positivo do Solenoide:** Conectado ao borne **NO** (Normalmente Aberto) do Módulo Relé.  
* **Proteção (Diodo 1N4007):** Conectado em paralelo aos fios do solenoide (Faixa cinza/cátodo no lado positivo, e o outro lado no negativo).

## 

## **5\. Lógica de Funcionamento do Software**

O código fonte opera de forma contínua no loop principal e segue o seguinte fluxo:

1. **Estado de Repouso (Aguardando):**  
   * O pino 4 (Biometria) é mantido em estado HIGH devido ao INPUT\_PULLUP.  
   * O pino 5 (Relé) emite estado HIGH, mantendo o módulo relé desligado (catraca travada).  
2. **Identificação e Validação:**  
   * Quando um usuário autorizado coloca a digital no Suprema, o leitor fecha seu relé interno.  
   * Isso conecta o pino 4 ao GND, alterando seu estado para LOW.  
3. **Acionamento da Catraca:**  
   * O Arduino detecta a mudança para LOW e envia o comando LOW ao pino 5\.  
   * O módulo relé é atracado, fechando o circuito de 12V e liberando a catraca.  
4. **Temporização e Bloqueio:**  
   * O sistema aguarda **5.000 milissegundos** (5 segundos).  
   * Após o tempo, o Arduino volta a enviar HIGH para o pino 5, cortando a energia do solenoide.  
   * O sistema aguarda mais **1.000 milissegundos** adicionais (Cooldown) para evitar leituras duplas ou falhas caso o usuário segure a catraca.

## 

## 

## 

## **6\. Avisos Críticos e Boas Práticas**

* **Proteção do Microcontrolador:** É **estritamente obrigatório** o uso do Diodo Retificador (1N4007) em paralelo à bobina do solenoide de 12V. Solenoides geram picos de tensão reversa ("Flyback") ao serem desligados, o que causa interferência eletromagnética (EMI), travamentos no Arduino e queima prematura dos contatos do módulo relé.  
* **Alimentação Isolada:** Nunca conecte a linha de 12V da fonte diretamente a nenhum pino de entrada ou saída do Arduino, sob risco de queima imediata da placa.  
* **Estado Seguro (Fail-Safe):** Caso o Arduino perca energia, o módulo relé desliga automaticamente e a catraca permanecerá **travada**, mantendo a segurança do local.