#include <"Keyboard.h">
/*
 * trial parameters
 */
const int ONTIME = 5000;
const int NUM_TRIALS = 5;
/*
 * stim delivery parameters
 */
const int NUM_ASSAYS = 1;//4; // number of assays I have
const int NUM_COLORS = 4; // number of colors I have

const int RED_PIN=11, GREEN_PIN=9, BLUE_PIN=10, WHITE_PIN=5;
const int colors[NUM_COLORS] = {RED_PIN, GREEN_PIN, BLUE_PIN, WHITE_PIN}; // original
int curr_colors[NUM_COLORS]; // to scramble

int order[NUM_ASSAYS];


// Button!
const int BUTTON=12;

// Pin for sending data to board
const int SIGNAL_PIN = 2;

void setup() {
  Serial.begin(9600);

  pinMode(BUTTON, INPUT);
  pinMode(SIGNAL_PIN, OUTPUT);
  
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT); pinMode(BLUE_PIN, OUTPUT); pinMode(WHITE_PIN, OUTPUT);
  
  resetArr(curr_colors, NUM_COLORS);
  
  // set up order of stimuli
  resetArr(order, NUM_ASSAYS); // init
  randomSeed(analogRead(0)); // seed
  randomizeAndShiftOrder(); // scramble and shift
}

void loop() {
  if (digitalRead(BUTTON)==HIGH){
    activateMontage();
    resetArr(order, NUM_ASSAYS); // ready to go again
    randomizeAndShiftOrder();    
  }
}

/*
 * Randomize a series of trials and run them
 */
void activateMontage(){
  for (int i=0; i<NUM_ASSAYS; i++){
    runTrials(order[i]);
    mdelay(500); // just give some time between events
  }
}

/*
 * Start trial
 */
void runTrials(int assay){  
  randomizeColors();
  printColors();

  switch (assay) {
    /*
     * Run CFF
     */
    case 3:
      //run...
      for (int i=0; i < NUM_COLORS; i++){
        sendMessage(assay, curr_colors[i]);
        CFFAssay(curr_colors[i]);
      }
      //reset
      resetArr(curr_colors, NUM_COLORS);
      break;
    /*
     * Run CDA
     */
    case 4:
      //run...
      for (int i=0; i < NUM_COLORS; i++){
        sendMessage(assay, curr_colors[i]); 
        colorDecayAssay(curr_colors[i]);
      }
      //reset
      resetArr(curr_colors,NUM_COLORS);
      break;
    /*
     * Run adaptation assay
     */
    case 5:
      //run...
      for (int i=0; i < NUM_COLORS; i++){
        sendMessage(assay, curr_colors[i]);
        colorAdaptationAssay(curr_colors[i]);
      }
      //reset
      resetArr(curr_colors, NUM_COLORS);
      break;
    /*
     * Run light/dark adaptation (I DONT KNOW THIS YET)
     */
    case 6:
      //run...
      for (int i=0; i < NUM_COLORS; i++){
        sendMessage(assay, curr_colors[i]);
        lightDarkAdaptation(curr_colors[i]);
      }
      //reset
      resetArr(curr_colors, NUM_COLORS);
      break;
    //-----------------------------------------------------------------------------------------------------------------------------     
    default: 
      // statements
    break; 
  }
}

/* Critical flicker frequency
 * Present randomized frequencies
 */
void CFFAssay(int pin){
  // replications
  for (int i = 0; i < 3; i++){
    for (int j = 1; j <=10; j++){ // increase Hz
      int period = 1000 * pow(0.5, j);
      
      // look @ literature for end Hz. Start at 1 Hz.
      digitalWrite(pin, HIGH);
      Keyboard.write('1');
      mdelay(period);
      
      digitalWrite(pin, LOW);
      Keyboard.write()
      mdelay(period);

    }
  }
}


// sees relative response of color
void colorDecayAssay(int pin){
  for (int i = 0; i < NUM_TRIALS; i++){
    analogWrite(pin, floor(255 * i / NUM_TRIALS));
    sendSig(1);
    mdelay(ONTIME);

    digitalWrite(pin, LOW);
    sendSig(1);
    mdelay(ONTIME);
  }
}

void colorAdaptationAssay(int pin){
  for (int i = 0; i < NUM_TRIALS; i++){
    digitalWrite(pin, HIGH);
    sendSig(1);
    mdelay(ONTIME);
    
    digitalWrite(pin,LOW);
    sendSig(1);
    mdelay(ONTIME);
  }
}

void lightDarkAdaptation(int pin){}

void mdelay(int msInterval){
  long prev = millis();
  while (millis() - prev < msInterval){/* wait!*/}
}
void udelay(int usInterval){
  long prev = micros();
  while (micros() - prev < usInterval){/* wait!*/}
}

bool contains(int val, int arr[], int arrS){
  for(int i = 0; i < arrS; i++){
    if(arr[i] == val){
      return true;
    }
  }
  return false;
}


//method for resetting both arrays
void resetArr(int arr[], int arrS){
  for (int i = 0; i < arrS; i++){
    arr[i] = -1;
  }
}

/*
 * initialize random order for tests and colors
 */
void randomizeAndShiftOrder(){  
  for (int i = 0; i < NUM_ASSAYS; i++){
    //should be different each time because was randomly seeded
    int val = random(NUM_ASSAYS);
    while(contains(val, order, NUM_ASSAYS)){
      val = random(NUM_ASSAYS);
    }
    order[i] = val;
    Serial.println(val);
    Serial.println(order[i]);
  } 
  
  // Finally, shift each value up by two, since 0 and 1 mean on and off. everything two and above means trial code.
  for (int i=0; i < NUM_ASSAYS; i++){
    order[i] = order[i] + 3;  // from 0, 1, 2, 3,... to 2, 3, 4, 5,...
  } 
}

void randomizeColors(){
  // randomizes pin numbers
  for (int i = 0; i < NUM_COLORS; i++){
    
    //should be different each time because was randomly seeded
    int val = colors[random(NUM_COLORS)]; // gets the pin number, not index
    
    while(contains(val, curr_colors, NUM_COLORS)){
      val = colors[random(NUM_COLORS)];
    }
    curr_colors[i] = val;
  }
}

void printColors(){
  for (int i=0; i < NUM_COLORS; i++){Serial.print(curr_colors[i]);}
}

void sendSig(int code){
  // send one aspect of assay id or on/off data
  for (int i = 0; i < code; i++){
    // 20 ms period
    Keyboard.write(char(code));
    mdelay(100);
  }
  mdelay(200);
}

void sendMessage(int assay, int color){
  //params: assay is just the assay value from case switch statement above, the color is the pin being sent.
  //send entire assay id
  sendSig(assay);

  switch(color){
    case 5:  // R
      sendSig(6);
      break;
    case 9:  // G
      sendSig(7);
      break;
    case 10:  // B
      sendSig(8);
      break;
    case 11:  // W
      sendSig(9);
      break;
    default:
      break;
  }
}
