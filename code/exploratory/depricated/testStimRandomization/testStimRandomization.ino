const int NUM_TRIALS = 4;

int order[NUM_TRIALS];

void setup() {
  
  resetArr(order, sizeof(order)/sizeof(order[0])); // init
  randomSeed(analogRead(0)); // seed
  delay(4000);
  randomizeAndShiftOrder(); // scramble and shift
  Serial.println(sizeof(order)/sizeof(order[0]));
  delay(10000);
}

void loop() {
  
  Serial.println("new");
  resetArr(order, sizeof(order)/sizeof(order[0])); // init
  delay(2000);

  for(int i=0; i<NUM_TRIALS; i++){  
    Serial.print(order[i]);
  }
  
  
  randomSeed(analogRead(0)); // seed
  randomizeAndShiftOrder(); // scramble and shift

}

//method for resetting both arrays
void resetArr(int arr[], int arrS){
  for (int i = 0; i < arrS; i++){
    arr[i] = -1;
  }
}


void randomizeAndShiftOrder(){  
    Serial.println("method");

  for (int i = 0; i < NUM_TRIALS; i++){
    Serial.println("\nfor");

    //should be different each time because was randomly seeded
    int val = random(NUM_TRIALS);
    while(contains(val, order)){
      Serial.println("while");

      val = random(NUM_TRIALS);
      Serial.println(val);
      
      for(int i=0; i<NUM_TRIALS; i++){
        Serial.print(order[i]);
      }
      delay(3000);
    }
    order[i] = val;
  }

  /*
   * Finally, shift each value up by two, since 0 and 1 mean on and off. everything two and above means trial code.
   */
  for (int i=0; i < NUM_TRIALS; i++){
    order[i] = order[i] + 2;  // from 0, 1, 2, 3,... to 2, 3, 4, 5,...
  } 

  for(int i=0; i<NUM_TRIALS; i++){
    Serial.print(order[i]);
  }
}



bool contains(int val, int arr[]){
  for(int i = 0; i < NUM_TRIALS; i++){
    if(arr[i] == val){
      return true;
    }
  }
  return false;
}
