// Ecobox lato ATMega328

// Documentazione libreria aRest https://github.com/marcoschwartz/aREST

/*
      SCHEMA DI FUNZIONAMENTO:
        Arduino/ATMega <-------> ESP8266 <-----------> Smartphone/PC
           softSerial            Seriale  Rete locale   appInventor
        Comandi per gli attuatori: contenuti nella stringa actuatorsCommands
                                   (numeri separati da virgole, primo elemento "$",
                                   ultimo elemento "&").
                                   Quando Arduino ha ricevuto la stringa corretta,
                                   risponde all'ESP con un "OK".
        Dati dei sensori: contenuti nella stringa payloadSensors
                          (numeri separati da virgole, primo elemento "$",
                          ultimo elemento "&").
                          Quando l'ESP ha ricevuto la stringa corretta,
                          risponde all'Arduino con un "OK".
                          Arduino spedisce i dati dei sensori all'ESP ogni minuto.
*/

#include <SoftwareSerial.h>

//Stringa da spedire all'ESP con
//i dati dei sensori: "$,0,0,0,0,0,&"
String payloadSensors = "";

//Stringa da ricevere dall'ESP con
//i comandi per gli attuatori: "$,0,0,0,0,0,&"
String actuatorsCommands = "";

// Flag per indicare la corretta ricezione dei dati dei sensori
bool stringComplete = false;

long tempoSensori = 60000;
long tempoPrec;

char inChar;
String inString = "";

SoftwareSerial swSer(10, 11);

void setup(void) {

  // Delay per dare tempo all'ESP di avviarsi
  delay(5000);

  // Avvia seriali e fai il flush
  Serial.begin(9600);

  swSer.begin(9600);
  while (swSer.available()) {
    swSer.read();
  }

  Serial.println("EcoBox ready!");

  tempoPrec = millis();

}

void loop() {

  // In ascolto per comandi attuatori...
  while (swSer.available() > 0) {
    
    inChar = (char)swSer.read();
    //Serial.print(carattere);
    inString += inChar;
    
    if (inChar == '&') {
      Serial.println(inString);
      actuatorsCommands = inString;
      inString = "";

      //azionaAttuatori(actuatorsCommands);
      
      break;
    }
  }

  // Una volta al minuto spedisci payload con dati sensori...
  if (millis() - tempoPrec >= tempoSensori) {

    //payloadSensors = leggiSensori();
    payloadSensors = "$,21,34,1020,0,298,&";

    swSer.println();
    tempoPrec = millis();
  }

}
