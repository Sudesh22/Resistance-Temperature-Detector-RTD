//--------------------------------------------IMPORTING LIBRARIES------------------------------------------------------------------------------------------------------//

#include <SPI.h>
#include <WiFi.h>
#include <Wire.h>
#include "RTClib.h"
#include <ADS1X15.h>
#include <AsyncTCP.h>
#include <SPIMemory.h>
#include <WiFiClient.h>
#include <ESPAsyncWebServer.h>
#include <LiquidCrystal_I2C.h>

//--------------------------------------------IMPORTING LIBRARIES------------------------------------------------------------------------------------------------------//



//--------------------------------------------OBJECT INITIALIZATION------------------------------------------------------------------------------------------------------//

RTC_DS3231 rtc;                         // RTC Object
SPIFlash memory;                        // Initialize SPI Flash memory object
LiquidCrystal_I2C lcd(0x27, 20, 4);     // Initialize I2C LCD object

WiFiClient client;                      // Initialising the WiFi Server Object
IPAddress ip(192, 168, 19, 2);          // IP Address of the server
IPAddress gateway(192, 168, 19, 2);     // Gateway same as the Client Device
IPAddress netmask(255, 255, 255, 0);    // Netmask of the Server
AsyncWebServer server(5210);            // Initialising the WebServer at port 5210

//--------------------------------------------OBJECT INITIALIZATION------------------------------------------------------------------------------------------------------//



//--------------------------------------------CHANNEL INITIALIZATION------------------------------------------------------------------------------------------------------//

uint16_t Device_ID = 2241;   // DEVICE ID change as per need
uint8_t i2cMuxes = 1;       // if max 4 then 0, if max 8 then 1, if max 16 then 2
uint8_t channels = 8;       // channel nuimbers can be: 4 / 8 / 16

//--------------------------------------------CHANNEL INITIALIZATION------------------------------------------------------------------------------------------------------//



//--------------------------------------------PIN DECLARATION------------------------------------------------------------------------------------------------------//

#define CHIP_SELECT_PIN 5                         // Pin connected to the SPI Flash memory chip select
const uint8_t shiftRegister1[] = { 12, 14, 13 };  // { ST_CP pin, SH_CP pin, DS pin } for Shift Register 1
const uint8_t shiftRegister2[] = { 26, 33, 25 };  // { ST_CP pin, SH_CP pin, DS pin } for Shift Register 2

//--------------------------------------------PIN DECLARATION------------------------------------------------------------------------------------------------------//



//--------------------------------------------INITIALIZING VARIABLES----------------------------------------------------------------------------------------------------------//

byte muxArray[] = { 0x70 };                                            // I2C Address array for I2C muxes
byte adsArray[] = { 0x48, 0x49, 0x4A, 0x4B, 0x48, 0x49, 0x4A, 0x4B };  // I2C Address array for ADS1115
byte dataShift[] = { 85, 170 };                                        // Data representing 01010101 (85) and 10101010 (170) to control the outputs using shift registers

uint32_t address = 16;          // Starting address to write data
uint32_t LastLocation = 0;      // Memory location to check if rom is empty

bool logState0 = false;         // variable to track when not to save data
bool logState15 = false;        // variable to track when to save data
bool Boot = true;               // variable to track when ESP is reset

float temperatureArray[8] = { 0 };      // Array to hold all the temperature values
float rtdArray[8] = { 0 };              // Array to hold all the RTD resistance values

const char *ssid = "VEDA THERMA X8";    // ESP SSID
const char *password = "12345678";      // ESP Password
String inputDate, inputTime;            // String to store the date and time input by the user

//--------------------------------------------INITIALIZING VARIABLES----------------------------------------------------------------------------------------------------------//



//------------------------------------------------TIME VARIABLES------------------------------------------------------------------------------------------------------//

unsigned long previousTime = 0;                   // variable that holds the previous time in millis
const unsigned long eventInterval = 900000;       // 15 mins into milliseconds that determine the interval after the data is stored into flash
//const unsigned long eventInterval = 30000;      // 30 secs into milliseconds that determine the interval after the data is stored into flash

//------------------------------------------------TIME VARIABLES------------------------------------------------------------------------------------------------------//



//-----------------------------------------------SETUP FUNCTION----------------------------------------------------------------------------------------------------------//

void setup() {
  Wire.begin();             // Begin the Wire object
  memory.begin();           // Initialize SPI Flash memory with chip select pin
  lcd.init();               // Initialise LCD
  lcd.backlight();          // Enable backlight
  
  if (!rtc.begin()) {                    // Check if RTC is available
    Serial.println("Couldn't find RTC");
    lcd.setCursor(0, 0);                 // Print error msg on RTC failure
    lcd.print("Couldn't find RTC");
    delay(500);
  }         

  lcd.setCursor(6, 0);      // Print welcome msg and company name on startup
  lcd.print("Welcome");
  delay(500);

  lcd.setCursor(8, 1);
  lcd.print("To");
  delay(500);

  lcd.setCursor(5, 2);
  lcd.print("Vedantrik");
  delay(500);

  lcd.setCursor(4, 3);
  lcd.print("Technologies");
  delay(3000);

  Serial.begin(115200);  // Begin the serial communication at 115200 bps

  for (uint8_t i = 0; i < 3; i++) {                 // Initialise the shift register 1 pins as output
    pinMode(shiftRegister1[i], OUTPUT);
  }

  for (uint8_t i = 0; i < 3; i++) {                 // Initialise the shift register 2 pins as output
    pinMode(shiftRegister2[i], OUTPUT);
  }

  WiFi.mode(WIFI_AP);                  // Initialize ESP32 in AP Mode
  WiFi.softAPConfig(ip, ip, netmask);  // Passing the IP Address, Gateway Address (same as IP Address), and Netmask
  WiFi.softAP(ssid, password);         // Setting up the NodeMCU in AP Mode with Credentials defined above

}

//-----------------------------------------------SETUP FUNCTION----------------------------------------------------------------------------------------------------------//



//--------------------------------------------FUNCTION DECLARATIONS------------------------------------------------------------------------------------------------------//

void TCA9548A(uint8_t var, uint8_t bus) {             //-----> Function that selects the I2C mux and selects a channel
  Wire.beginTransmission(muxArray[var]);                                              // TCA9548A address is selected by the for loop
  Serial.println("I2C Mux Addr: 0x" + String(muxArray[var], HEX) + " bus: " + bus);
  Wire.write(1 << bus);                                                               // send byte to select bus
  Wire.endTransmission();                                                             // End the transmission
}

void displayTemp(uint8_t var){                        //-----> Function that displays the temperature readings on the LCD
  if (var < 4){                                                      // Prints the 1st column on left of the LCD display
  lcd.setCursor(0, var);                                             // var acts as a row selector from 0 to 3
  lcd.print(String(var+1) + ")" + String(temperatureArray[var]));
  }
  else{                                                              // Prints the 2nd column on right of the LCD display
  lcd.setCursor(10, var%4);                                          // var acts as a row selector from 4 to 7 as 4%4 = 0, 5%4 = 1... & so on
  lcd.print(String(var+1) + ")" + String(temperatureArray[var]));
  }
}

void getReadings(uint8_t var) {                       //-----> Test function to check if all ADS1115 work
  ADS1115 ADS1(adsArray[var]);                        // Initialise ADS1115 with an address
  
  ADS1.begin();                                       // Initialise the ADS object
  ADS1.setGain(0);                                    // Set gain to 6.144v
  int16_t val_01 = ADS1.readADC_Differential_0_1();   // Get the differential ADC value
  float volts_01 = ADS1.toVoltage(val_01);            // Convert ADC value to voltage
  // Serial.print("\tval_01: ");                      // Serial print the ADC values and voltage
  // Serial.print(val_01);
  // Serial.print("\t");
  // Serial.println(volts_01, 3);
}

void activateLine(const uint8_t shiftRegArr[]) {      //-----> Function that activates the shift register controlled Lead line
  digitalWrite(shiftRegArr[0], LOW);                                  // ST_CP LOW to keep LEDs from changing while reading serial data
  shiftOut(shiftRegArr[2], shiftRegArr[1], MSBFIRST, dataShift[0]);   // Send the data to the Shift Register
  Serial.println(dataShift[0], BIN);                                  // Print data in Binary to show the data shifted
  digitalWrite(shiftRegArr[0], HIGH);                                 // ST_CP HIGH to show the data
}

void activateRTD(const uint8_t shiftRegArr[]) {       //-----> Function that activates the shift register controlled RTD line
  digitalWrite(shiftRegArr[0], LOW);                                  // ST_CP LOW to keep LEDs from changing while reading serial data
  shiftOut(shiftRegArr[2], shiftRegArr[1], MSBFIRST, dataShift[1]);   // Send the data to the Shift Register
  Serial.println(dataShift[1], BIN);                                  // Print data in Binary to show the data shifted
  digitalWrite(shiftRegArr[0], HIGH);                                 // ST_CP HIGH to show the data
}

float RTDReader(const uint8_t var) {                  //-----> Function that calculates the RTD Resistance
  ADS1115 ADS1(adsArray[var]);                                                        // Initialise ADS1115 with an address
  Serial.println("CH" + String(var) + " Addr: 0x" + String(adsArray[var], HEX));
  ADS1.begin();                                                                       // Initialise the ADS object
  ADS1.setGain(0);                                                                    // Set gain to 6.144v
  int16_t val_02 = ADS1.readADC_Differential_0_2();                                   // Get the ADC value
  float volts_02 = ADS1.toVoltage(val_02);                                            // Convert the ADC value into voltage
  // volts_02 = sqrt(volts_02 * volts_02);
  float I = volts_02 / 300.0;                                                         // Calculate current by dividing V/R
  // Serial.print("vrtd: ");
  // Serial.print("\t");
  // Serial.println(volts_02, 6);
  // Serial.print("\I: ");
  // Serial.print("\t");
  // Serial.println(I, 6);

  int16_t val_01 = ADS1.readADC_Differential_0_1();                                   // Get the ADC value
  float volts_01 = ADS1.toVoltage(val_01);                                            // Convert the ADC value into voltage
  // volts_01 = sqrt(volts_01 * volts_01);
  // Serial.print("vload: ");
  // Serial.print("\t");
  // Serial.println(volts_01, 6);
  float RTD = (volts_01 / I) - (0.0121 * (volts_01 / I));                             // Calculate the RTD resistance by subtracting the correction factor
  // Serial.print("\RTD: ");
  // Serial.print("\t");
  // Serial.println(RTD, 6);
  return RTD;                                                                         // Return the RTD resistance value
}

float LeadReader(const uint8_t var){                  //-----> Function that calculates lead resistance and returns temperature from all channels
  ADS1115 ADS1(adsArray[var]);                                                        // Initialise ADS1115 with an address
  Serial.println("CH" + String(var) + " Addr: 0x" + String(adsArray[var], HEX));
  ADS1.begin();                                                                       // Initialise the ADS object
  ADS1.setGain(0);                                                                    // Set gain to 6.144v
  int16_t val_02 = ADS1.readADC_Differential_0_2();                                   // Get the ADC value
  float volts_02 = ADS1.toVoltage(val_02);                                            // Convert the ADC value into voltage
  // volts_02 = sqrt(volts_02 * volts_02);
  float I = volts_02 / 300.0;                                                         // Calculate current by dividing V/R
  // Serial.print("\I: ");
  // Serial.print("\t");
  // Serial.println(I, 6);

  int16_t val_03 = ADS1.readADC_Differential_0_3();                                   // Get the ADC value
  float volts_03 = ADS1.toVoltage(val_03);                                            // Convert the ADC value into voltage
  // volts_03 = sqrt(volts_03 * volts_03);
  // Serial.print("v03: ");
  // Serial.print("\t");
  // Serial.println(volts_03, 6);
  float lead = (volts_03 / I)-(0.121*(volts_03 / I));                                 // Calculate the Lead resistance by subtracting the correction factor
  // Serial.print("\lead: ");
  // Serial.print("\t");
  // Serial.println(lead, 6);

  float t = sqrt((rtdArray[var] - lead) * (rtdArray[var] - lead));                    // Calculate the Temperature by subtracting the Lead resistance from RTD resistance
  // Serial.println(t, 6);
  // Serial.println();
  // Serial.print("t: ");
  // Serial.print("\t");
  // Serial.println((((t - 100.0) / 100.0) / 0.00385055), 6);
  return t;                                                                           // Return the temperature value
  
}

void LogValue() {                                     //-----> Function that saves to the Flash Memory

  DateTime now = rtc.now();                                                   // Re-initialise the now Datetime object

  float dd = (float)now.day();                                                // Storing the date as a float
  float mm = (float)now.month();                                              // Storing the month as a float
  float yy = (float)now.year();                                               // Storing the year as a float
  float HH = (float)now.hour();                                               // Storing the hours as a float
  float MM = (float)now.minute();                                             // Storing the minutes as a float

  if (String(memory.readFloat(LastLocation)) == "nan") {                      // Check if the data at the 0th index is nan to decide whether to save the Device ID or not
    memory.writeFloat(LastLocation + 1 * sizeof(float), (float)Device_ID);    // Write the Device ID into the 4th index in the Flash Memory
    Boot = false;                                                             // Render Boot false since the Device ID is saved
  }

  if (Boot) {                                                                 // Checks if the ESP is Reset
    for (uint32_t addr = 16; addr < memory.getCapacity(); addr += 4) {        // For loop to iterate over the data
      if (String(memory.readFloat(addr)) == "nan") {                          // Checks if the data at 16th index is nan
        address = addr;                                                       // 
        break;
      }
    }
    Boot = false;
  }

  for(uint32_t i = 0; i < channels; i++){
    memory.writeFloat(address+i, temperatureArray[i]);                  // Writing the Temperature into the memory location
  }
  
  memory.writeFloat(address + 8 * sizeof(float), HH);        // Writing the Hours into the memory location
  memory.writeFloat(address + 9 * sizeof(float), MM);        // Writing the Minutes into the memory location
  memory.writeFloat(address + 10 * sizeof(float), dd);       // Writing the Date into the memory location
  memory.writeFloat(address + 11 * sizeof(float), mm);       // Writing the Month into the memory location
  memory.writeFloat(address + 12 * sizeof(float), yy);       // Writing the Year into the memory location

  memory.writeFloat(LastLocation, address);                  // Writing adress at the 0th index to just indicate that configuraton process is complete
  address += 13 * sizeof(float);                             // Incrementing the adress variable by 12 locations since we wrote Temp, Hours and Minutes
}

//--------------------------------------------FUNCTION DECLARATIONS------------------------------------------------------------------------------------------------------//



//-----------------------------------------------LOOP FUNCTION-----------------------------------------------------------------------------------------------------------//

void loop() {

  // Updates frequently
  unsigned long currentTime = millis();         // Get the current instance in milliseconds

  DateTime now = rtc.now();                     // Re-initialise the RTC module
  float MM = (float)now.minute();               // Storing the minutes as a float

  // if ((MM == (float)0) && (logState0 == 0)) {
  //   logState0 = true;
  //   logState30 = false;

  //   LogValue();                                 // Calling the LogValue function to save the readings into the Flash memory
  // } else if ((MM == (float)30) && (logState30 == 0)) {
  //   logState30 = true;
  //   logState0 = false;

  //   LogValue();                                 // Calling the LogValue function to save the readings into the Flash memory
  // } else {
  // }

  lcd.clear();
  for(uint8_t i = 0; i < channels; i++){
    displayTemp(i);
  }

  activateRTD(shiftRegister1);            // Enable RTD line via Shift Register 1
  activateRTD(shiftRegister2);            // Enable RTD line via Shift Register 2
  delay(2000);                            // Wait for 2 secs to stabilise the voltage on pins
  for(uint8_t i = 0; i < channels; i++){
    TCA9548A(0,i);
    rtdArray[i] = RTDReader(i);           // Read and save all the RTD resistance values in the rtdArray Array
  }

  activateLine(shiftRegister1);           // Enable Lead line via Shift Register 1
  activateLine(shiftRegister2);           // Enable Lead line via Shift Register 2
  delay(2000);                            // Wait for 2 secs to stabilise the voltage on pins
  for(uint8_t i = 0; i < channels; i++){
    TCA9548A(0,i);
    temperatureArray[i] = LeadReader(i);  // Calculate the temperature values and store in the temperature array
  }
}

//-----------------------------------------------LOOP FUNCTION-----------------------------------------------------------------------------------------------------------//