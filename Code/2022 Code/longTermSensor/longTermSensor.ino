

//Libraries
//#include <LowPower.h>//Low Power Library; Be careful on USB based Arduino to properly activate USB
#include <SPI.h>//Library for SPI, allows communication between components
#include <SparkFunDS3234RTC.h>//Library for Real Time Clock
#include <SD.h>//Library for SD card

//Declartion to put Date in US standard form
#define PRINT_USA_DATE
#define DS13074_CS_PIN 10
// Declaration of Variables
int voltCathode =A0;
int presVoltage = 0;
int calculatedVolt;
void setup()
{
delay(10000);
rtc.begin(DS13074_CS_PIN);
rtc.autoTime();
Serial.begin(9600);
Serial.println("Timestamp, Voltage(mV)");
}

void loop() 
{
  presVoltage=analogRead(voltCathode);
  calculatedVolt=presVoltage*(3300/1024)*1.07527;
  rtc.update();
  printTime();
  Serial.println(calculatedVolt);
//  Serial.print("Here is your voltage(mV):");
//  Serial.println(calculatedVolt);
  delay(3000);
 // USBCON = 0;
  //LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF); 
 // USBDevice.attach();
}

void printTime()
{
 #ifdef PRINT_USA_DATE
  Serial.print(String(rtc.month()) + "/" +   // Print month
                 String(rtc.date()) + "/");  // Print date
 #else
  Serial.print(String(rtc.date()) + "/" +    // (or) print date
                 String(rtc.month()) + "/"); // Print month
 #endif
  Serial.print(String(rtc.year())+" ");        // Print year
  Serial.print(String(rtc.hour()) + ":"); // Print hour
  if (rtc.minute() < 10)
    Serial.print('0'); // Print leading '0' for minute
  Serial.print(String(rtc.minute()) + ":"); // Print minute
  if (rtc.second() < 10)
    Serial.print('0'); // Print leading '0' for second
  Serial.print(String(rtc.second())+ ','); // Print second

  if (rtc.is12Hour()) // If we're in 12-hour mode
  {
    // Use rtc.pm() to read the AM/PM state of the hour
    if (rtc.pm()) Serial.print(" PM"); // Returns true if PM
    else Serial.print(" AM");
  }
  // Few options for printing the day, pick one:
  //Serial.print(rtc.dayStr()); // Print day string
  //Serial.print(rtc.dayC()); // Print day character
  //Serial.print(rtc.day()); // Print day integer (1-7, Sun-Sat)
}
