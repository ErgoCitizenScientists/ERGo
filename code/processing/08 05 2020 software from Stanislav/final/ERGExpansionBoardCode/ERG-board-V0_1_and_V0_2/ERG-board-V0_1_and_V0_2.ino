/*
 * DJP 7/1/2019
 * 
 * modified by Stanislav Mircic on 3. Dec. 2019
 * modified by Stanislav Mircic on 3. Aug. 2020
 * made for ERG expansion board V0.1 and V0.2 depending on BOARD_VERSION_0_1/BOARD_VERSION_0_2
 * Based on ATmega328 (Arduino UNO)
 * made to interface with Grass Foundation board V0.93 and H&B SpikerBox Pro V0.5 Gen 1 
 * ENCODER pin on expansion board should be connected to Vcc
 */


/*
* SHoose board version
*/
//#define BOARD_VERSION_0_1
#define BOARD_VERSION_0_2



const int NUM_ASSAYS = 1; // number of assays to run
const int NUM_COLORS = 5; // number of colors to send

#define RED_PIN 9
#define GREEN_PIN 10 
#define BLUE_PIN 11

#if defined(BOARD_VERSION_0_1)
  #define IR_PIN 7 
  #define UV_PIN 8
  #define IO5 6  
#elif defined(BOARD_VERSION_0_2)
  #define IR_PIN 5 
  #define UV_PIN 6
  #define IO5 7 
#else
  #error Unsupported board version selection.
#endif


#define BTN1 3
#define BTN2 2


//CLK is output of ERG board defines clock for event transmition. It should be connected to IO1 on SpikerBox
#define CLK 12
//DATA_OUT is output of ERG board defines serial data for event transmition. It should be connected to IO2 on SpikerBox
#define DATA_OUT 4

const int colors[NUM_COLORS] = {RED_PIN, GREEN_PIN, BLUE_PIN, IR_PIN, UV_PIN}; // original
int currColors[NUM_COLORS]; // to scramble

// defines for setting and clearing register bits
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif


/*
 * trial parameters
 */
const int ONTIME = 2000;//ms
const int NUM_TRIALS = 4; // number of repetitions for each assay for each color


int order[NUM_ASSAYS];

int eventWaitToBeSent = 0;
uint8_t eventThatNeedsToBeSent = 0;

void setup() {
  Serial.begin(230400);

  pinMode(BTN1, INPUT);
  pinMode(BTN2, INPUT);
  pinMode(IO5, OUTPUT);

  
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  pinMode(IR_PIN, OUTPUT);
  pinMode(UV_PIN, OUTPUT);

  pinMode(CLK, OUTPUT);
  pinMode(DATA_OUT, OUTPUT);

  pinMode(IO5, OUTPUT);

  digitalWrite(CLK, LOW);
  digitalWrite(DATA_OUT, LOW);
  digitalWrite(IO5, LOW);

  
  digitalWrite(RED_PIN, HIGH);
  digitalWrite(GREEN_PIN, HIGH);
  digitalWrite(BLUE_PIN, HIGH);
  digitalWrite(IR_PIN, HIGH);
  digitalWrite(UV_PIN, HIGH);


 
  
  // initialize to -1 for each slot
  resetArr(currColors, NUM_COLORS);
  
  // set up order of stimuli
  resetArr(order, NUM_ASSAYS); // init
  randomSeed(analogRead(0)); // seed
  randomizeAndShiftOrder(); // scramble and shift

  cli();//stop interrupts
  //Make ADC sample faster. Change ADC clock
  //Change prescaler division factor to 16
  sbi(ADCSRA,ADPS2);//1
  cbi(ADCSRA,ADPS1);//0
  cbi(ADCSRA,ADPS0);//0

  //set timer1 interrupt at 10kHz
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;//initialize counter value to 0;
  OCR1A = 198;// 10kHz timer
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS11 bit for 8 prescaler
  TCCR1B |= (1 << CS11);   
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);
  
  sei();//allow interrupts
  //END TIMER SETUP
  TIMSK1 |= (1 << OCIE1A);
}



volatile int stimulationEnabledCounter = 0;
#define LENGTH_OF_SIMULUS  20000
int periodCounter = 0;
int currFreq = 0;
int currentPin = 0;
int halfperiod=0;
//serial communication
uint8_t bitsToBeSent = 0;
int lengthOfHalfOfBit = 8;
int lengthOfBit = 13;
int counterForBit = 0;

ISR(TIMER1_COMPA_vect) 
{

  if (stimulationEnabledCounter>0)
  {
    stimulationEnabledCounter--;

    periodCounter++;
    if(periodCounter==halfperiod)
    {
      periodCounter = 0;
      digitalWrite(IO5,digitalRead(currentPin));
      digitalWrite(currentPin, !digitalRead(IO5));

    }
      
  }
  else
  {
    digitalWrite(RED_PIN, HIGH);
    digitalWrite(GREEN_PIN, HIGH);
    digitalWrite(BLUE_PIN, HIGH);
    digitalWrite(IR_PIN, HIGH);
    digitalWrite(UV_PIN, HIGH);
    digitalWrite(IO5,LOW);
  }

  if(eventWaitToBeSent == 1)
  {
    eventWaitToBeSent = 0;
    bitsToBeSent = 8;
    counterForBit =0;
  }

  if(bitsToBeSent>0)
  {
      if(counterForBit==0)
      {
        counterForBit = lengthOfBit;
        digitalWrite(DATA_OUT, (eventThatNeedsToBeSent>>(bitsToBeSent-1)) & B00000001);
        digitalWrite(CLK, HIGH);
        
        
      }
      else
      {
        if(counterForBit==lengthOfHalfOfBit)
        {
          digitalWrite(CLK, LOW);
          bitsToBeSent--;
        }
        counterForBit--;
      }
  }
  else
  {
    digitalWrite(CLK, LOW);
    digitalWrite(DATA_OUT, LOW);
  }

}




void loop() {
  if (digitalRead(BTN1)==HIGH){
    activateMontage();
    resetArr(order, NUM_ASSAYS); // ready to go again
    randomizeAndShiftOrder();
  }
}



void sendEvent(int eventNumber)
{
  eventWaitToBeSent = 1;
  eventThatNeedsToBeSent = eventNumber;
}



/*
 * Randomize a series of trials and run them
 */
void activateMontage(){
  for (int i=0; i<NUM_ASSAYS; i++){
    runTrials(order[i]);
    delay(1000); // must be 1000 so that it adds to 500 ms of delay and is outside of the range of frequencies used to extract data
  }
}

/*
 * Start trial
 */
void runTrials(int assay){  
  randomizeColors();
  CFFAssay2D();
}


/* Critical flicker frequency
 * Present randomized frequencies and colors
 */

void CFFAssay2D(){
  int freqs[] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512};
  const int NUM_FREQS = sizeof(freqs)/sizeof(freqs[0]);
  
  // initialize random order
  int CFFOrder[NUM_FREQS*NUM_COLORS];
  for (int i = 0; i < NUM_FREQS*NUM_COLORS; i++){
      CFFOrder[i] = -1;
  }
  
  // randomize order
  randomize2D(CFFOrder, NUM_FREQS, NUM_COLORS);
  //test
  for (int i = 0; i < NUM_FREQS; i++){
    for (int j = 0; j < NUM_COLORS; j++){
      Serial.print(CFFOrder[i*NUM_COLORS + j]);
      Serial.print("\t");
    }
    Serial.println();
  }
  
  // run stimuli
  for (int i = 0; i < NUM_FREQS * NUM_COLORS; i++)
  {
    // get current frequency and color
    int freqIndex=getFreq(CFFOrder, NUM_FREQS, NUM_COLORS, i);
    currFreq = freqs[freqIndex];

    int pinIndex=getColor(CFFOrder, NUM_FREQS, NUM_COLORS, i);
    currentPin = colors[pinIndex];

    // send info about color
    delay(1000);
    //Serial.print("New stimulation color: ");
    //Serial.print(currentPin);
    //Serial.print(" frequency: ");
    //Serial.println(currFreq);
    halfperiod = (float(10000) / float(currFreq) )/ float(2) ;
    
    sendEvent(pinIndex);
    stimulationEnabledCounter = halfperiod *2* int(LENGTH_OF_SIMULUS/(halfperiod*2));

    while(1)
    {
      if(stimulationEnabledCounter==0)
      {
        break;
        }
    }

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
//    Serial.println(val);
//    Serial.println(order[i]);
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

void randomize(int arr[], int arrS){
  for (int i = 0; i < arrS; i++){
    //should be different each time because was randomly seeded
    int val = random(arrS);
    
    while(contains(val, arr, arrS)){val = random(arrS);}
    
    arr[i] = val;
  }
}
