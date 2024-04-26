
#include <WiFi.h>
#include <HTTPClient.h>
#include <time.h>
#include <ArduinoJson.h>

// time stuff
const char* ntpServer = "fi.pool.ntp.org";
const int daylight_offset_sec =  3600;
const int gmt_offset_sec = 2 * 3600;

const char* ssid     = "kuusipuu";
const char* password = "koivupuu";

int btnGPIO = 0;
int btnState = false;

// runtime stuff
String json_string;
struct tm timeinfo; //  järjestelmän aika bro!!!
int time_slot_arr[24*4]; //  kokoelma time_slotteja hinta järjestyksessä 15 min slotteja 0= 0:0 , 5 = 1:15 jne.
struct tm t_slot_array_date; // tarkistellaan että toimitaan oikean päivän mukaan
                            // fallback array?
bool on_off_arr[24][4];
int num_on_slots = 16 ; // default time to keep electricity on  15min*4 = 16 slots = 4h
int used_slots = 0;

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
void CreateOnOffArray(){
// array of 24*4 slots, where slot can be on or off;  
  // puhdistetaan ensin
  bool flat_on_off_arr[94];
  for (int i = 0 ; i<24*4; i++){
    flat_on_off_arr[i] = false;
  }

  for ( int i = 0; i < num_on_slots; i++){
    flat_on_off_arr[time_slot_arr[i]] = true;
  } // nyt on off sisältää true jokaisessa paikassa jossa haluaan olla päällä.

  // saatanan laiton looppi
  int flat_index = 0; // yksiuloteisen etsimis indeksi
  for(int i = 0; i<24; i++){
    for(int u =0; u <4; u++){
      on_off_arr[i][u] =flat_on_off_arr[flat_index];
      flat_index++;
    }
  }
}

void SetOnOff(){
  int cur_hour = timeinfo.tm_hour;
  int cur_minute  = timeinfo.tm_min;
  int vartti = cur_minute/15;
  if (on_off_arr[cur_hour][vartti] == true){
    // outputti piälle.

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
      //Serial.println(json_string);
      delay(1);
      parseJsonAndCalcOnHours(json_string);
      
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


void GetTimeFromNtp(){
  // time from public ntp server  -- tarviaa daylight saving homman jomman!
  configTime(gmt_offset_sec, daylight_offset_sec, ntpServer);
  struct tm timeinfo;
}

void printLocalTime(){

  if(!getLocalTime(&timeinfo)){
    Serial.println("No time available (yet)");
    return;
  }
  Serial.print(" järjestelmän aika");
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}




bool parseJsonAndCalcOnHours(String input){
  // returns true if ok and false if failed to parse or current date not pressent in the json
  // end result will end up in global timeslotarr

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, input);
  // vartti hinnoittelut tänne sisään eli array varttikohtaisia hintoja.
  // 24 tuntia ja 4 varttia jokaisessa
  float prices_today[24][4];

  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return false;
  }

  // ei virhettä
  int size = doc.size();

  bool found = false;
  for ( int i = 0; i< size; i++){
    const char* d = doc[i]["date"];
    struct tm parsed_date;
    time_t now;

    strptime(d, "%Y-%m-%d", &parsed_date);
    Serial.println(&timeinfo,  "%A, %B %d %Y %H:%M:%S");// nykyinen
    Serial.println(&parsed_date,  "%A, %B %d %Y %H:%M:%S"); //parsettu
    if (parsed_date.tm_year == timeinfo.tm_year &&
        parsed_date.tm_mon == timeinfo.tm_mon &&
        parsed_date.tm_mday == timeinfo.tm_mday) {
          found = true;
          t_slot_array_date = timeinfo; // asetetaan muuttuja joka kertoo minkä päivän tietoa soratussa arrayssa on
          Serial.println("The parsed date is today.");
          // tämän päivähn tedot tauluun kissa
          JsonObject tunnit = doc[i]["tunnit"];
          for (JsonPair kv : tunnit){
              String tmp = kv.key().c_str();
              int h = tmp.toInt();
              for (int i = 0; i< 4; i++){
                prices_today[h][i] = kv.value()[i];
              }
            }
            
          }

    else {
          //Serial.println("The parsed date is not today.\n");        
    }
  }
  if (!found) 
        { return false;}

  SortTimesAsc(prices_today ); 

  return true;
}



void SortTimesAsc(float prices[24][4]){
  Serial.println(prices[0][1]);
  // flaten the array do a pseudo map :D
  float value_arr[24*4];
  int flat_i =0;
  for (int i = 0; i < 4; i++){
    for(int u = 0; u < 24; u++){
      value_arr[flat_i] =  prices[i][u];
      flat_i++;
    }
  }
  // timeslot array populated with  matching indices
   for ( int i = 0 ; i < 24*4; i++){
    time_slot_arr[i] = i;
  }

  // insertion sort 
    insertionSort(value_arr, time_slot_arr, 94);
  
}

/* Function to sort an array using insertion sort*/
void insertionSort(float arr[], int timeslot_arr[], int n){ // nää on pass by ref vaikka en täysin käsitäkkään!!
  // sorts based on first arr  does transorms on both arrays.
  //Serial.println(arr[1]);
    float key, ind_key;
    int i, j;
  
    for (i = 1; i < n; i++) {
        key = arr[i];
        ind_key = timeslot_arr[i];
        j = i - 1;
        while (j >= 0 && arr[j] > key) {
            arr[j + 1] = arr[j];
            timeslot_arr[j + 1] = timeslot_arr[j];
            j = j - 1;
        }
        arr[j + 1] = key;
        timeslot_arr[j+1] = ind_key;
    }
}



