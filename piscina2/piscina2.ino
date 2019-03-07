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


//Varial para verificar modo de funcionamento 
int modo = 1;



//Variaveis de Tempo

long currentMillis;
long nextMillis;
long timerMiliis;

boolean funcBomba = false;


void setup(){

  Serial.begin(9600);

  //Iniciando o recebimento IR
  Serial.println("Enabling IRin");
  irrecv.enableIRIn(); 
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

  timerMiliis = -1;

}

void loop(){

    // Leitura IR 
    if (irrecv.decode(&results)) {
        Serial.println(results.value, HEX);
        irrecv.resume(); // Recebe o proximo valor
    }

    // Verifica teclas pressionadas, set modo
    if(results.value == 0xF171 || results.value == 0x95B38CA){
        Serial.println("Modo 2");
        if(modo == 1){
            ligaBomba();
            modo = 2;
        }else if(modo == 2){
            modo = 1;
            Serial.println("Modo 1");
            desligaBomba();
        }else if(modo == 4){
            desligaBomba();
            delay(1000);
            ligaBomba();

            modo = 2;
            
        }
    }else if(results.value == 0xF1B1  || results.value == 0xB3EE6F0E){
        Serial.println("desliga termostato");
        if(modo != 3){
          desligaBomba();
            modo = 3;
        }else{
            modo = 1;
        }
    }else if(results.value == 0xF1A1  || results.value == 0x19130F62){
        Serial.println("Liga Timer");
        if(modo != 4){
            ligaBomba();
            timerMiliis = millis() + 600000;
            modo = 4;
        }else{
            desligaBomba();
            modo = 1;
        }
    }
    


    // Verifica qual modo de funcionamento esta ativo 
    switch (modo){
        case 1:
            Termostato();
            break;

        case 2:
            Serial.println("Bomba Ligada");

            //ligaBomba();
            break;
        
        case 3:
            Serial.println("Desligado");
            break;

        case 4:
            timer();
            break;
    
        default:
            break;
    }


    results.value = 0;
    delay(100);
}





// Função de termostato
void Termostato(){

    currentMillis = millis();
    if(currentMillis >= nextMillis){

        nextMillis = millis() + 60000;
        //ler temperatura
        double dt = verificaTemp();

        if(dt >= 2.5){
            digitalWrite(rele1, HIGH);
            digitalWrite(rele2, HIGH);
            
            funcBomba = true;
            //Serial.println("liga Pelo ddt");
        }else if(dt <= 1 && funcBomba == true){
            digitalWrite(rele1, LOW);
            digitalWrite(rele2, LOW);
            funcBomba = false;
            Serial.println("desliga Pelo ddt");
        }

    }



}

//função de Timer
void timer(){
    currentMillis = millis();

    if(currentMillis >= timerMiliis){
        desligaBomba();
        modo = 1;
    }
}



// Desliga a bomba
void ligaBomba(){
    digitalWrite(rele1, HIGH);
    digitalWrite(rele2, HIGH);
}

// Liga a bomba
void desligaBomba(){
    digitalWrite(rele1, LOW);
    digitalWrite(rele2, LOW);
}





// Ler Temperatura
double verificaTemp(){

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
    //ligaTemp(temp2 - temp1);

    return temp2 - temp1;
  
}
