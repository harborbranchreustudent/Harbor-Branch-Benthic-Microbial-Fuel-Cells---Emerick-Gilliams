/*
 * This setup allows you to measure the voltage of two objects (Fuel Cells, Batteries, etc.) up to 3000 mV. 
 * The setup is currently calibrated to measure once every 10 seconds and goes to low power sleep when not actively measuring anything.
 * Calibrate the Cali variable to the voltage of the VCC pin that is going to the breadboard. 
 * This variation can be used for both charging and discharging tests on fuel cells. 
 * Use A0 for Fuel Cell 1 and A2 for Fuel Cell 2
 * This is calibrated for the USB power source from a PC. 
 * 
 */


#include <Time.h>
#include <TimeLib.h>
#include <DS3232RTC.h>
#include <SD.h>
#include <SPI.h>
#include <avr/sleep.h> //this AVR library contains the methods that controls the sleep modes
#define interruptPin 2 //Pin we are going to use to wake up the Arduino

//Calibration from VCC pin. 
float Cali = 3.35;

//Files
File voltData;

//Pin Values
int pinCS = 10; //SD Card

//Variables for Timed Events
const unsigned long eventTime_1_Volt = 300; // in ms
const unsigned long eventTime_2_SD = 2100;

unsigned long previousTime_1 = 0;
unsigned long previousTime_2 = 0;

//Voltmeter Variables
float input_voltage = 0.0;
float input_voltage2 = 0.0;
float temp=0.0;

//RTC Module global variables
const int time_interval=10;  // Sets the wakeup interval in seconds

// -#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-# //
//                                                            SETUP
// -#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-# //

 
void setup()
{
  //---------------------------------------------------SERIAL and SLEEP---------------------------------------------------//
  Serial.begin(9600);     //  opens serial port, sets data rate to 9600 bps
  pinMode(LED_BUILTIN,OUTPUT);//We use the led on pin 13 to indecate when Arduino is A sleep
  pinMode(interruptPin,INPUT_PULLUP);//Set pin d2 to input using the buildin pullup resistor
  digitalWrite(LED_BUILTIN,HIGH);//turning LED on


  //----------------------------------------------------------RTC----------------------------------------------------------//
  
  // initialize the alarms to known values, clear the alarm flags, clear the alarm interrupt flags. DO NOT TOUCH
  RTC.setAlarm(ALM1_MATCH_DATE, 0, 0, 0, 1);
  RTC.setAlarm(ALM2_MATCH_DATE, 0, 0, 0, 1);
  RTC.alarm(ALARM_1);
  RTC.alarm(ALARM_2);
  RTC.alarmInterrupt(ALARM_1, false);
  RTC.alarmInterrupt(ALARM_2, false);
  RTC.squareWave(SQWAVE_NONE);
  
  /*
   * JUST USE THE SERIAL SET UP EXAMPLE
   * 
   * Uncomment the block block to set the time on your RTC. Remember to comment it again 
   * otherwise you will set the time at everytime you upload the sketch
   * /
   /* Begin block *_///<---- Delete underscore to edit
   tmElements_t tm;
  tm.Hour = 16;               // set the RTC to an arbitrary time
  tm.Minute = 59;
  tm.Second = 30;
  tm.Day = 5;
  tm.Month = 6;
  tm.Year = 2020;      // tmElements_t.Year is the offset from 1970. DOESN'T WORKKKKK
  RTC.write(tm);              // set the RTC from the tm structure
   //Block end * */
  
  time_t t; //create a temporary time variable so we can set the time and read the time from the RTC
  t=RTC.get();//Gets the current time of the RTC
  RTC.setAlarm(ALM1_MATCH_SECONDS , second(t) + time_interval, 0, 0, 0); //Goes off every 10 seconds. 
  //RTC.setAlarm(ALM1_MATCH_MINUTES , 59, minute(t)+time_interval, 0, 0);// Setting alarm 1 to go off 10 minutes from now
  //RTC.setAlarm(ALM1_MATCH_MINUTES , 59, 0, 0, 0);// Setting alarm 1 to go off at the top of the next hour.
  // clear the alarm flag
  RTC.alarm(ALARM_1);
  // configure the INT/SQW pin for "interrupt" operation (disable square wave output)
  RTC.squareWave(SQWAVE_NONE);
  // enable interrupt output for Alarm 1
  RTC.alarmInterrupt(ALARM_1, true);

  //----------------------------------------------------------SD----------------------------------------------------------//
  pinMode(pinCS, OUTPUT); //Setting Pin
  
  if (SD.begin())
  {
    Serial.println("SD card is ready to use.");
  
  //Setup the File for Data  
    voltData = SD.open("VoltData.txt", FILE_WRITE);
  
    // If the file opened, write to it:
    if (voltData) {
      // Write to file
      voltData.print("Date");
      voltData.print(",    ");
      voltData.print("Time");
      voltData.print(",     ");
      voltData.print("Voltage (A0)");
      voltData.print(",     ");
      voltData.println("Voltage (A2)");
      voltData.close();}
  } 
  else
  {
    Serial.println("SD card initialization failed");
    return;
  }
  
}

// -#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-# //
//                                                   LOOP
// -#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-# //

void loop()
{
// Updates constantly
  unsigned long currentTime = millis();
  
//Event 1  
  if(currentTime - previousTime_1 >= eventTime_1_Volt){
  
//Conversion formula for voltage
   
    int analog_value = analogRead(A0);
    input_voltage = (analog_value * Cali * 1000) / 1024.0; 
    int analog_value2 = analogRead(A2);
    input_voltage2 = (analog_value2 * Cali * 1000) / 1024.0; 
   
    if (input_voltage < 0.01) 
    {
      input_voltage=0.0;
    } 
    if (input_voltage2 < 0.01) 
    {
      input_voltage2=0.0;
    }
    Serial.print("v = ");
    Serial.print(input_voltage);
    Serial.print(", ");
    Serial.println(input_voltage2);

    
    //Reset the clock
    previousTime_1 = currentTime;
  }
  //--------------------------------------------------------------SD START---------------------------------------------------------------//
  
//Event 2  
  if(currentTime - previousTime_2 >= eventTime_2_SD){

  tmElements_t tmCurr; //tmCurr is just a variable to hold current time
  time_t tCurr;
  RTC.read(tmCurr);
  tCurr = makeTime(tmCurr);
  
  int a = day(tCurr);
  String d;
  d = String(a); 
  
  // Create/Open file 
  if (day(tCurr) < 10) {
    voltData = SD.open("0"+d+monthShortStr(month(tCurr))+"20.txt", FILE_WRITE);
  }
  else {
    voltData = SD.open(d+monthShortStr(month(tCurr))+"20.txt", FILE_WRITE);
  }
  if (voltData.available() == 0) {
    
  }

  //----------------------------------------------------------SD REPORT OF DATA----------------------------------------------------------//
  
  // If the file opened, write to it:
  if (voltData) {
    // Write to file
    

    
    voltData.print(month(tCurr));
    voltData.print("/");
    voltData.print(day(tCurr));
    voltData.print("/");
    voltData.print(year(tCurr));
    voltData.print(", ");
    
    voltData.print(hour(tCurr));
    voltData.print(":");
    voltData.print(minute(tCurr));
    voltData.print(":");
    voltData.print(second(tCurr));
    voltData.print(", ");
    
    voltData.print(input_voltage);
    voltData.print(", ");
    voltData.println(input_voltage2);
    voltData.close();
    
    Serial.print("Written to file: ");
    
  //----------------------------------------------------------SERIAL REPORT OF DATA----------------------------------------------------------//
    
    Serial.print(month(tCurr));
    Serial.print("/");
    Serial.print(day(tCurr));
    Serial.print("/");
    Serial.print(year(tCurr));
    Serial.print(", ");
    
    Serial.print(hour(tCurr));
    Serial.print(":");
    Serial.print(minute(tCurr));
    Serial.print(":");
    Serial.print(second(tCurr));
    Serial.print(", ");
    
    Serial.print(input_voltage);
    Serial.print(", ");
    Serial.println(input_voltage2);
  }
  
  // If the file didn't open, print an error:
  else {
    Serial.println("error opening file");
  }

  Going_To_Sleep();
  
    
  //Reset the clock
  previousTime_2 = currentTime;
  }
}

void Going_To_Sleep(){
    sleep_enable();//Enabling sleep mode
    attachInterrupt(0, wakeUp, LOW);//attaching a interrupt to pin d2
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);//Setting the sleep mode, in our case full sleep
    digitalWrite(LED_BUILTIN,LOW);//turning LED off
    time_t tUp;// creates temp time variable
    tUp=RTC.get(); //gets current time from rtc
    Serial.println("Sleep  Time: "+String(hour(tUp))+":"+String(minute(tUp))+":"+String(second(tUp)));//prints time stamp on serial monitor
    delay(500); //wait 1/2 second to allow the led to be turned off before going to sleep
    sleep_cpu();//activating sleep mode
    
    //Interrupt occurs from alarm and wakes up Arduino
    digitalWrite(LED_BUILTIN,HIGH);//turning LED on
    time_t tDown;
    tDown=RTC.get();
    Serial.println("WakeUp Time: "+String(hour(tDown))+":"+String(minute(tDown))+":"+String(second(tDown)));//Prints time stamp 
    //Set New Alarm
    //RTC.setAlarm(ALM1_MATCH_MINUTES , 59, minute(tDown)+time_interval, 0, 0);

    int Alm;
    if ((second(tDown) + time_interval) > 60) {
      Alm = (second(tDown) + time_interval) - 60;
    }
    else {
      Alm = (second(tDown) + time_interval);
    }
    RTC.setAlarm(ALM1_MATCH_SECONDS , Alm, 0, 0, 0);
  
  // clear the alarm flag
  RTC.alarm(ALARM_1);
  }

void wakeUp(){
  Serial.println("Interrrupt Activated");//Print message to serial monitor
   sleep_disable();//Disable sleep mode
  detachInterrupt(0); //Removes the interrupt from pin 2;
 
}
