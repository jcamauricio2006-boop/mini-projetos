#ifndef MOTORES_H
#define MOTORES_H

#define EN1  22
#define EN2  23
#define EN3  25
#define EN4  26
extern int valor_pwm;
extern int dif;

void frente();
void atras();
void esquerda();
void direita();
void parar();
void girar();

#endif
