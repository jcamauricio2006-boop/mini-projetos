#include <Arduino.h>
#include <TFT_eSPI.h>
#include <mat.h>

TFT_eSPI tela = TFT_eSPI();

// Dados recebidos do Python
int CPU_PORCEN = 0;
int RAM_PORCEN = 0;
int GPU_PORCEN = 0;
String RELOGIO;

// Tamanho máximo das barras
int gra_inteiro = 220;


void dadosPY()
{
  if (Serial.available() > 0)
  {
    String dados_recebidos = Serial.readStringUntil('\n');

    dados_recebidos.trim();

    // Procura as duas vírgulas
    int primeira_virgula = dados_recebidos.indexOf(',');
    int segunda_virgula = dados_recebidos.indexOf(
      ',',
      primeira_virgula + 1
    );
    int terceira_virgula = dados_recebidos.indexOf(',', segunda_virgula + 1);
    if (primeira_virgula != -1 && segunda_virgula != -1)
    {
      // Python envia: CPU,RAM,GPU

      CPU_PORCEN = dados_recebidos.substring(
        0,
        primeira_virgula
      ).toInt();

      RAM_PORCEN = dados_recebidos.substring(
        primeira_virgula + 1,
        segunda_virgula
      ).toInt();

      GPU_PORCEN = dados_recebidos.substring(
        segunda_virgula + 1
      ).toInt();
      RELOGIO = dados_recebidos.substring(
        terceira_virgula+1
      );
    }
  }
}


void setup()
{
  Serial.begin(115200);

  tela.init();
  tela.setRotation(1);
  tela.fillScreen(TFT_BLACK);

  tela.setTextSize(2);

  // Título
  tela.drawString("sistema", 10, 20);
  
  // Linha separadora
  tela.drawLine(10, 40, 310, 40, TFT_WHITE);

  // Molduras das barras
  tela.drawRect(10, 50, gra_inteiro, 18, TFT_WHITE);//CPU
  tela.drawRect(10, 70, gra_inteiro, 18, TFT_WHITE);//RAM
  tela.drawRect(10, 90, gra_inteiro, 18, TFT_WHITE);//GPU
  
  //linha separadora
  tela.drawLine(10,120, 310, 120, TFT_WHITE);

}

void loop()
{
  dadosPY();

  tela.fillRect(10,50,gra_inteiro,18, TFT_BLACK);//CPU
  tela.fillRect(10,70,gra_inteiro,18, TFT_BLACK);//RAM
  tela.fillRect(10,90,gra_inteiro,18, TFT_BLACK);//GPU
  
  tela.drawString("tempo "+RELOGIO, 180, 20);
  
  //grafico barra
  int gra_porcen_CPU = (gra_inteiro*CPU_PORCEN)/100;
  int gra_porcen_RAM = (gra_inteiro*RAM_PORCEN)/100;
  int gra_porcen_GPU = (gra_inteiro*GPU_PORCEN)/100;
  
  //info
  tela.drawString("CPU:"+ String(CPU_PORCEN)+"%", 240, 50);
  tela.drawString("RAM:"+ String(RAM_PORCEN)+"%", 240, 70);
  tela.drawString("GPU:"+ String(GPU_PORCEN)+"%", 240, 90);
  
  //barra 
  tela.fillRect(10,50,gra_porcen_CPU,18, TFT_GREEN);//CPU
  tela.fillRect(10,70,gra_porcen_RAM,18, TFT_GREEN);//RAM
  tela.fillRect(10,90,gra_porcen_GPU,18, TFT_GREEN);//GPU

  delay(90);
}