/*
 * Codigo para Temostato de piscina
 * Com sensor de temperatura e sensor IR
 * 
 * Autor: Artur Massaro Gonzaga
 */

#include <IRremote.h>
#include <NTC_Thermistor.h>

#define rele1  6
#define rele2  7

//Variaveis sensor de temperatura
#define SENSOR_PIN             A1
#define SENSOR2_PIN            A2
#define REFERENCE_RESISTANCE   10000
#define REFERENCE_RESISTANCE2  10
#define NOMINAL_RESISTANCE     900
#define NOMINAL_TEMPERATURE    25
#define B_VALUE                3950


NTC_Thermistor* t1 = NULL;
NTC_Thermistor* t2 = NULL;

double temp1;
double temp2;




//Variaveis do sensor IR
int RECV_PIN = 11;
IRrecv irrecv(RECV_PIN);
decode_results results;


//Variaveis booleanas 
boolean bomb = false;
boolean verfTemp = true;
boolean tempBomb = false;
boolean func = true;
boolean timer = false;


//Variaveis de Tempo

long currentMillis;
long nextMillis;
long switchBomb;


void setup()
{
  Serial.begin(9600);

  //Iniciando o recebimento IR
  Serial.println("Enabling IRin");
  irrecv.enableIRIn(); // Start the receiver
  Serial.println("Enabled IRin");

  //Configurando os Reles
  pinMode(rele1, OUTPUT);
  pinMode(rele2, OUTPUT);

  digitalWrite(rele1, LOW);
  digitalWrite(rele2, LOW);


  //Configurando os sensores de temperatura
  t1 = new NTC_Thermistor(
    SENSOR_PIN,
    REFERENCE_RESISTANCE,
    NOMINAL_RESISTANCE,
    NOMINAL_TEMPERATURE,
    B_VALUE
  );

  t2 = new NTC_Thermistor(
    SENSOR2_PIN,
    REFERENCE_RESISTANCE2,
    NOMINAL_RESISTANCE,
    NOMINAL_TEMPERATURE,
    B_VALUE
  );




  //setando millis
  currentMillis = millis();
  nextMillis = millis();

  switchBomb = -1;

}

void loop() {

  // Leitura IR 
  if (irrecv.decode(&results)) {
    Serial.println(results.value, HEX);
    irrecv.resume(); // Recebe o proximo valor
  }

  // Leitura temperatura
  currentMillis = millis();
  if(currentMillis >= nextMillis){
    verificaTemp();
    nextMillis = currentMillis + 60000;
    
  }

  


  if(results.value == 0xF171 || results.value == 0x95B38CA){
    Serial.println("Liga Rele");
    ligaBomba();
  }else if(results.value == 0xF1B1  || results.value == 0xB3EE6F0E){
    Serial.println("desliga Rele");
    desligaBomb();
  }else if(results.value == 0xF1A1  || results.value == 0x19130F62){
    Serial.println("Liga Timer");
    timerBomb(10);
  }
  

  if(verfTemp){
    
    ligaTemp(temp2 - temp1);
    
  }


  currentMillis = millis();
  if(currentMillis >= switchBomb && timer){
    bomb = true;
    ligaBomba();
  }


  results.value = 0;
  delay(100);
}



// Ler Temperatura
void verificaTemp(){

  int i = bitRead(PORTD,rele1);


  digitalWrite(rele1, LOW);
  digitalWrite(rele2, LOW);
    Serial.println("Desliga verficaTemp");
  
  temp1 = t1->readCelsius() - 4;
  temp2 = t2->readCelsius() - 4;

  Serial.print("Temperatura Piscina: ");
  Serial.print(String(temp1) + "ºC | ");
  Serial.println("Temperatura Placa: " + String(temp2) + "ºC");

  if(i == 1){
    digitalWrite(rele1, HIGH);
    digitalWrite(rele2, HIGH);
    Serial.println("Liga verficaTemp");
  }
  ligaTemp(temp2 - temp1);
  
}


//verifica temperatura
void ligaTemp(double dt){
  if(dt >= 2.5){
    digitalWrite(rele1, HIGH);
    digitalWrite(rele2, HIGH);
    
    tempBomb = true;
    //Serial.println("liga Pelo ddt");
  }else if(dt <= 1 && tempBomb == true){
    digitalWrite(rele1, LOW);
    digitalWrite(rele2, LOW);
    tempBomb = false;
    Serial.println("desliga Pelo ddt");
  }
  
  
}


//switch bomba
void ligaBomba(){
  if(bomb){
    digitalWrite(rele1, LOW);
    digitalWrite(rele2, LOW);
    Serial.println("Desliga LigaBomba");
    verfTemp = true;
    bomb = false;
    tempBomb = false;
    delay(5000);
  }else{
    digitalWrite(rele1, HIGH);
    digitalWrite(rele2, HIGH);
    Serial.println("Liga LigaBomba");
    bomb = true;
    verfTemp = false;
  }
}


//switch funcionamento
void desligaBomb(){
  digitalWrite(rele1, LOW);
  digitalWrite(rele2, LOW);
  Serial.println("Desliga desligaBomb");

  if(func == true){
    verfTemp = false;
    bomb = false;
    //tempBomb = false;
    func = false;
    Serial.println("Desliga Funcionamento");
  }else{
    verfTemp = true;
    bomb = false;
    //tempBomb = true;
    func = true;
    Serial.println("Liga Funcionamento");
    verificaTemp();
  }
}

void timerBomb(int time){

  timer = true;
  long tempo = 60000 * time;

  switchBomb = tempo;

  bomb = false;
  ligaBomba();
}
