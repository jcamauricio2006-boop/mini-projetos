#include <Arduino.h>
#include <QTRSensors.h>
#include <motores.h>
#define EN1  22
#define EN2  23
#define EN3  25
#define EN4  26
QTRSensors qtr;
const uint8_t  NUM_SENSORES = 8;
uint16_t leitura[NUM_SENSORES];
uint8_t pinosSensor[NUM_SENSORES] = {13,14,27,26,25,33,32,35};
const uint8_t  PINO_EMISSOR = 14;
const uint8_t  PINO_LED_CAL = 2;
const int32_t ZONA_MORTA = 150;
void setup() {
    Serial.begin(115200);
//===================================================================
//motores
    pinMode(EN1, OUTPUT);
    pinMode(EN2, OUTPUT);
    pinMode(EN3, OUTPUT);
    pinMode(EN4, OUTPUT);
    int pot1 = constrain(analogRead(EN1), 0, 100);
    int pot2 = constrain(analogRead(EN2), 0, 100);
    int pot3 = constrain(analogRead(EN3), 0, 100);
    int pot4 = constrain(analogRead(EN4), 0, 100);

//====================================================================
    qtr.setTypeRC();
    qtr.setSensorPins(pinosSensor, NUM_SENSORES);
    qtr.setEmitterPin(PINO_EMISSOR);
    qtr.setTimeout(2500);
    pinMode(PINO_LED_CAL, OUTPUT);
    digitalWrite(PINO_LED_CAL, HIGH);
    Serial.print("Calibrando sensores");
    for (uint16_t i = 0; i < 250; i++) {
        qtr.calibrate();
        delay(10);
        esquerda();
    }
    Serial.println("Calibração concluida");
    digitalWrite(PINO_LED_CAL, LOW);
    delay(1000);
}
void loop() {
    // ligando os motores
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
    // condições para o sentido da linha
    }
    if (erro = -3500){
        esquerda();
    } else if ( erro = 3500){
        direita();
    } else if (erro = 0){
        frente();
    }
    
    Serial.print("  >>  Erro: ");
    Serial.println(erro);
    delay(50);
}