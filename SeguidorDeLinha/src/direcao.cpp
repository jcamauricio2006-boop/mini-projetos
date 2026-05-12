#include <Arduino.h>
#include "motores.h"
// Entradas =====================================================================
#define EN1  22
#define EN2  23
#define EN3  25
#define EN4  26
//Parametro PWM ================================================================
int valor_pwm = 0;
int dif;
// Direções ====================================================================
void frente(){
//Relacionando o valor PWM com as variáveis dos motores=========================
   for (valor_pwm = 0; valor_pwm < 255 ; valor_pwm++){
        analogWrite(EN1, valor_pwm); 
        analogWrite(EN3, valor_pwm); 
        analogWrite(EN2, 0);         
        analogWrite(EN4, 0);   
        delay(10);
   }
}
void atras() {
    for (valor_pwm = 0 ; valor_pwm < 255; valor_pwm++){
        analogWrite(EN2, valor_pwm);
        analogWrite(EN4, valor_pwm);
        analogWrite(EN1, 0);
        analogWrite(EN3, 0);
        delay(10);
    }
}
// OBS temp -  tenho que ver com mais cautela o diferencial na hora de virar o carrinho.
void esquerda() {
    for (valor_pwm = 0 ; valor_pwm < 255; valor_pwm++){
        dif = (int)(valor_pwm*0.6);
        analogWrite(EN3, valor_pwm); 
        analogWrite(EN1, dif);
        analogWrite(EN2, 0);
        analogWrite(EN4, 0);
        delay(10);
    }
}

void direita() {
    for (valor_pwm = 0 ; valor_pwm < 255; valor_pwm++){
        dif = (int)(valor_pwm*0.6);
        analogWrite(EN1, valor_pwm); 
        analogWrite(EN3, dif);         
        analogWrite(EN2, 0);
        analogWrite(EN4, 0);
        delay(10);
    }
}
void parar() {
    for (valor_pwm= 255; valor_pwm >= 255; valor_pwm--){
        analogWrite(EN1, valor_pwm);
        analogWrite(EN2, valor_pwm);
        analogWrite(EN3, valor_pwm);
        analogWrite(EN4, valor_pwm);
        delay(10);
    } 
}
// direções extremas ============================================
void esquerdaT() {
    for (valor_pwm = 0; valor_pwm <255; valor_pwm++){
        analogWrite(EN1, valor_pwm);
        analogWrite(EN2, 0);
        analogWrite(EN3, 0);
        analogWrite(EN4, 0);
    }
}
void direitaT() {
    for (valor_pwm = 0; valor_pwm <255; valor_pwm++){
        analogWrite(EN1, 0);
        analogWrite(EN2, 0);
        analogWrite(EN3, valor_pwm);
        analogWrite(EN4, 0);
    }
}
/*
tabela de direção 
                    EN1/EN3   |   EN2/EN4
sentido horario -    true     |   false
sentido antihorario- false    |   true
descanso -           false    |   false
freio  -             true     |   true
*/
