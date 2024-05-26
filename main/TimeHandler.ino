/* in this file we define stuff needed to calc what timeslots are on.
 day is  divided to 24*4 timeslots, as electricy pricing will be based on 15 min intervals strarting on 2025
 */

void printLocalTime(){

  if(!getLocalTime(&timeinfo)){
    Serial.println("No time available (yet)");
    return;
  }
  Serial.print(" järjestelmän aika");
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}



void SortTimesAsc(float prices[24][4], int time_slot_arr[24*4] ){ //  nää on pass by reff eli pitäis olla ihan fine
  /* does flat copy of price array and uses it to sort time slot aray  in asceding order
  so that cheapess time slots are i the beginng of this flat time slot arr
  */

  // flaten the array do a pseudo map price - index  in timeslot array
  float value_arr[24*4];
  int flat_i =0;
  for (int i = 0; i < 24; i++){
    for(int u = 0; u < 4; u++){
      value_arr[flat_i] =  prices[i][u];
      flat_i++;
    }
    //Serial.print("tunnilla:");Serial.print(i); Serial.print("hinta:");Serial.println(value_arr[flat_i - 1],4);
  }
  // timeslot array populated with  matching indices
   for ( int i = 0 ; i < 24*4; i++){
    time_slot_arr[i] = i;
  }

  // insertion sort, based on vall arrays but we are interested  in time_slot_arr
    insertionSort(value_arr, time_slot_arr, 94);
  
}



void insertionSort(float arr[], int timeslot_arr[], int n){ // nää on pass by ref vaikka en täysin käsitäkkään!!
  // sorts based on first arr  does transorms on both arrays.
  // ie. sort based on price array and do transforms on "array that contains corresponding timeslots"
  // so if we need to have output on for 8 timeslots per day we can choose 8 first from timelost array and those are the cheapest
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
    /*
    for (i = 1; i < n; i++) {
      Serial.println(arr[i]);
    } */
}

void CreateOnOffArray(int  time_slot_arr[24*4], bool on_off_arr[24][4], int num_on_slots ){
// array of 24*4 slots, where slot can be on or off;  
// 
// first we clean the array to have false in each slot surely there is c way to this :D
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
    //Serial.print( "tunilla"); Serial.print(i); Serial.print(" on / off : "); Serial.println(on_off_arr[i][1]);
  }
}


void createDefaultOnOffArray(bool on_off_arr[24][4], int num_slots){
  // tässä on sellainen bugi, että  pyöristää alempaan tuntiin jos num slots  ei ole jaollinen 4
  int def_hours_arr[24] = {5,4,3,2,1,0,15,14,13,12,16,17,18,6,23,19,20,21,22,7,11,10,9,8};

  bool local_arr[24][4];

  for ( int i =0; i<num_slots/4; i++){
    for( int u = 0; u<4; u++){
      local_arr[def_hours_arr[i]][u] =true;
    }
    Serial.print( "päällä : tunilla "); Serial.println(def_hours_arr[i]);
  }
  on_off_arr= local_arr;
  //delay(10);

}