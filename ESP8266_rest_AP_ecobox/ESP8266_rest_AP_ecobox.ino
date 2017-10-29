// Ecobox lato ESP8266

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

        Collega un led al pin 2 per il debug
*/

#include <ESP8266WiFi.h>
#include <aREST.h>

// Crea nuova istanza aRest
aREST rest = aREST();

// Parametri WiFi (per protezione wifi e n. canali scommenta)
const char* ssid = "nTheractive EcoBox";
//const char* password = "your_wifi_network_password";
//const int channels = 2;

// Porta (dell'ESP) da cui ascoltare connessioni TCP in entrata
#define LISTEN_PORT           80

// Crea l'istanza del server
WiFiServer server(LISTEN_PORT);

byte ledPin = 2;

/* Variabili da esporre alle API ( Arduino ---> ESP ---> Smartphone)
   192.168.4.1/payload_sensors mostra il contenuto della variabile
*/
String payloadSensors = "$,10,20,78,54,42,&";

/* Prototipi funzioni da esporre alle API ( Smartphone ---> ESP ---> Arduino)
   192.168.4.1/led?params=1                                Accende (1) o spegne (0) il led al pin 2
   192.168.4.1/actuatorsControl?params= 0,1,255,128,1,1    Manda una stringa con i comandi per gli attuatori
*/
int ledControl(String command);
int actuatorsControl(String command);

// Flag per indicare la corretta ricezione dei dati dei sensori 
bool stringComplete = false;

void setup(void) {

  // Avvia seriale e fai il flush
  Serial.begin(9600);
  while (Serial.available()) {
    Serial.read();
  }

  pinMode(ledPin, OUTPUT);

  // Esposizione variabili
  rest.variable("payloadSensors", &payloadSensors);

  // Esposizione funzioni
  rest.function("ledControl", ledControl);
  rest.function("actuatorsControl", actuatorsControl);

  // Assegna nome e ID all'ESP (ID deve essere lunga 6 caratteri)
  rest.set_id("000001");
  rest.set_name("Ecobox Classe 4AS");

  // Imposta rete WiFi
  //WiFi.softAP(ssid, password, channels);  // http://arduino-esp8266.readthedocs.io/en/latest/esp8266wifi/soft-access-point-class.html
  WiFi.softAP(ssid);

  Serial.println("#WiFi creata");

  // Avvia il server
  server.begin();
  Serial.println("#Server avviato");

  // Mostra l'indirizzo IP dell'ESP
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("#Indirizzo IP dell'AP: ");
  Serial.println(myIP);
}

void loop() {

  // Gestione chiamate REST
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
  while (!client.available()) {
    delay(1);
  }
  rest.handle(client);

}

// Funzioni accessibili dalle API
/*You can also define your own functions in your sketch that can be called
   using the REST API. To access a function defined in your sketch, you have
   to declare it first, and then call it from with a REST call.
   Note that all functions needs to take a String as the unique argument
   (for parameters to be passed to the function) and return an integer.
   For example:
   If your aREST instance is called "rest" and the function "ledControl":

       rest.function("led",ledControl); declares the function in the Arduino sketch
       /ledControl?params=0 executes the function

       /actuatorsControl?params=1,0,1,255,128,1 executes the function
*/

int ledControl(String command) {

  //Serial.print("#Ricevuto comando ledControl: ");
  //Serial.println(command);

  int state = command.toInt();
  //Serial.println(state);
  //Serial.println("#");
  digitalWrite(ledPin, state);
  return 1; // La funzione deve sempre restituire un intero
}

int actuatorsControl(String actuatorsCommands) {

  //Serial.println("#Ricevuto comando actuatorsControl: ");
  Serial.print(actuatorsCommands);
  //Serial.println("#");
  return 2; // La funzione deve sempre restituire un intero

}

/*
  SerialEvent occurs whenever a new data comes in the hardware serial RX. This
  routine is run between each time loop() runs, so using delay inside loop can
  delay response. Multiple bytes of data may be available.
*/
void serialEvent() {

  String inputString;
  char inChar = (char)Serial.read();

  if (inChar == '&') {

    while (Serial.available()) {
      // get the new byte:
      char inChar = (char)Serial.read();
      // add it to the inputString:
      inputString += inChar;

      payloadSensors = inputString;

      // if the incoming character is a newline, set a flag so the main loop can
      // do something about it:
      if (inChar == '\n') {
        stringComplete = true;
        Serial.println("OK");
      }
    }
  }
}
