const int NUM_TRIALS = 4; // number of assays I have
const int NUM_COLORS = 5; // number of colors I have

const int RED_PIN=9, GREEN_PIN=10, BLUE_PIN=11, IR_PIN=5, UV_PIN=6;
const int colors[NUM_COLORS] = {RED_PIN, GREEN_PIN, BLUE_PIN, IR_PIN, UV_PIN}; // original
int curr_colors[NUM_COLORS]; // to scramble
void setup() {
  // put your setup code here, to run once:
  delay(4000);
  resetArr(curr_colors, NUM_COLORS);
  randomizeColors();

}

void loop() {
  // put your main code here, to run repeatedly:
  for(int i=0; i<NUM_COLORS; i++){
    Serial.print(curr_colors[i]);
  }
  
  resetArr(curr_colors, NUM_COLORS); // init
  randomizeColors();
  for(int i =0; i < NUM_COLORS; i++){
    digitalWrite(colors[i], HIGH);
    delay(200);
    digitalWrite(colors[i], LOW);
  }
}

void randomizeColors(){
  // randomizes pin numbers
  for (int i = 0; i < NUM_COLORS; i++){
    Serial.println("for");
    
    //should be different each time because was randomly seeded
    int val = colors[random(NUM_COLORS)]; // gets the pin number, not index
    while(contains(val, curr_colors)){
      Serial.println("while");
      
      val = colors[random(NUM_COLORS)];
//      delay(1000);
      for(int i=0; i<NUM_COLORS; i++){
        Serial.print(curr_colors[i]);
      }
      Serial.println();
    }
    curr_colors[i] = val;
    
  }
}

void resetArr(int arr[], int arrS){
  for (int i = 0; i < arrS; i++){
    arr[i] = -1;
  }
}

bool contains(int val, int arr[]){
  for(int i = 0; i < NUM_COLORS; i++){
    if(arr[i] == val){
      return true;
    }
  }
  return false;
}
