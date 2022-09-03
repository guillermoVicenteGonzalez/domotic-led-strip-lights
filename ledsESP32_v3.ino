/***********************************************************
 * Autor: Guillermo Vicente Gonzalez
***********************************************************/


/************************************************************
 * Librerias
************************************************************/

#include <WiFi.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

/************************************************************
 * Funciones
************************************************************/

/************************************************************
 * Constantes
************************************************************/


IPAddress ip(192, 168, 0, 121);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress dns (192, 168, 0, 1);

//constantes para el adafruit
#define WIFI_SSID       "casita1"
#define WIFI_PASS       "monoxidodedihidrogeno"
#define IO_USERNAME    "campodetenis"
#define IO_KEY         "aio_vCbx58Z4a8DPGZ5HFamj6Zaey7Of"

//constantes del programa
#define LED 32 //led rojo a secas
#define PIN 2 //tira de leds
#define DETECTOR 18 //detector
#define NUMPIXELS 30 //numero de leds de la tira
#define DELAYVAL 500

//objetos de adafruit.io


//objetos de la tira led
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

//handler del proceso
TaskHandle_t Task1;

//colores
const uint32_t rojo = pixels.Color(255,0, 0);
const uint32_t verde = pixels.Color(0,155, 0);
const uint32_t rosa = pixels.Color(230,0, 126);
const uint32_t blanco = pixels.Color(255,255,255);

//variables compartidas
uint32_t colorActual =pixels.Color(0,0,0);
uint32_t colorAux;
int modoActual = 1;
bool interruptor = false;
bool sensorActivo = false;


//variables programa
char junk;
String inputString = "";
String currentOption = "1";
bool deteccionMovimiento = true; //indica si se le hace caso al detector o no
int value = 0;
int contador = 1; //para la secuencia de inicio
int bandera =1; //para no repetir la animacion del sensor
unsigned long startTime;
unsigned long tiempoPasado;
unsigned long tiempoEspera = 30 * 1000;

WiFiServer server(80);


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("conectado");
  pinMode(LED,OUTPUT);
  pinMode(DETECTOR,INPUT);

  pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
  pixels.clear();
  pixels.show();

  //creo la task para el otro nucleo
  xTaskCreatePinnedToCore(
      Task1code, 
      "Task1", 
      10000,  
      NULL,  
      0,  
      &Task1,  
      0); 


  WiFi.config(ip,dns,gateway,subnet);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
      colorWipe(pixels.Color(255,0,0),5);
  }

  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
  server.begin();
  colorWipe(pixels.Color(0,255,0),10);
  delay(500);
  colorWipe(pixels.Color(0,0,0),5);
  Serial.println();
}

void loop() {
  WiFiClient client = server.available();   // listen for incoming clients
  if (client) {                             // if you get a client,
    //Serial.println("New Client.");           // print a message out the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        //Serial.write(c);                    // print it out the serial monitor
        if (c == '\n') {                    // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

            // the content of the HTTP response follows the header:
            /*
            client.print("Click <a href=\"/V\">here</a> to turn the LED green.<br>");
            client.print("Click <a href=\"/R\">here</a> to turn the LED red.<br>");
            client.print("Click <a href=\"/R\">here</a> to turn the LED pink.<br>");
            client.print("Click <a href=\"/O\">here</a> to turn the LED rainbow.<br>");*/

            // The HTTP response ends with another blank line:
            client.println();
            // break out of the while loop:
            break;
          } else {    // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }

        // Check to see if the client request was "GET /H" or "GET /L":
        if (currentLine.endsWith("GET /V")) {
          colorActual = pixels.Color(0,255, 0);
          //client.print("Verde");
          interruptor = true; 
        }
        if (currentLine.endsWith("GET /R")) {
          colorActual= pixels.Color(255,0, 0);
          //client.print("Rojo");
          interruptor = true;
        }if (currentLine.endsWith("GET /P")) {
          colorActual = rosa;
          //client.print("Rosa");
          interruptor = true;
          Serial.println(interruptor);
        }if (currentLine.endsWith("GET /A")){
          //client.print("Azul");
          colorActual = pixels.Color(0,0,255);
        }if (currentLine.endsWith("GET /O")) {
          modoActual = 0;
          //client.print("Arco iris");
          interruptor = true;
        }if(currentLine.endsWith("GET /I")){
          //i de idle
          //client.print("Modo Normal");
          modoActual = 1;
        }if(currentLine.endsWith("GET /H")){
          //client.print("Modo pulso");
          //H de heartbeat
          modoActual=3;
        }if(currentLine.endsWith("GET /C")){
          //client.print("Modo chase");
          //C de chase
          modoActual=2;
        }if (currentLine.endsWith("GET /Z")){
          //client.print("Zelda");
          colorActual = pixels.Color(0,155, 0);
        }if (currentLine.endsWith("GET /M")){
          //client.print("Morado");
          colorActual = pixels.Color(149,59, 232);
        }if (currentLine.endsWith("GET /Y")){
          //client.print("Amarillo");
          colorActual = pixels.Color(255,248, 56);
        }if (currentLine.endsWith("GET /B")){
          //client.print("Blanco");
          colorActual = pixels.Color(255,255,255);
        }if (currentLine.endsWith("GET /D")){
          //client.print("Apagado");
          pixels.clear();
          pixels.show();
          //colorActual = pixels.Color(0,0,0);
        }if (currentLine.endsWith("GET /T")){
          //client.print("Turquesa");
          colorActual = pixels.Color(93,193,185);
        }if (currentLine.endsWith("GET /N")){
          //client.print("Naranja");
          colorActual = pixels.Color(255,128,0);
        }if (currentLine.endsWith("GET /S")){
          //client.print("modo sensor");
          Serial.println("sensor activado");
          sensorActivo = true;
          pixels.clear();
        }if (currentLine.endsWith("GET /W")){
          //client.print("sensor apagado");
          sensorActivo = false;
        }
      }
    }
    // close the connection:
    client.stop();
    //Serial.println("Client Disconnected.");
  }  
    //delay(2000);
   
}

//codigo del otro core
void Task1code( void * parameter) {
  for(;;) {
        //colorWipe(pixels.Color(255,0, 0),50);
        if(!sensorActivo){
          switch (modoActual){
            case 0:
              rainbow(50);
              //theaterChaseRainbow(50);
              break;
            case 1:
            pixels.setBrightness(255);     
              if(colorAux != colorActual){
                //interruptor = false;
                colorAux = colorActual;
                colorWipe(colorActual,50);              
              }
              break;
            case 2:
              theaterChase(colorActual,100);
              break;
            case 3:
               cambiarColorOndas(colorActual);
               break;
          }
        }else{
          if(digitalRead(DETECTOR) == HIGH){
             startTime = millis();
            //ya se mantienen en su color (o en el que toque)
            Serial.println("sensor HIGH");
          }
          
          if((tiempoPasado - startTime) < tiempoEspera){
              if(bandera == 0){
              cambiarColorBrillo(colorActual);
              bandera = 1;
            }
            Serial.println((tiempoPasado - startTime));
          }else{
            bandera = 0;
            Serial.println("SensorLow");
            pixels.clear();
            pixels.show();
          }
          tiempoPasado = millis();
        }
  }
}



/*********************************************************************
 * MIS ANIMACIONES
 *******************************************************************/

void cambiarColorBrillo(uint32_t nuevoColor){
  int tiempoTotal = 2000;
  int tiempoRelativo = 0;
  tiempoRelativo = tiempoTotal /255;  
  
  pixels.clear();
  for(int i=0;i<255;i++){
    pixels.setBrightness(i);
    for(int i=0;i<NUMPIXELS;i++){
      pixels.setPixelColor(i,nuevoColor);
    }
    pixels.show();
    delay(tiempoRelativo);
  }
}


void cambiarColorOndas(uint32_t nuevoColor){
  int tiempoTotal = 5000;
  int tiempoRelativo = 0;
  tiempoRelativo = tiempoTotal /255;  

//ida
  pixels.clear();
  for(int i=0;i<255;i++){
    pixels.setBrightness(i);
    for(int i=0;i<NUMPIXELS;i++){
      pixels.setPixelColor(i,nuevoColor);
    }
    pixels.show();
    delay(tiempoRelativo);
  }

//vuelta
  for(int i=255;i>10;i--){
    pixels.setBrightness(i);
    for(int i=0;i<NUMPIXELS;i++){
      pixels.setPixelColor(i,nuevoColor);
    }
    pixels.show();
    delay(tiempoRelativo);
  }
  
  pixels.show();
}


/*********************************************************************
 * ANIMACIONES ADAFRUIT NEOPIXEL
 * ******************************************************************/
void colorWipe(uint32_t color, int wait) {
  for(int i=0; i<pixels.numPixels(); i++) { // For each pixel in strip...
    pixels.setPixelColor(i, color);         //  Set pixel's color (in RAM)
    pixels.show();                          //  Update strip to match
    delay(wait);                           //  Pause for a moment
  }
}



 // Theater-marquee-style chasing lights. Pass in a color (32-bit value,
// a la strip.Color(r,g,b) as mentioned above), and a delay time (in ms)
// between frames.
void theaterChase(uint32_t color, int wait) {
  for(int a=0; a<10; a++) {  // Repeat 10 times...
    for(int b=0; b<3; b++) { //  'b' counts from 0 to 2...
      pixels.clear();         //   Set all pixels in RAM to 0 (off)
      // 'c' counts up from 'b' to end of strip in steps of 3...
      for(int c=b; c<pixels.numPixels(); c += 3) {
        pixels.setPixelColor(c, color); // Set pixel 'c' to value 'color'
      }
      pixels.show(); // Update strip with new contents
      delay(wait);  // Pause for a moment
    }
  }
}

// Rainbow cycle along whole strip. Pass delay time (in ms) between frames.
void rainbow(int wait) {
  // Hue of first pixel runs 5 complete loops through the color wheel.
  // Color wheel has a range of 65536 but it's OK if we roll over, so
  // just count from 0 to 5*65536. Adding 256 to firstPixelHue each time
  // means we'll make 5*65536/256 = 1280 passes through this outer loop:
  for(long firstPixelHue = 0; firstPixelHue < 5*65536; firstPixelHue += 256) {
    for(int i=0; i<pixels.numPixels(); i++) { // For each pixel in strip...
      // Offset pixel hue by an amount to make one full revolution of the
      // color wheel (range of 65536) along the length of the strip
      // (strip.numPixels() steps):
      int pixelHue = firstPixelHue + (i * 65536L / pixels.numPixels());
      // strip.ColorHSV() can take 1 or 3 arguments: a hue (0 to 65535) or
      // optionally add saturation and value (brightness) (each 0 to 255).
      // Here we're using just the single-argument hue variant. The result
      // is passed through strip.gamma32() to provide 'truer' colors
      // before assigning to each pixel:
      pixels.setPixelColor(i, pixels.gamma32(pixels.ColorHSV(pixelHue)));
    }
    pixels.show(); // Update strip with new contents
    delay(wait);  // Pause for a moment
  }
}


// Rainbow-enhanced theater marquee. Pass delay time (in ms) between frames.
void theaterChaseRainbow(int wait) {
  int firstPixelHue = 0;     // First pixel starts at red (hue 0)
  for(int a=0; a<30; a++) {  // Repeat 30 times...
    for(int b=0; b<3; b++) { //  'b' counts from 0 to 2...
      pixels.clear();         //   Set all pixels in RAM to 0 (off)
      // 'c' counts up from 'b' to end of strip in increments of 3...
      for(int c=b; c<pixels.numPixels(); c += 3) {
        // hue of pixel 'c' is offset by an amount to make one full
        // revolution of the color wheel (range 65536) along the length
        // of the strip (strip.numPixels() steps):
        int      hue   = firstPixelHue + c * 65536L / pixels.numPixels();
        uint32_t color = pixels.gamma32(pixels.ColorHSV(hue)); // hue -> RGB
        pixels.setPixelColor(c, color); // Set pixel 'c' to value 'color'
      }
      pixels.show();                // Update strip with new contents
      delay(wait);                 // Pause for a moment
      firstPixelHue += 65536 / 90; // One cycle of color wheel over 90 frames
    }
  }
}
