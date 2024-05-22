
#include <WiFi.h>
#include <HTTPClient.h>
#include <time.h>
#include <ArduinoJson.h>

// insert ur stuff here
char* api_url = "INSERT HERE";
const char* ssid     = "INSERT HERE"; 
const char* password = "INSERT HERE";

int btnGPIO = 0;
int btnState = false;
int ohitus_btn_state = false;

// runtime stuff
String json_string;
struct tm timeinfo; //  järjestelmän aika bro!!!
int time_slot_arr[24*4]; //  kokoelma time_slotteja hinta järjestyksessä 15 min slotteja 0= 0:0 , 5 = 1:15 jne.
struct tm t_slot_array_date; // tarkistellaan että toimitaan oikean päivän mukaan
                            // fallback array?
bool on_off_arr[24][4];
int num_on_slots = 20 ; // default time to keep electricity on  15min*5 = 20 slots = 5h
int used_slots = 0;
bool need_new_data = true;
uint on_off_counter = 0; // this just temp, remove when we have non blocking timer
uint main_upp_counter =0;
bool ohitetaan = false; // onko koko paska käytössä vai ohitetaanko !!



void setup()
{
    pinMode(btnGPIO, INPUT); //  esp boardilla oleva buttoni
    pinMode(4, OUTPUT); // kontaktori

    //status ledit
    pinMode(14, OUTPUT);
    pinMode(27, OUTPUT);
    pinMode(26, OUTPUT);
    pinMode(25, OUTPUT);
    // nabbulat
    pinMode(18, INPUT_PULLUP);
    //pinMode(19, INPUT);
    pinMode(22, INPUT_PULLUP);
    pinMode(23, INPUT_PULLUP);



    Serial.begin(115200);
    digitalWrite(4, LOW); //
    doNetworkStuff();
    mainUpdate();

    // status

}

void loop()
{
    digitalWrite(25, ohitetaan);// kirjoitetaan viimoiseen lediin ohituksen tila.


    btnState = digitalRead(btnGPIO);
    ohitus_btn_state = digitalRead(22);

    
    if ( digitalRead(23) == 0){
      delay(300); // köyhän miehen debounce :D
      toogleOhitus();
    }
    if (btnState == LOW || digitalRead(18) == 0) {// pakko update
      mainUpdate();
    }

    delay(10);
    on_off_counter++;
    if (on_off_counter > 6000) { // tämä ois kiva olla jollain kivalla non blokkaavalla,  ehkä seuraavalla kerralla...
      SetOnOff();
      on_off_counter =0;
      
      main_upp_counter++;
      if (main_upp_counter >10){ // käydään netissä ja tehrään kaikki kiva.
        main_upp_counter= 0;
        mainUpdate(); // tämän voisi ajaa paljon harvemminkin( kerran vuorokaudessa). toisaalta näin on aika varmaa että jossain vaiheessa saadaan yhteys ja homma hoituu
      }

      
    }   
}

bool toogleOhitus(){
  ohitetaan = !ohitetaan;
  SetOnOff();
  Serial.print(" ohitus: ");
  Serial.println(ohitetaan);
  return ohitetaan;

  

}


bool doNetworkStuff(){
    bool connection_ok;
    connection_ok = connectWifi();

    if (connection_ok){
      initTime();
      //GetTimeFromNtp();
      delay(5);
      printLocalTime();
      json_string = getJsonFromServer(api_url);
      return true;
    }
    else {return false;}
}

void  mainUpdate(){

    bool network_success = doNetworkStuff();
    bool parse_success =  parseJsonAndCalcOnHours(json_string);
    if(network_success & parse_success){
      //  this should be the default
      CreateOnOffArray(time_slot_arr, on_off_arr, num_on_slots);
      digitalWrite(27, HIGH);
      digitalWrite(26, HIGH);
    }
    else if (!network_success & parse_success){
      // ei  nettiä, mutta vanhasta json stringistä saatiin tämän päivän hinnat :)
      CreateOnOffArray(time_slot_arr, on_off_arr, num_on_slots);
      digitalWrite(27, LOW);
      digitalWrite(26, HIGH);

    }
    else{
      //  kaikki feilaa 
      // täytyy käyttää default tunteja.
      createDefaultOnOffArray(on_off_arr, num_on_slots);
      digitalWrite(27, LOW);
      digitalWrite(26, LOW);
    }

}



void SetOnOff(){
  int cur_hour = timeinfo.tm_hour;
  int cur_minute  = timeinfo.tm_min;
  int vartti = cur_minute/15;
  if (ohitetaan){
    digitalWrite(4,HIGH);
    digitalWrite(14, HIGH);
  }
  else if (on_off_arr[cur_hour][vartti] == true){
    digitalWrite(4,HIGH);
    digitalWrite(14, HIGH); // LEDI
  }
  else{
    digitalWrite(4,LOW);
    digitalWrite(14, LOW);
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






