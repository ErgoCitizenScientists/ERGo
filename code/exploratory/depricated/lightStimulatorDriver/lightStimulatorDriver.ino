/*
   DJP 7/1/2019
   This iteration doesn't randomize the assays. It may randomize the delivery of stimuli within assays.
   This iteration doesn't deliver on/off signals. Those are interpolated later.
*/


/*
  STIM DELIVERY PARAMETERS
*/
const int NUM_COLORS = 5; // number of colors to send, i.e., CFF (1), CFF and CDA (2), CFF and CDA and ... (3)

const int RED_PIN = 9, GREEN_PIN = 10, BLUE_PIN = 11, IR_PIN = 5, UV_PIN = 6;

const int colors[NUM_COLORS] = {RED_PIN, GREEN_PIN, BLUE_PIN, IR_PIN, UV_PIN}; // original
int currColors[NUM_COLORS]; // to scramble


/*
   TRIAL PARAMETERS
*/
const int ONTIME = 2000;
const int NUM_TRIALS = 4; // number of repetitions for each assay for each color

const int BTN1 = 3;
const int BTN2 = 2;

// Pin for sending data to board
const int CFF_SIGNAL_PIN = 4; //2; WHOOOO IT'S 4 FOR THE NEW BOARD!!!; sends '3' for cff information
const int ONOFF_PIN = 8;//5; // for sending messages of on/off, sends '3' for on/off
const int ASSAY_SIGNAL_PIN = 7;//6; // sends '2' mark for saying which assay it is
void setup() {
  Serial.begin(9600);

  pinMode(BTN1, INPUT);
  pinMode(BTN2, INPUT);
  pinMode(CFF_SIGNAL_PIN, OUTPUT);
  pinMode(ONOFF_PIN, OUTPUT);
  pinMode(ASSAY_SIGNAL_PIN, OUTPUT);

  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  pinMode(IR_PIN, OUTPUT);
  pinMode(UV_PIN, OUTPUT);

  digitalWrite(RED_PIN, HIGH);
  digitalWrite(GREEN_PIN, HIGH);
  digitalWrite(BLUE_PIN, HIGH);
  digitalWrite(IR_PIN, HIGH);
  digitalWrite(UV_PIN, HIGH);


  // initialize to -1 for each slot
  resetArr(currColors, NUM_COLORS);

  // set up order of stimuli
  randomSeed(analogRead(0)); // seed
}
//method for resetting both arrays
void resetArr(int arr[], int arrS) {
  for (int i = 0; i < arrS; i++) {
    arr[i] = -1;
  }
}

// Overloaded for CFF
void sendAssayTypeSig(int code, int pin) {
  // send one aspect of assay id or on/off data
  for (int i = 0; i < code; i++) {
    digitalWrite(pin, LOW);
    mdelay(100);
    digitalWrite(pin, HIGH);
    mdelay(100);
  }
  mdelay(400);
}

/* Critical flicker frequency
   Present randomized frequencies and colors
*/

void CFFAssay() {
  int freqs[] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512};
  const int NUM_FREQS = sizeof(freqs) / sizeof(freqs[0]);

  // initialize random order
  // clear variable
  int CFFOrder[NUM_FREQS * NUM_COLORS];
  for (int i = 0; i < NUM_FREQS * NUM_COLORS; i++) {
    CFFOrder[i] = -1;
  }

  // randomize order of variable
  randomize2D(CFFOrder, NUM_FREQS, NUM_COLORS);
  //test
  for (int i = 0; i < NUM_FREQS; i++) {
    for (int j = 0; j < NUM_COLORS; j++) {
      Serial.print(CFFOrder[i * NUM_COLORS + j]);
      Serial.print("\t");
    }
    Serial.println();
  }

  // run stimuli
  for (int i = 0; i < NUM_FREQS * NUM_COLORS; i++) {
    // get current frequency and color
    int freqIndex = getFreq(CFFOrder, NUM_FREQS, NUM_COLORS, i);
    int currFreq = freqs[freqIndex];

    int pinIndex = getColor(CFFOrder, NUM_FREQS, NUM_COLORS, i);
    int colorPin = colors[pinIndex];

    // send signal
    sendAssayTypeSig(freqIndex + 1, CFF_SIGNAL_PIN); // need to have at least one to have the right interval
    sendAssayTypeSig(colorPin     , CFF_SIGNAL_PIN); // corresponds to a pin in the array of led pins

    int period = float(1000 / currFreq / 2 );
    unsigned long startTime = millis();

    delay(20); // another little wait

    int nPeriods = floor(ONTIME / period / 2);
    while (millis() - startTime < ONTIME ) {  // for (int j = 0; j < nPeriods; j++){  //

      digitalWrite(colorPin, LOW);

      // becomes interval is less than a millisecond,
      if (currFreq == 512) {
        udelay(period * 1000);
      } else {
        mdelay(period);
      }

      digitalWrite(colorPin, HIGH);
      mdelay(period);
    }

    delay(3000); // wait a bit second before the next trial
  }
}


void randomize2D(int arr[], int arrS1, int arrS2) {
  for (int i = 0; i < arrS1; i++) {
    for (int j = 0; j < arrS2; j++) {
      int val = random(arrS1 * arrS2);
      while (contains2D(val, arr, arrS1, arrS2)) {
        val = random(arrS1 * arrS2);
      }
      arr[i * arrS2 + j] = val;
    }
  }
}

bool contains2D(int val, int arr[], int arrS1, int arrS2) {
  for (int i = 0; i < arrS1; i++) {
    for (int j = 0; j < arrS2; j++) {
      if (arr[i * arrS2 + j] == val) {
        return true;
      }
    }
  }
  return false;
}

int getFreq(int arr[], int arrS1, int arrS2, int index) {
  for (int i = 0; i < arrS1; i++) {
    for (int j = 0; j < arrS2; j++) {
      if (arr[i * arrS2 + j] == index) {
        return i;
      }
    }
  }
  Serial.print("error in getFreq");
  return -1;
}

int getColor(int arr[], int arrS1, int arrS2, int index) {
  for (int i = 0; i < arrS1; i++) {
    for (int j = 0; j < arrS2; j++) {
      if (arr[i * arrS2 + j] == index) {
        return j;
      }
    }
  }
  Serial.print("error in getColor");
  return -1;
}

// sees relative response of color
void colorDecayAssay(int pin) {
  for (int i = 0; i < NUM_TRIALS; i++) {
    analogWrite(pin, NUM_TRIALS - floor(255 * i / NUM_TRIALS)); // inverted because LEDS are connected to VCC
    digitalWrite(ONOFF_PIN, HIGH);
    mdelay(ONTIME);

    digitalWrite(ONOFF_PIN, LOW);
    digitalWrite(pin, HIGH);
    mdelay(ONTIME);
  }
}

// Stimulus specific adaptation. Is this useful given the CFF?
void colorAdaptationAssay(int pin) {
  // TODO: make this so that you are changing the duty-
  // cycle to see how it affects the filter properties of the retina
  // TODO: does this have to be randomized too?
  for (int i = 0; i < NUM_TRIALS; i++) {
    digitalWrite(pin, LOW);
    digitalWrite(ONOFF_PIN, HIGH);
    mdelay(ONTIME);

    digitalWrite(pin, HIGH);
    digitalWrite(ONOFF_PIN, LOW);
    mdelay(ONTIME);
  }
}



void mdelay(int msInterval) {
  long prev = millis();
  while (millis() - prev < msInterval) {
    /* wait!*/
  }
}
void udelay(int usInterval) {
  long prev = micros();
  while (micros() - prev < usInterval) {
    /* wait!*/
  }
}

bool contains(int val, int arr[], int arrS) {
  for (int i = 0; i < arrS; i++) {
    if (arr[i] == val) {
      return true;
    }
  }
  return false;
}



/*
   initialize random order for colors
*/

void randomizeColors() {
  // randomizes pin numbers
  for (int i = 0; i < NUM_COLORS; i++) {

    //should be different each time because was randomly seeded
    int val = colors[random(NUM_COLORS)]; // gets the pin number, not index

    while (contains(val, currColors, NUM_COLORS)) {
      val = colors[random(NUM_COLORS)];
    }
    currColors[i] = val;
  }
}


void loop() {
  if (digitalRead(BTN1) == HIGH) {
    // Critical flicker fusion frequency
    sendAssayTypeSig(1, ASSAY_SIGNAL_PIN);
    CFFAssay();

//    // color decay
//    for (int i = 0; i < NUM_COLORS - 2; i++) { //  because ir and uv arent pwm. wait still?
//      sendAssayTypeSig(2, ASSAY_SIGNAL_PIN);
//      sendAssayTypeSig(currColors[i], ASSAY_SIGNAL_PIN);
//      colorDecayAssay(currColors[i]);
//    }
//    resetArr(currColors, NUM_COLORS);
//    randomizeColors();
//
//    // color adaptation
//    for (int i = 0; i < NUM_COLORS; i++) {
//      sendAssayTypeSig(3, ASSAY_SIGNAL_PIN);
//      sendAssayTypeSig(currColors[i], ASSAY_SIGNAL_PIN);
//      colorAdaptationAssay(currColors[i]);
//    }
//    resetArr(currColors, NUM_COLORS);
//    randomizeColors();

    // light adaptation
//    for (int i = 0; i < NUM_COLORS; i++) {
//      sendAssayTypeSig(4, ASSAY_SIGNAL_PIN);
//      sendAssayTypeSig(currColors[i], ASSAY_SIGNAL_PIN);
//      lightDarkAdaptation(currColors[i]);
//    }
//    resetArr(currColors, NUM_COLORS);
//    randomizeColors();
//  }
}
