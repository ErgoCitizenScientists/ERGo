/**
Author: DJP
To test whether the Arduino can send light pulses in a temporall precise fashion,
I'll analyze the delivery train in two different ways, one using the little helper markers 
I have, the other using my knowledge of how long it should take (interpolation). If interpo-
lation doesn't work 
*/
const int NUM_ASSAYS = 1; // number of assays to run, i.e., CFF (1), CFF and CDA (2), CFF and CDA and ... (3)
const int NUM_COLORS = 1; // number of colors to send, i.e., CFF (1), CFF and CDA (2), CFF and CDA and ... (3)

const int RED_PIN=9, GREEN_PIN=10, BLUE_PIN=11, IR_PIN=5, UV_PIN=6;

const int colors[NUM_COLORS] = {RED_PIN};//, GREEN_PIN, BLUE_PIN, IR_PIN, UV_PIN}; // original
int currColors[NUM_COLORS]; // to scramble


/*
 * TRIAL PARAMETERS
 */
const int ONTIME = 2000; // duration of a CFF trial. So for 1 Hz, 8 Hz, etc., there are a different amount of light deliveries, but they will take approximately the same amount of time.
const int NUM_TRIALS = 4; // number of repetitions for each assay for each color

int order[NUM_ASSAYS];

const int BTN1=3;
const int BTN2=2;

// Pin for sending data to board
const int CFF_SIGNAL_PIN = 4; //2; WHOOOO IT'S 4 FOR THE NEW BOARD!!!; sends '2' for cff information
const int ONOFF_PIN = 8;//5; // for sending messages of on/off, sends '3' for on/off
const int ASSAY_SIGNAL_PIN = 7;//6; // sends '4' mark for saying which non-cff assay it is 
void setup() {
  Serial.begin(9600);

  pinMode(BTN1, INPUT);
  pinMode(BTN2, INPUT);
  pinMode(CFF_SIGNAL_PIN, OUTPUT);
  pinMode(ONOFF_PIN, OUTPUT);
  pinMode(ASSAY_SIGNAL_PIN, OUTPUT);
  
  pinMode(RED_PIN, OUTPUT);

  digitalWrite(RED_PIN, HIGH);
 
  // initialize to -1 for each slot
  resetArr(currColors, NUM_COLORS);
  
  // set up order of stimuli
  resetArr(order, NUM_ASSAYS); // init
  randomSeed(analogRead(0)); // seed
  randomizeAndShiftOrder(); // scramble and shift
}

void loop() {
  if (digitalRead(BTN1)==HIGH){
    CFFAssay2D(0);
    resetArr(order, NUM_ASSAYS); // ready to go again
    randomizeAndShiftOrder();
  }
}

void sendAssayTypeSig(int code){
  // send one aspect of assay id or on/off data
  for (int i = 0; i < code; i++){
    digitalWrite(ASSAY_SIGNAL_PIN, LOW);
    mdelay(20);
    digitalWrite(ASSAY_SIGNAL_PIN, HIGH);
    mdelay(20);
  }
  mdelay(100);
}

void sendSigCFF(int code){
  // Send ON/OFF signal
  for (int i = 0; i < code; i++){
    digitalWrite(CFF_SIGNAL_PIN, LOW);
    mdelay(20);
    digitalWrite(CFF_SIGNAL_PIN, HIGH);
    mdelay(20);
  }
  mdelay(100);
}

/* Critical flicker frequency, iterating a 2D array
 * Present all possible combinations of frequencies and colors. 
 * Because these are two dimensions of randomization, the way I represent the 
 * combinations are with a 2D array (squashed into 1D). That's why it's called CFF Assay2D
 */
void CFFAssay2D(bool markers){
  
  int freqs[] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512};
  const int NUM_FREQS = sizeof(freqs)/sizeof(freqs[0]);
  
  // initialize random order
  int CFFOrder[NUM_FREQS*NUM_COLORS];
  for (int i = 0; i < NUM_FREQS*NUM_COLORS; i++){CFFOrder[i] = -1;}
  
  // Randomizes the CFFOrder array by treating it like a globally-scoped variable
  randomize2D(CFFOrder, NUM_FREQS, NUM_COLORS);
  
  // run stimuli
  for (int i = 0; i < NUM_FREQS * NUM_COLORS; i++){
    // get current frequency and color
    int freqIndex= getFreq(CFFOrder, NUM_FREQS, NUM_COLORS, i);
    int currFreq = freqs[freqIndex];

    int pinIndex=getColor(CFFOrder, NUM_FREQS, NUM_COLORS, i);
    int pin = colors[pinIndex];

    // send signal
    sendSigCFF(freqIndex + 2); // need to have at least two to have the right interval
    delay(100);
    sendSigCFF(pin); // corresponds to a pin in the array of led pins
    
    int period = float(1000 / currFreq / 2 );
    unsigned long startTime = millis();

    delay(20); // another little wait
    
    // if no markers,
    if (!markers){digitalWrite(ONOFF_PIN, HIGH);}
    
    while (millis() - startTime < ONTIME ){
      
      digitalWrite(pin, LOW);
      // if markers, 
      if(markers){digitalWrite(ONOFF_PIN, HIGH);}
      mdelay(period);

      digitalWrite(pin, HIGH);
      // if markers,
      if(markers){digitalWrite(ONOFF_PIN, LOW);}

      mdelay(period); 
    }
    // if no markers
    if (!markers){digitalWrite(ONOFF_PIN, LOW);}
  }
}

void randomize2D(int arr[], int arrS1, int arrS2){
  for (int i = 0; i < arrS1; i++){
    for (int j = 0; j < arrS2; j++){
      int val = random(arrS1*arrS2);
      while(contains2D(val, arr, arrS1, arrS2)){val = random(arrS1*arrS2);}
      arr[i * arrS2 + j] = val;
    }
  }
}

bool contains2D(int val, int arr[], int arrS1, int arrS2){
  for(int i = 0; i < arrS1; i++){
    for (int j = 0; j < arrS2; j++){
      if(arr[i*arrS2+j] == val){return true;}
    }
  }
  return false;
}

int getFreq(int arr[], int arrS1, int arrS2, int index){
  for (int i = 0; i < arrS1; i++){
    for (int j = 0; j < arrS2; j++){
      if (arr[i*arrS2+j] == index){
        return i;
      }
    }
  }
  Serial.print("error in getFreq");
  return -1;
}

int getColor(int arr[], int arrS1, int arrS2, int index){
  for (int i = 0; i < arrS1; i++){
    for (int j = 0; j < arrS2; j++){
      if (arr[i*arrS2+j] == index){
        return j;
      }
    }
  }
  Serial.print("error in getColor");
  return -1;
}

void mdelay(int msInterval){
  // for intervals MORE than 1.953 ms
  // for pulse frequencies LESS than 512 Hz
  long prev = millis();
  while (millis() - prev < msInterval){/* wait!*/}
}

void udelay(int usInterval){
  // for intervals LESS than 1.953 ms
  // for pulse frequencies MORE than 512 Hz
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
    while(contains(val, order, NUM_ASSAYS)){val = random(NUM_ASSAYS);}
    order[i] = val;
  }
  
  // Finally, shift each value up by 1 so that you can send it in a message
  for (int i=0; i < NUM_ASSAYS; i++){
    order[i] = order[i] + 1;  // from 0, 1, 2, 3,... to 1, 2, 3, 4
  } 
}

void randomizeColors(){
  // randomizes pin numbers
  for (int i = 0; i < NUM_COLORS; i++){
    
    //should be different each time because was randomly seeded
    int val = colors[random(NUM_COLORS)]; // gets the pin number, not index
    
    while(contains(val, currColors, NUM_COLORS)){
      val = colors[random(NUM_COLORS)];
    }
    currColors[i] = val;
  }
}
