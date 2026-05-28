#include <Arduino.h>
#include <QTRSensors.h>

// =============================================================================
//  SEGUIDOR DE LINHA — ESP32 DevKit V1  |  Versão de Competição
// =============================================================================
//
//  PINAGEM
//  ┌──────────────────────────────────────────────────────────────┐
//  │  SENSORES QTR-RC    GPIO  │  L298N (4 fios)         GPIO    │
//  │  S0 (ext. esq.)      13  │  EN1 — Motor ESQ  A      23     │
//  │  S1                  14  │  EN2 — Motor ESQ  B      22     │
//  │  S2                  16  │  EN3 — Motor DIR  A      18     │
//  │  S3                  17  │  EN4 — Motor DIR  B      19     │
//  │  S4                  21  │                                  │
//  │  S5                  25  │  LED onboard (status)     2     │
//  │  S6                  26  │                                  │
//  │  S7 (ext. dir.)      33  │                                  │
//  │  Emissor IR          32  │                                  │
//  └──────────────────────────┴──────────────────────────────────┘
//
//  COMO FUNCIONA O CONTROLE DE MOTOR SEM PINO ENABLE:
//  Esta L298N usa 4 pinos de direção (EN1..EN4) sem ENA/ENB separados.
//  A velocidade é controlada colocando PWM diretamente nesses pinos:
//    Motor ESQ frente  →  EN1 = PWM,  EN2 = 0
//    Motor ESQ ré      →  EN1 = 0,    EN2 = PWM
//    Motor ESQ parado  →  EN1 = 0,    EN2 = 0
//    (mesmo padrão para Motor DIR com EN3/EN4)
//
//  O ESP32 não tem analogWrite(). Usa o periférico interno LEDC para gerar
//  PWM em qualquer GPIO. Cada pino físico precisa de um "canal LEDC" (número
//  de 0 a 15, puramente interno — não aparece na placa). Aqui usamos:
//    Canal 0 → GPIO 23 (EN1)
//    Canal 1 → GPIO 22 (EN2)
//    Canal 2 → GPIO 18 (EN3)
//    Canal 3 → GPIO 19 (EN4)
//
//  CALIBRAÇÃO:
//  Automática nos primeiros ~6 s. Mova o robô sobre a linha E o fundo.
//  O LED pisca durante a calibração e apaga quando terminar.
// =============================================================================

// -----------------------------------------------------------------------------
//  PAINEL DE CONTROLE
// -----------------------------------------------------------------------------
const bool IMPRIMIR_SERIAL = true;   // false em competição (libera ~2 ms/loop)
const bool LIGAR_MOTORES   = true;   // false para depurar sem movimentar o robô

// -----------------------------------------------------------------------------
//  PINOS FÍSICOS — SENSORES
// -----------------------------------------------------------------------------
static const uint8_t SensorCount                  = 8;
static const uint8_t PINOS_SENSORES[SensorCount]  = {13, 14, 16, 17, 21, 25, 26, 33};
static const uint8_t PINO_EMISSOR                 = 32;

// -----------------------------------------------------------------------------
//  PINOS FÍSICOS — L298N
//  Estes são os únicos pinos que existem na sua ponte H.
//  Velocidade via PWM aplicado diretamente a eles (sem ENA/ENB).
// -----------------------------------------------------------------------------
static const int PINO_EN1 = 23;  // Motor ESQ — direção A
static const int PINO_EN2 = 22;  // Motor ESQ — direção B
static const int PINO_EN3 = 18;  // Motor DIR — direção A
static const int PINO_EN4 = 19;  // Motor DIR — direção B

// -----------------------------------------------------------------------------
//  CANAIS LEDC — internos do ESP32, invisíveis na placa
//  Mapeamento: canal → pino físico acima
// -----------------------------------------------------------------------------
static const uint8_t  LEDC_CANAL_EN1 = 0; // → GPIO 23
static const uint8_t  LEDC_CANAL_EN2 = 1; // → GPIO 22
static const uint8_t  LEDC_CANAL_EN3 = 2; // → GPIO 18
static const uint8_t  LEDC_CANAL_EN4 = 3; // → GPIO 19

static const uint32_t LEDC_FREQ = 20000;  // 20 kHz — inaudível + suave
static const uint8_t  LEDC_RES  = 8;      // 8 bits → duty 0..255

// Wrapper transparente entre arduino-esp32 v2 e v3
static inline void ledc_configurar(uint8_t canal, int pino) {
#if ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcAttachChannel(pino, LEDC_FREQ, LEDC_RES, canal);
#else
  ledcSetup(canal, LEDC_FREQ, LEDC_RES);
  ledcAttachPin(pino, canal);
#endif
}

// -----------------------------------------------------------------------------
//  LED DE STATUS
// -----------------------------------------------------------------------------
static const uint8_t LED_PIN = 2;

// -----------------------------------------------------------------------------
//  PARÂMETROS PID
//  Suba Kp de 10 em 10 se passar direto nas curvas.
//  Suba Kd de 50 em 50 se tremer na reta.
//  Deixe Ki = 0 na maioria das pistas.
// -----------------------------------------------------------------------------
static const int32_t Kp         = 80;
static const int32_t Ki         = 0;
static const int32_t Kd         = 700;
static const int32_t ESCALA_PID = 1000; // evita usar float; divide no final

// -----------------------------------------------------------------------------
//  VELOCIDADE E COMPORTAMENTO
// -----------------------------------------------------------------------------
static const int16_t VEL_MAXIMA  = 255;
static const int16_t VEL_MINIMA  = 100;  // mínimo para não travar nas curvas
static const int16_t VEL_RESGATE = 220;  // força de giro ao perder a linha
static const int32_t FATOR_FREIA = 45;   // reduz vel. proporcional ao desvio

// Cruzamento em X: >= 5 sensores na linha simultaneamente
static const uint8_t  LIMIAR_CRUZAMENTO = 5;
// Threshold para sensor "sobre linha preta" (escala calibrada 0–1000)
static const uint16_t LIMIAR_PRETO      = 600;

// -----------------------------------------------------------------------------
//  ESTADO GLOBAL
// -----------------------------------------------------------------------------
QTRSensors qtr;
uint16_t   sensorValues[SensorCount];

int32_t  erroAnterior = 0;
int32_t  integral     = 0;
int16_t  ultimoPwmEsq = -999; // cache para evitar writes desnecessários no LEDC
int16_t  ultimoPwmDir = -999;

// -----------------------------------------------------------------------------
//  PROTÓTIPOS
// -----------------------------------------------------------------------------
static inline int32_t calcularPID(int32_t erro);
static inline void    acionarMotores(int16_t velEsq, int16_t velDir);
static inline uint8_t contarSensoresNaLinha();
static inline bool    linhaFoiPerdida();

// =============================================================================
//  SETUP
// =============================================================================
void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n=== SEGUIDOR DE LINHA — ESP32 ===");

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Configura LEDC para os 4 pinos da L298N
  ledc_configurar(LEDC_CANAL_EN1, PINO_EN1);
  ledc_configurar(LEDC_CANAL_EN2, PINO_EN2);
  ledc_configurar(LEDC_CANAL_EN3, PINO_EN3);
  ledc_configurar(LEDC_CANAL_EN4, PINO_EN4);
  acionarMotores(0, 0); // garante motores parados no boot

  // Configura QTR-RC
  qtr.setTypeRC();
  qtr.setSensorPins(PINOS_SENSORES, SensorCount);
  qtr.setEmitterPin(PINO_EMISSOR); // apaga o IR entre leituras → mais preciso

  // ── Calibração automática (~6 s) ──────────────────────────────────────────
  Serial.println(">>> CALIBRANDO — mova sobre linha e fundo <<<");
  for (uint16_t i = 0; i < 400; i++) {
    if (i % 20 == 0) digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    qtr.calibrate();
    delay(15);
  }
  digitalWrite(LED_PIN, LOW);
  Serial.println("Calibracao OK!\n");

  Serial.println("  S |  Min  |  Max");
  Serial.println("  --|-------|------");
  for (uint8_t i = 0; i < SensorCount; i++) {
    Serial.printf("  %d |  %4d  |  %4d\n", i,
      qtr.calibrationOn.minimum[i],
      qtr.calibrationOn.maximum[i]);
  }

  if (!LIGAR_MOTORES) {
    Serial.println("\n[!] MOTORES DESLIGADOS — modo debug");
  } else {
    Serial.println("\n[!] Motores ativos — largando em 2 s...");
    delay(2000);
  }
}

// =============================================================================
//  LOOP PRINCIPAL
// =============================================================================
void loop() {
  // 1. LEITURA ─────────────────────────────────────────────────────────────
  uint16_t posicaoLinha = qtr.readLineBlack(sensorValues);
  int32_t  erro         = (int32_t)posicaoLinha - 3500;

  // 2. DEBUG (throttled a 100 ms) ──────────────────────────────────────────
  if (IMPRIMIR_SERIAL) {
    static uint32_t tPrint = 0;
    if (millis() - tPrint >= 100) {
      tPrint = millis();
      for (uint8_t i = 0; i < SensorCount; i++) Serial.printf("%4d ", sensorValues[i]);
      Serial.printf("| pos=%4d  err=%5d\n", posicaoLinha, erro);
    }
  }

  // 3. ANTI-CRUZAMENTO ─────────────────────────────────────────────────────
  //    >= 5 sensores na linha = cruzamento em X ou T → vai reto
  if (contarSensoresNaLinha() >= LIMIAR_CRUZAMENTO) {
    integral     = 0;
    erroAnterior = 0;
    acionarMotores(VEL_MAXIMA, VEL_MAXIMA);
    return;
  }

  // 4. RESGATE DE LINHA PERDIDA ─────────────────────────────────────────────
  //    Só ativa quando nenhum sensor vê linha (evita confundir com linha no S0)
  if (linhaFoiPerdida()) {
    if (erroAnterior < 0) acionarMotores(-VEL_RESGATE,  VEL_RESGATE);
    else                  acionarMotores( VEL_RESGATE, -VEL_RESGATE);
    return;
  }

  // 5. PID ─────────────────────────────────────────────────────────────────
  int32_t correcao = calcularPID(erro);

  // 6. BRAKE BIAS — reduz vel. conforme o desvio ───────────────────────────
  int32_t penalidade  = (abs(erro) * FATOR_FREIA) / 1000;
  int16_t velDinamica = (int16_t)(VEL_MAXIMA - penalidade);
  if (velDinamica < VEL_MINIMA) velDinamica = VEL_MINIMA;

  // 7. ENVIA PARA OS MOTORES ───────────────────────────────────────────────
  acionarMotores(velDinamica + (int16_t)correcao,
                 velDinamica - (int16_t)correcao);
}

// =============================================================================
//  FUNÇÕES
// =============================================================================

static inline uint8_t contarSensoresNaLinha() {
  uint8_t n = 0;
  for (uint8_t i = 0; i < SensorCount; i++)
    if (sensorValues[i] > LIMIAR_PRETO) n++;
  return n;
}

static inline bool linhaFoiPerdida() {
  return (contarSensoresNaLinha() == 0);
}

static inline int32_t calcularPID(int32_t erro) {
  integral += erro;
  if      (integral >  2000) integral =  2000;
  else if (integral < -2000) integral = -2000;

  int32_t derivativo = erro - erroAnterior;
  erroAnterior = erro;

  return ((Kp * erro) + (Ki * integral) + (Kd * derivativo)) / ESCALA_PID;
}

// ─────────────────────────────────────────────────────────────────────────────
//  acionarMotores — única função que toca nos pinos da L298N
//
//  velEsq / velDir: -255 (ré máxima) a +255 (frente máxima)
//
//  Tradução para a L298N sem ENA/ENB:
//    frente  →  ENx = PWM,  ENx+1 = 0
//    ré      →  ENx = 0,    ENx+1 = PWM
//    parado  →  ENx = 0,    ENx+1 = 0
//
//  ledcWrite(canal, duty) escreve no canal LEDC interno do ESP32 que está
//  mapeado para o pino físico correspondente — não é um pino da L298N.
// ─────────────────────────────────────────────────────────────────────────────
static inline void acionarMotores(int16_t velEsq, int16_t velDir) {
  if (!LIGAR_MOTORES) { velEsq = 0; velDir = 0; }

  // Saturação ±255
  if      (velEsq >  255) velEsq =  255;
  else if (velEsq < -255) velEsq = -255;
  if      (velDir >  255) velDir =  255;
  else if (velDir < -255) velDir = -255;

  // Cache: só atualiza o LEDC se os valores mudaram
  if (velEsq == ultimoPwmEsq && velDir == ultimoPwmDir) return;
  ultimoPwmEsq = velEsq;
  ultimoPwmDir = velDir;

  // Motor Esquerdo → EN1 (GPIO 23) e EN2 (GPIO 22)
  if (velEsq >= 0) { ledcWrite(LEDC_CANAL_EN1, velEsq); ledcWrite(LEDC_CANAL_EN2, 0); }
  else             { ledcWrite(LEDC_CANAL_EN1, 0); ledcWrite(LEDC_CANAL_EN2, -velEsq); }

  // Motor Direito → EN3 (GPIO 18) e EN4 (GPIO 19)
  if (velDir >= 0) { ledcWrite(LEDC_CANAL_EN3, velDir); ledcWrite(LEDC_CANAL_EN4, 0); }
  else             { ledcWrite(LEDC_CANAL_EN3, 0); ledcWrite(LEDC_CANAL_EN4, -velDir); }
}
