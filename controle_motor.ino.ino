#include<Math.h>
#define ANALOG_PIN_0 36
const int RPWM_Output = 22;
const int LPWM_Output = 23;

const int freq = 5000; 
const int ledChannel = 0;
const int resolution = 12; //maximo: 12
const int current_pin = 34;
const float securityCurrent=10.0;
const float maxCurrent=15;
const int linearIncrementStep=20;
const unsigned int maxPWM = pow(2,resolution)-1;
unsigned int PWMThreshold = maxPWM;
unsigned int limitePWM = 0;
unsigned int atualPWM=0;
unsigned int maxAccelerationTime=4000; //miliseconds
unsigned int maxPotentiometer=2200;
unsigned long lastTurn = 0; //
int tipoIncremento = 0; //0 - Quadrático | 1 - Linear
int intensidadeAnterior=0;
short int sequentialOverLoads=0;
short int sequentialRiskOverLoads=0;
unsigned int overLoadDelay;
bool overLoad=false;
void setup() {
  Serial.begin(115200);
  pinMode(RPWM_Output, OUTPUT);
  pinMode(LPWM_Output, OUTPUT);
  pinMode(current_pin, INPUT);
  ledcSetup(ledChannel, freq, resolution);
  ledcAttachPin(RPWM_Output, ledChannel);
}


void calculaPWM(){
  int read_current;
  float rms_current = 0.0;
  read_current = 2815- analogRead(current_pin);
  rms_current = read_current / 100;
  //Serial.println(rms_current);
  if(rms_current>maxCurrent){
    sequentialRiskOverLoads++;
    sequentialOverLoads++;
    char buff[100];
    sprintf(buff, "*** ALERTA **** Max: %d Corrent: %f Qtd: %d", atualPWM, rms_current, sequentialOverLoads); 
    Serial.println(buff);
    atualPWM=0;
    tipoIncremento=1;
    overLoadDelay = 2000;
    overLoad=true;  
  }else{
    if(rms_current>=securityCurrent){
      overLoad=true;
      sequentialOverLoads++;
      char buff[50];
      sprintf(buff, "Max: %d Corrent: %f Qtd: %d", atualPWM, rms_current, sequentialOverLoads); 
      Serial.println(buff);
      if(sequentialOverLoads==1){
        atualPWM = ((float)atualPWM)*0.75;
        tipoIncremento=1;
        overLoadDelay = 100;
      }else if(sequentialOverLoads==2){
        atualPWM = ((float)atualPWM)*0.5;
        tipoIncremento=1;
        overLoadDelay=200;
      }else if(sequentialOverLoads==3){
        atualPWM = ((float)atualPWM)*0.25;
        tipoIncremento=1;
        overLoadDelay = 300;      
      }else if(sequentialOverLoads==4){
        atualPWM = 0;
        tipoIncremento=0;
        overLoadDelay = 400;      
      }else{
        atualPWM = 0;
        tipoIncremento=0;
        overLoadDelay = 500;
      }
    }else{
      overLoad=false;
      sequentialOverLoads=0;
      if(tipoIncremento==0)
        if(atualPWM==0)
          atualPWM=1;
        else
          atualPWM = atualPWM*2;
      else
        atualPWM = atualPWM + linearIncrementStep;
    }
  }
  if(atualPWM>maxPWM)
    atualPWM=maxPWM;  
}

void loop() {
  int analog_value = 0;
  int intensidade;
  float ganhoIntensidade;
  analog_value = analogRead(ANALOG_PIN_0);
  intensidade = analog_value-920;
  if(intensidade<20)
    intensidade=0;
  if(intensidade>maxPotentiometer)
    intensidade=maxPotentiometer;
  calculaPWM();
  //Serial.println(intensidade);
  if(intensidade>intensidadeAnterior){
    unsigned long maxAcceleration = ((float)maxPotentiometer)/((float)maxAccelerationTime)*((float)(millis()-lastTurn));
    if(intensidade-intensidadeAnterior>maxAcceleration)
      intensidade=intensidadeAnterior+maxAcceleration;
  }
  char buff[100];
  sprintf(buff, "Anterior: %d Atual %d PWM Max: %d", intensidadeAnterior, intensidade, atualPWM);
  Serial.println(buff);
  intensidadeAnterior=intensidade;
  lastTurn=millis();
  intensidade=(int)((float)intensidade/(float)maxPotentiometer*((float)atualPWM));   

//  if(ganhoIntensidade>aceleracaoMaxima)
//    intensidade = (float)intensidadeAnterior*aceleracaoMaxima;


  
  digitalWrite(LPWM_Output, 0);
  ledcWrite(ledChannel, intensidade);
  //Serial.println(atualPWM);
  if(overLoad)
    delay(overLoadDelay);
  delay(100);
  
}
