/**import library**/
#include "DHT.h"
#include <WiFi.h>
#include <WebServer.h>
#include "ArduinoJson.h" 

#define DHTPIN 33     // Digital pin connected to the DHT sensor
#define LEDPIN 26     //
#define BUTTONPIN 34 


#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

DHT dht(DHTPIN, DHTTYPE);

bool LEDSTATE = false;
bool BUTTONSTATE;

const char* ssid = "isissid";
const char* password = "isipasswordnya";

WebServer server(80);

float temperature;
float humidity;

//http header
String header;

// Auxiliar variables to store the current output state
String output26State = "OFF";

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;


unsigned long gap; 

StaticJsonDocument<250> jsonDocument;
char buffer[250];

//pembuat json tipe float
void create_json(char *tag, float value, char *unit) {  
  jsonDocument.clear();  
  jsonDocument["type"] = tag;
  jsonDocument["value"] = value;
  jsonDocument["unit"] = unit;
  serializeJson(jsonDocument, buffer);
}
//pembuat json tipe string
void create_json2(char *tag, String value, char *unit) {  
  jsonDocument.clear();  
  jsonDocument["type"] = tag;
  jsonDocument["value"] = value;
  jsonDocument["unit"] = unit;
  serializeJson(jsonDocument, buffer);
}

void create_json3(char *tag, float suhu, float lembab, String led) {  
  jsonDocument.clear();  
  jsonDocument["type"] = tag;
  jsonDocument["suhu"] = suhu;
  jsonDocument["lembab"] = lembab;
  jsonDocument["led"] = led;
  serializeJson(jsonDocument, buffer);
}

//GET pembacaan temperatur
void getTemperature() {
//  Serial.println("Get temperature");
  create_json("temperature", temperature, "°C");
  server.send(200, "application/json", buffer);
}
//GET pembacaan kelembapan
void getHumidity() {
//  Serial.println("Get humidity");
  create_json("humidity", humidity, "%");
  server.send(200, "application/json", buffer);
}
//GET pembacaan status LED
void getLed() {
//  Serial.println("Get led");
  create_json2("led", output26State , "");
  server.send(200, "application/json", buffer);
}

//GET pembacaan dari ESP32
void getAll() {
//  Serial.println("Get all");
  create_json3("all", temperature, humidity, output26State);
  server.send(200, "application/json", buffer);
}

//fungsi baca sensor DHT
void readData() {
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();
}

//String html = "<html><body><h1>Tugas 2 - SBM</h1><p>Temperature: <span id='tem'>"+ String(temperature) +"</span></p><p>Humidity: <span id='hum'>"+ String(humidity) +"</span></p><p>LED: <span id='led'>"+ String(output26State) +"</span></p><button><a href='/ledtoggle' id='toggle'>Toggle</a></button><script type='text/javascript' defer src='/script1'></script></body></html>";
 String html;
String script1 = "document.addEventListener('DOMContentLoaded', () => {console.log('loaded');const led = document.getElementById('toggle'); led ? led.addEventListener('click', (e) => { e.preventDefault(); fetch('/ledtoggle');}) : null;}); setInterval(() => {fetch('/temperature').then(res => res.json()).then((data) => {document.getElementById('tem').innerHTML = `${data.value}`;}); fetch('/humidity').then(res => res.json()).then((data) => {document.getElementById('hum').innerHTML = `${data.value}`;}); fetch('/led').then(res => res.json()).then((data) => {document.getElementById('led').innerHTML = `${data.value}`;}); }, 2000);";

//GET html
void getUi() {
//  Serial.println("Get ui");
  server.send(200, "text/html", html); 
}

//GET script js
void getScript1() {
//  Serial.println("Get script1");
  server.send(200, "text/javascript", script1);
}

//GET untuk toggle LED
void getLedToggle(){
  digitalWrite(LEDPIN, !LEDSTATE);
  LEDSTATE = !LEDSTATE;
  if(LEDSTATE){
    output26State = "ON";
  } else{
    output26State = "OFF";
  }
  server.send(200,"application/json", "{'status': 'ok'}");
  delay(500); //simple debouncer 
}

int incoming; 

void setup() {
  Serial.begin(115200);
  pinMode(LEDPIN, OUTPUT);
  digitalWrite(LEDPIN, LOW);
  pinMode(BUTTONPIN, INPUT);
  dht.begin();

  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
  //route untuk mendapat data temperatur
  server.on("/temperature", getTemperature);
  //route untuk mendapat data kelembapan        
  server.on("/humidity", getHumidity);   
  //route untuk mendapat status nyala LED
  server.on("/led", getLed);
  
  //route untuk men-toggle LED
  server.on("/ledtoggle", getLedToggle);
  
  //route untuk menampilkan halaman html
  server.on("/tugas2", getUi); 
  server.on("/script1", getScript1);
  
  server.begin();
}

void loop() {
  server.handleClient();
  BUTTONSTATE = digitalRead(BUTTONPIN);
  if(BUTTONSTATE){
    LEDSTATE = !LEDSTATE;
    digitalWrite(LEDPIN, LEDSTATE);
    delay(500);
    if (LEDSTATE) {
      output26State = "ON";
    } else {
      output26State = "OFF";
    }
  }

  if (Serial.available() > 0) {
    // read the incoming byte:
    incoming = Serial.read();
    if (incoming == 49 || incoming == 4910){
      LEDSTATE =!LEDSTATE;
      digitalWrite(LEDPIN, LEDSTATE);
      delay(10);
      if (LEDSTATE) {
        output26State = "ON";
      } else {
        output26State = "OFF";
      }
    }
  }
   
  gap = millis();
  //update data dht pada periode tertentu  
  if(!(gap%500)){
    readData();
    Serial.print(F("Humidity: "));
    Serial.print(humidity);
    Serial.print(F("%  Temperature: "));
    Serial.print(temperature);
    Serial.print(F("°C LED: "));
    Serial.print(output26State);
    Serial.print(F("\n"));
    html = "<html><body><h1>Tugas 2 - SBM</h1><p>Temperature: "+ String(temperature) +"</p><p>Humidity: "+ String(humidity) +"</p><p>LED: "+ String(output26State) +"</p><button><a href='/ledtoggle' id='toggle'>Toggle</a></button><script type='text/javascript' defer src='/script1'></script></body></html>";
   
    if (isnan(humidity) || isnan(temperature)) {
      Serial.println(F("Failed to read from DHT sensor!"));
      return;
    }
  } 
}
