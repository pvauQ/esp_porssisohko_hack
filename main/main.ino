
#include <WiFi.h>
#include <HTTPClient.h>
#include <time.h>
#include <ArduinoJson.h>


// time stuff
const char* ntpServer = "fi.pool.ntp.org";
const int daylight_offset_sec =  3600;
const int gmt_offset_sec = 2 * 3600;

char* api_url = "http://pvauq.eu/dayprices.json";
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

uint tmp_timer = 0; // this just temp, remove when we have non blocking timer

void setup()
{

    Serial.begin(115200);
    delay(10);

    // Set GPIO0 Boot button as input
    pinMode(btnGPIO, INPUT);
    pinMode(4, OUTPUT);
    digitalWrite(4, LOW);


    // We start by connecting to a WiFi network
    // To debug, please enable Core Debug Level to Verbose

    connectWifi();

}



void loop()
{
    // Read the button state
    btnState = digitalRead(btnGPIO);
    
    if (btnState == LOW) {
      //:TODO tähän logiikka jolla varmistellaan että ollaan wiifissä

      GetTimeFromNtp();
      delay(5);
      printLocalTime();
      json_string = getJsonFromServer(api_url);
      delay(5);
      if (parseJsonAndCalcOnHours(json_string)){ // eli jsoni parseentui nätisti ja kova oletus että kaikki alafunkkarit toimi
        CreateOnOffArray(time_slot_arr, on_off_arr, num_on_slots); // 1d time_slotit ja 2d on off array

      }
    }
    delay(10);
    tmp_timer++;
    if (tmp_timer > 6000) {
      SetOnOff();
      tmp_timer = 0;
    }   
}

void SetOnOff(){
  int cur_hour = timeinfo.tm_hour;
  int cur_minute  = timeinfo.tm_min;
  int vartti = cur_minute/15;
  if (on_off_arr[cur_hour][vartti] == true){
    digitalWrite(4,HIGH);
  }
  else{
    digitalWrite(4,LOW);
  }
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

  SortTimesAsc(prices_today, time_slot_arr ); 

  return true;
}






