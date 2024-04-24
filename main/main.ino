
#include <WiFi.h>
#include <HTTPClient.h>
#include <time.h>
#include <ArduinoJson.h>

// time stuff
const char* ntpServer = "fi.pool.ntp.org";
const int daylight_offset_sec =  3600;
const int gmt_offset_sec = 3 * 3600;

const char* ssid     = "kuusipuu";
const char* password = "koivupuu";

int btnGPIO = 0;
int btnState = false;

// runtime stuff
String json_string;

void setup()
{

    Serial.begin(115200);
    delay(10);

    // Set GPIO0 Boot button as input
    pinMode(btnGPIO, INPUT);

    // We start by connecting to a WiFi network
    // To debug, please enable Core Debug Level to Verbose

    connectWifi();

}
void connectWifi(){
    Serial.print("[WiFi] Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);
// Auto reconnect is set true as default
// To set auto connect off, use the following function
// WiFi.setAutoReconnect(false)

    // Will try for about 10 seconds (20x 500ms)
    int tryDelay = 500;
    int numberOfTries = 20;

    // Wait for the WiFi event
    while (true) {
        
        switch(WiFi.status()) {
          case WL_NO_SSID_AVAIL:
            Serial.println("[WiFi] SSID not found");
            break;
          case WL_CONNECT_FAILED:
            Serial.print("[WiFi] Failed - WiFi not connected! Reason: ");
            return;
            break;
          case WL_CONNECTION_LOST:
            Serial.println("[WiFi] Connection was lost");
            break;
          case WL_SCAN_COMPLETED:
            Serial.println("[WiFi] Scan is completed");
            break;
          case WL_DISCONNECTED:
            Serial.println("[WiFi] WiFi is disconnected");
            break;
          case WL_CONNECTED:
            Serial.println("[WiFi] WiFi is connected!");
            Serial.print("[WiFi] IP address: ");
            Serial.println(WiFi.localIP());

            getJsonFromServer();
            GetTimeFromNtp();
            return;
            break;
          default:
            Serial.print("[WiFi] WiFi Status: ");
            Serial.println(WiFi.status());
            break;
        }
        delay(tryDelay);
        
        if(numberOfTries <= 0){
          Serial.print("[WiFi] Failed to connect to WiFi!");
          // Use disconnect function to force stop trying to connect
          WiFi.disconnect();
          return;
        } else {
          numberOfTries--;
        }
    }
}


void loop()
{
    // Read the button state
    btnState = digitalRead(btnGPIO);
    
    if (btnState == LOW) {

      GetTimeFromNtp();
      delay(1);
      printLocalTime();
      json_string = getJsonFromServer();
      Serial.println(json_string);
      delay(1);
      parseJsonAndCalcOnHours(json_string);
      // Disconnect from WiFi
      //Serial.println("[WiFi] Disconnecting from WiFi!");
      // This function will disconnect and turn off the WiFi (NVS WiFi data is kept)
      //if(WiFi.disconnect(true, false)){
      //  Serial.println("[WiFi] Disconnected from WiFi!");
      //}
      
    }    
}

String getJsonFromServer(){
//hakee päivähinnat jsonin  
  HTTPClient http;
  String payload;

  Serial.println("[HTTP] begin...\n");
  http.begin("http://pvauq.eu/dayprices.json"); //HTTP

  Serial.println("[HTTP] GET...\n");
  // start connection and send HTTP header
  int httpCode = http.GET();

  // httpCode will be negative on error
  if(httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.println("[HTTP] GET... code: %d\n"+ httpCode);

      // file found at server
      if(httpCode == HTTP_CODE_OK) {
          payload = http.getString();
          Serial.println( http.errorToString(httpCode).c_str());
          http.end();
          
      }
  } else {
      Serial.println( http.errorToString(httpCode).c_str());
      http.end();
  }
  return payload;
}


// time from public ntp server
void GetTimeFromNtp(){
  configTime(gmt_offset_sec, daylight_offset_sec, ntpServer);
  struct tm timeinfo;
}


void printLocalTime()
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("No time available (yet)");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}


bool parseJsonAndCalcOnHours(String input){
  JsonDocument doc;

  DeserializationError error = deserializeJson(doc, input);

  if (error) {
    Serial.print("deserializeJson() failed: ");
    //Serial.println(error.c_str());
    return false;
  }

  const char* date = doc[0]["date"]; // "2024-04-24"
  Serial.println(date);
  String eg =  doc[0]["tunnit"]["1"][1];


  // chekataan et tänään on tää päivä ennenkuin mennään laittamaan listalle.!

// vartti hinnoittelut tänne sisään eli array varttikohtaisia hintoja.
// 24 tuntia ja 4 varttia jokaisessa
  float prices_today[24][4];

  JsonObject tunnit = doc[0]["tunnit"];
  for (JsonPair kv : tunnit){
      String tmp = kv.key().c_str();
      int h = tmp.toInt();
      for (int i = 0; i< 4; i++){
        prices_today[h][i] = kv.value()[i];
      }
    }
  Serial.println(prices_today[1][2]);
  
  //Serial.println(prices_today[1][2]);
  return true;
}

