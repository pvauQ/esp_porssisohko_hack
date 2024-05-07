/* in this file we do all off the wifi communication stuff including 
connection and actually getting all the data
*/

 String getJsonFromServer(char* url){
//hakee päivähinnat jsonin  
  HTTPClient http;
  String payload;

  Serial.println("[HTTP] begin...\n");
  http.begin(url); //HTTP

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




bool connectWifi(){
  // does lot of retries, and return true after connected to network,
  // returns false if failed to connect
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
            return false;
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

            return true;

            //getJsonFromServer();
            //GetTimeFromNtp();
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
          return false;
        } else {
          numberOfTries--;
        }
    }
  return false;
}