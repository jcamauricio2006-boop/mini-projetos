#include <Arduino.h>
#include <QTRSensors.h>
#include <motores.h>
//entrada dos motores ======================================================
#define EN1  22
#define EN2  23
#define EN3  25
#define EN4  26
// definindo sensores  ======================================================
QTRSensors qtr;
const uint8_t  NUM_SENSORES = 8;
uint16_t leitura[NUM_SENSORES];
uint8_t pinosSensor[NUM_SENSORES] = {13,14,27,26,25,33,32,35};
const uint8_t  PINO_EMISSOR = 14;
const uint8_t  PINO_LED_CAL = 2;
const int32_t ZONA_MORTA = 150;
// PWM dos motores ===========================================================
int valor_pwm = 0;
int dif; 
// ===========================================================================
void setup() {
    Serial.begin(115200);
//motores===================================================================
    pinMode(EN1, OUTPUT);
    pinMode(EN2, OUTPUT);
    pinMode(EN3, OUTPUT);
    pinMode(EN4, OUTPUT);
//====================================================================
    qtr.setTypeRC();
    qtr.setSensorPins(pinosSensor, NUM_SENSORES);
    qtr.setEmitterPin(PINO_EMISSOR);
    qtr.setTimeout(2500);
    pinMode(PINO_LED_CAL, OUTPUT);
    digitalWrite(PINO_LED_CAL, HIGH);
    Serial.print("Calibrando sensores");
//calibrando sensores ==========================================
    for (uint16_t i = 0; i < 250; i++) {
        qtr.calibrate();
        delay(10);
        // Na hora da competição ele calibrará indo para esquerda, assim nao precisando de ajuda manual.
        esquerdaT();
    }
//calibração concluida ===========================================
    Serial.println("Calibração concluida");
    digitalWrite(PINO_LED_CAL, LOW);
    delay(1000);
}
void loop() {
// leitura da linha preta ===================================================
    uint16_t posicaoBruta = qtr.readLineBlack(leitura);
    int32_t erro = map(posicaoBruta, 0, 7000, 3500, -3500);
    erro = constrain(erro, -3500, 3500);
    if (erro > -ZONA_MORTA && erro < ZONA_MORTA) {
        erro = 0;
    }
    for (uint8_t i = 0; i < NUM_SENSORES; i++) {
        Serial.print("S");
        Serial.print(i + 1);
        Serial.print(":");
        Serial.print(leitura[i]);
        if (i < NUM_SENSORES - 1) Serial.print(" | ");
    
    }
 // condições de direção do seguidor de linha =============================
    if (erro == 0){
        frente();
    } else if (erro >= 1500){
        direita();
    } else if (erro <= -1500){
        esquerda();
    } else {
        parar();
    }
// mandando print pro terminal , sobre a variavel de erro ==============================
    Serial.print("  >>  Erro: ");
    Serial.println(erro);
    delay(50);
}
