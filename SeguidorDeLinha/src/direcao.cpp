#include <Arduino.h>
#include "motores.h"

#define EN1  22
#define EN2  23
#define EN3  25
#define EN4  26

void frente(){
    digitalWrite(EN1, HIGH);
    digitalWrite(EN2, LOW);
    digitalWrite(EN3, HIGH);
    digitalWrite(EN4, LOW);

}
void atras() {
    digitalWrite(EN1, LOW);
    digitalWrite(EN2, HIGH);
    digitalWrite(EN3, LOW);
    digitalWrite(EN4, HIGH);
}
void esquerda() {
    digitalWrite(EN1, LOW);
    digitalWrite(EN2, LOW);
    digitalWrite(EN3, HIGH);
    digitalWrite(EN4, LOW);
}
void direita() {
    digitalWrite(EN1, HIGH);
    digitalWrite(EN2, LOW);
    digitalWrite(EN3, LOW);
    digitalWrite(EN4, LOW);
}
