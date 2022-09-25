#include <avr/sleep.h> // Header files
#include "LowPower.h"
//#define DEBUGGING

const int trig = 3;     // trig pin of HC-SR04
const int echo = 2;     // echo pin of HC-SR04
const int mosfetUltrasonic = 9; // pin of mosfet connected to ultrasonic HC-SR04
const int mosfetPump = 10; // pin of mosfet connected to load (pump)
const int Waterlevel_limit = 10; //(cm) attach the minimum duration between water surface and the ultrasonic sensor 

int Times = 30; // times the ultrasonic does to verify the duration
int timesSleep = 75; // (times) [8 * timesSleep] seconds in low power mode
int timePump = 30000; // (ms) time the pump does every once
byte checkertimes = 0;
byte checkertimesPower = 0;

void setup()
{    
//    #ifdef DEBUGGING
//    Serial.begin(9600);     // commucicate Serial with baudrate 9600
//    Serial.println("Aupump starts");
//    // in debugging mode, we can reduce time of the test
//    timesSleep = 1;
//    timePump = 3000;
//    Times = 1;
//    #endif

    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(mosfetUltrasonic, OUTPUT);
    pinMode(mosfetPump, OUTPUT);
    pinMode(trig,OUTPUT);   // trig pin sends signals
    pinMode(echo,INPUT);    // echo pin receives signals

    //set all sensors off
    digitalWrite(mosfetUltrasonic, 0);
    digitalWrite(mosfetPump, 0);
}

void(* resetFunc) (void) = 0; //declare reset function at address 0 - MUST BE ABOVE LOOP

void loop()
{
    if(checkertimesPower >= 12) resetFunc(); //call reset every 2 hours
    
    if(getDistance() < Waterlevel_limit) // if the water level is too high, lower it 
    {
      driversTroubleshoot(1);        
      
      digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
      delay(500);                       // wait for a second
      digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
      
      digitalWrite(mosfetPump, 1); // switch the pump on
      delay(timePump); //wait for about timePump in miliseconds
      digitalWrite(mosfetPump, 0); // switch the pump off
    }
    else
    {
        driversTroubleshoot(0);
        
//      #ifdef DEBUGGING
//      Serial.println("Sleep");
//      #endif
      
      // Enter power down state for timeSleep * 8 mins with ADC and BOD module disabled
      for(int i = 0; i < timesSleep; i++)
        LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);

//      #ifdef DEBUGGING
//      Serial.println("Power on");
//      #endif
    }
}

int getDistance()
{
  digitalWrite(mosfetUltrasonic, 1); // turn ultrasensor on
  
  delay(500);
  
  unsigned long duration; // measure time
  int distance;           // refer direction through wave time
  int arr[30] = {};
  byte maximize = 0, Val = 30;

//  #ifdef DEBUGGING
//  Serial.println("Starting ultrasonic");
//  #endif
  
  for(byte i = 0; i < Times; i++)
  {
    /* transmit pulse in trig */
    digitalWrite(trig,0);   // trig off
    delayMicroseconds(2);
    digitalWrite(trig,1);   // transmit in trig
    delayMicroseconds(10);   // on 5 microSeconds
    digitalWrite(trig,0);   // then turn off the process
    
    /* measure time */
    // Measure the width of HIGH pulse in echo pin
    duration = pulseIn(echo, HIGH);  
    // Calculate the distance
    distance = int(duration /2 /29.412);

    if(distance < 30)
    {
      if(++arr[distance] >= maximize)
      {
        maximize = arr[distance];
        Val = distance;
      }
      
//      #ifdef DEBUGGING
//      /* Print the result in Serial Monitor */
//      Serial.print(distance);
//      Serial.println("cm");
//      #endif
    }
    delay(30);
  }
  digitalWrite(mosfetUltrasonic, 0); // turn ultrasensor off
  
//  #ifdef DEBUGGING
//  Serial.print("Exact distance:");
//  Serial.println(Val);
//  Serial.println("End");
//  #endif
  
  return Val;
}

void driversTroubleshoot(int Signal)
{
  switch (Signal)
  {
    case 0:
      checkertimes = 0;
      checkertimesPower++;
    break;

    case 1:
      checkertimes++;

      if((checkertimes > 40) && (getDistance() <= 6)) // if drivers have a trouble when pumping a lot
        {
//          #ifdef DEBUGGING
//            Serial.print("Something went wrong. Please check again and reboot me");
//          #endif
          digitalWrite(LED_BUILTIN, HIGH);
          set_sleep_mode (SLEEP_MODE_PWR_DOWN);
          sleep_enable();
          sleep_cpu ();
        }
        else checkertimes = 0;
    break;
  }
}
