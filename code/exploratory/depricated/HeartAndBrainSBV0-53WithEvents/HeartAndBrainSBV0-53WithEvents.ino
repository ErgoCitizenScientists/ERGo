//
// Heart & Brain based on Leonardo
// V1.0
// Made for Heart & Brain SpikerBox (V0.53 - Production)
// Backyard Brains
// Stanislav Mircic
// https://backyardbrains.com/
//
// Carrier signal is at DIO 13 
//

#define CURRENT_SHIELD_TYPE "HWT:HBLEOSB;"


#define BUFFER_SIZE 256  //sampling buffer size
#define SIZE_OF_COMMAND_BUFFER 30 //command buffer size

// defines for setting and clearing register bits
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

int buffersize = BUFFER_SIZE;
int head = 0;//head index for sampling circular buffer
int tail = 0;//tail index for sampling circular buffer
byte writeByte;
char commandBuffer[SIZE_OF_COMMAND_BUFFER];//receiving command buffer
byte reading[BUFFER_SIZE]; //Sampling buffer
#define ESCAPE_SEQUENCE_LENGTH 6
byte escapeSequence[ESCAPE_SEQUENCE_LENGTH] = {255,255,1,1,128,255};
byte endOfescapeSequence[ESCAPE_SEQUENCE_LENGTH] = {255,255,1,1,129,255};

#define CARRIER_PIN 13

/// Interrupt number - very important in combination with bit rate to get accurate data
int interrupt_Number=198;// Output Compare Registers  value = (16*10^6) / (Fs*8) - 1  set to 1999 for 1000 Hz sampling, set to 3999 for 500 Hz sampling, set to 7999 for 250Hz sampling, 199 for 10000 Hz Sampling
int numberOfChannels = 1;//current number of channels sampling
int tempSample = 0; 
int commandMode = 0;//flag for command mode. Don't send data when in command mode

#define EVENT_INPUT_1 2
#define EVENT_INPUT_2 3
#define EVENT_INPUT_3 4
#define EVENT_INPUT_4 5
#define EVENT_INPUT_5 6
#define EVENT_INPUT_6 7

#define MAX_DEBOUNCE_TIME 2000
#define MAX_DELAY_COUNTER 32000
int delayCounter = 0;
int buttonEnabled1 = 1;
int buttonDebounceTimer1 = 0;
int buttonEnabled2 = 1;
int buttonDebounceTimer2 = 0;
int buttonEnabled3 = 1;
int buttonDebounceTimer3 = 0;
int buttonEnabled4 = 1;
int buttonDebounceTimer4 = 0;
int buttonEnabled5 = 1;
int buttonDebounceTimer5 = 0;
int buttonEnabled6 = 1;
int buttonDebounceTimer6 = 0;


void setup(){ 
  Serial.begin(230400); //Serial communication baud rate (alt. 115200)
  //while (!Serial)
  //{}  // wait for Serial comms to become ready
  delay(300); //whait for init of serial
  Serial.println("StartUp!");
  Serial.setTimeout(2);
  pinMode(CARRIER_PIN, OUTPUT);//PB6 - pin 10

  pinMode(EVENT_INPUT_1, INPUT_PULLUP);
  pinMode(EVENT_INPUT_2, INPUT_PULLUP);
  pinMode(EVENT_INPUT_3, INPUT_PULLUP);
  pinMode(EVENT_INPUT_4, INPUT_PULLUP);
  pinMode(EVENT_INPUT_5, INPUT_PULLUP);
  pinMode(EVENT_INPUT_6, INPUT_PULLUP);
   
  // TIMER SETUP- the timer interrupt allows preceise timed measurements of the reed switch
  //for mor info about configuration of arduino timers see http://arduino.cc/playground/Code/Timer1
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
  OCR1A = interrupt_Number;// Output Compare Registers 
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




ISR(TIMER1_COMPA_vect) 
{
   PORTC ^= B10000000;//generate 5kHz carrier signal for AM modulation on D13 (bit 7 on port C)
   
   if(commandMode!=1)
   {
     //Put samples in sampling buffer "reading". Since Arduino Leonardo has 10bit ADC we will split every sample to 2 bytes
     //First byte will contain 3 most significant bits and second byte will contain 7 least significat bits.
     //First bit in all byte will not be used for data but for marking begining of the frame of data (array of samples from N channels)
     //Only first byte in frame will have most significant bit set to 1
     
       //Sample first channel and put it into buffer
       tempSample = analogRead(A0);
       reading[head] =  (tempSample>>7)|0x80;//Mark begining of the frame by setting MSB to 1
       head = head+1;
       if(head==BUFFER_SIZE)
       {
         head = 0;
       }
       reading[head] =  tempSample & 0x7F;
       head = head+1;
       if(head==BUFFER_SIZE)
       {
         head = 0;
       }
       
   }
   
   if(delayCounter>MAX_DELAY_COUNTER)
   {
   
        //check if button is pressed (HIGH)
        if (digitalRead(EVENT_INPUT_1))
        { 
              if(buttonEnabled1==1)
              {
                if(buttonDebounceTimer1 == 0)
                {
                  buttonEnabled1 = 0;
                  buttonDebounceTimer1 = MAX_DEBOUNCE_TIME;
    
                  sendMessage("EVNT:1;");
                }    
              }
        }
        else
        {
            buttonEnabled1 = 1;
        }
        if(buttonDebounceTimer1>0)
        {
          buttonDebounceTimer1--;  
        }
    
        //check if button is pressed (HIGH)
        if (digitalRead(EVENT_INPUT_2))
        { 
              if(buttonEnabled2==1)
              {
                if(buttonDebounceTimer2 == 0)
                {
                  buttonEnabled2 = 0;
                  buttonDebounceTimer2 = MAX_DEBOUNCE_TIME;
                  sendMessage("EVNT:2;");
                }    
              }
        }
        else
        {
            buttonEnabled2 = 1;
        }
        if(buttonDebounceTimer2>0)
        {
          buttonDebounceTimer2--;  
        }
    
        //check if button is pressed (HIGH)
        if (digitalRead(EVENT_INPUT_3))
        { 
              if(buttonEnabled3==1)
              {
                if(buttonDebounceTimer3 == 0)
                {
                  buttonEnabled3 = 0;
                  buttonDebounceTimer3 = MAX_DEBOUNCE_TIME;
                  sendMessage("EVNT:3;");
                }    
              }
        }
        else
        {
            buttonEnabled3 = 1;
        }
        if(buttonDebounceTimer3>0)
        {
          buttonDebounceTimer3--;  
        }
    
        //check if button is pressed (HIGH)
        if (digitalRead(EVENT_INPUT_4))
        { 
              if(buttonEnabled4==1)
              {
                if(buttonDebounceTimer4 == 0)
                {
                  buttonEnabled4 = 0;
                  buttonDebounceTimer4 = MAX_DEBOUNCE_TIME;
    
                  sendMessage("EVNT:4;");
                }    
              }
        }
        else
        {
            buttonEnabled4 = 1;
        }
        if(buttonDebounceTimer4>0)
        {
          buttonDebounceTimer4--;  
        }
    
        //check if button is pressed (HIGH)
        if (digitalRead(EVENT_INPUT_5))
        { 
              if(buttonEnabled5==1)
              {
                if(buttonDebounceTimer5 == 0)
                {
                  buttonEnabled5 = 0;
                  buttonDebounceTimer5 = MAX_DEBOUNCE_TIME;
    
                  sendMessage("EVNT:5;");
                }    
              }
        }
        else
        {
            buttonEnabled5 = 1;
        }
        if(buttonDebounceTimer5>0)
        {
          buttonDebounceTimer5--;  
        }
    
    
        //check if button is pressed (HIGH)
        if (digitalRead(EVENT_INPUT_6))
        { 
              if(buttonEnabled6==1)
              {
                if(buttonDebounceTimer6 == 0)
                {
                  buttonEnabled6 = 0;
                  buttonDebounceTimer6 = MAX_DEBOUNCE_TIME;
    
                  sendMessage("EVNT:6;");
                }    
              }
        }
        else
        {
            buttonEnabled6 = 1;
        }
        if(buttonDebounceTimer6>0)
        {
          buttonDebounceTimer6--;  
        }

    
   }
   else
   {
      delayCounter++; 
   }

}
  

//push message to main sending buffer
//timer for sampling must be dissabled when 
//we call this function
void sendMessage(const char * message)
{

  int i;
  //send escape sequence
  for(i=0;i< ESCAPE_SEQUENCE_LENGTH;i++)
  {
      reading[head++] = escapeSequence[i];
      if(head==BUFFER_SIZE)
      {
        head = 0;
      }
  }

  //send message
  i = 0;
  while(message[i] != 0)
  {
      reading[head++] = message[i++];
      if(head==BUFFER_SIZE)
      {
        head = 0;
      }
  }

  //send end of escape sequence
  for(i=0;i< ESCAPE_SEQUENCE_LENGTH;i++)
  {
      reading[head++] = endOfescapeSequence[i];
      if(head==BUFFER_SIZE)
      {
        head = 0;
      }
  }
  
}




void loop(){
    
    while(head!=tail && commandMode!=1)//While there are data in sampling buffer whaiting 
    {
      Serial.write(reading[tail]);
      //Move thail for one byte
      tail = tail+1;
      if(tail>=BUFFER_SIZE)
      {
        tail = 0;
      }
    }

    if(Serial.available()>0)
    {
                  commandMode = 1;//frag that we are receiving commands through serial
                  //TIMSK1 &= ~(1 << OCIE1A);//disable timer for sampling
                  // read untill \n from the serial port:
                  String inString = Serial.readStringUntil('\n');
                
                  //convert string to null terminate array of chars
                  inString.toCharArray(commandBuffer, SIZE_OF_COMMAND_BUFFER);
                  commandBuffer[inString.length()] = 0;
                  
                  
                  // breaks string str into a series of tokens using delimiter ";"
                  // Namely split strings into commands
                  char* command = strtok(commandBuffer, ";");
                  while (command != 0)
                  {
                      // Split the command in 2 parts: name and value
                      char* separator = strchr(command, ':');
                      if (separator != 0)
                      {
                          // Actually split the string in 2: replace ':' with 0
                          *separator = 0;
                          --separator;
                          if(*separator == 'c')//if we received command for number of channels
                          {
                            separator = separator+2;
                            numberOfChannels = 1;//atoi(separator);//read number of channels
                          }
                           if(*separator == 's')//if we received command for sampling rate
                          {
                            //do nothing. Do not change sampling rate at this time.
                            //We calculate sampling rate further below as (max Fs)/(Number of channels)
                          }

                          if(*separator == 'b')//if we received command for impuls
                          {
                            sendMessage(CURRENT_SHIELD_TYPE);
                          }
                      }
                      // Find the next command in input string
                      command = strtok(0, ";");
                  }
                  //calculate sampling rate
                  
                  //TIMSK1 |= (1 << OCIE1A);//enable timer for sampling
                  commandMode = 0;
      }
    
}
