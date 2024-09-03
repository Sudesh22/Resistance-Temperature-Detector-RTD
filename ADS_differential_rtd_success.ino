#include <ADS1X15.h>
#include <LiquidCrystal_I2C.h>

ADS1115 ADS(0x48);
ADS1115 ADS2(0x49);
LiquidCrystal_I2C lcd(0x27, 20, 4);

#define rtdline 2
#define rtdline2 4
#define leadline 10
#define leadline2 12

void setup() {
  Serial.begin(115200);
  Serial.println(__FILE__);
  Serial.print("ADS1X15_LIB_VERSION: ");
  Serial.println(ADS1X15_LIB_VERSION);

  Wire.begin();
  lcd.init();
  lcd.backlight();

  ADS.begin();
  ADS.setGain(0);
  ADS2.begin();
  ADS2.setGain(0);
  pinMode(rtdline, OUTPUT);
  pinMode(leadline, OUTPUT);
  pinMode(rtdline2, OUTPUT);
  pinMode(leadline2, OUTPUT);
}


void loop() {
  digitalWrite(rtdline, 1);                               /////////////////////////////////////////////////////////////////
  delay(2000);
  int16_t val_02 = ADS.readADC_Differential_0_2();
  float volts_02 = ADS.toVoltage(val_02);
  // volts_02 = sqrt(volts_02 * volts_02);
  float I = volts_02 / 300.0;
  Serial.print("vrtd: ");
  Serial.print("\t");
  Serial.println(volts_02, 6);
  Serial.print("\I: ");
  Serial.print("\t");
  Serial.println(I, 6);

  int16_t val_01 = ADS.readADC_Differential_0_1();                                                             
  float volts_01 = ADS.toVoltage(val_01);
  // volts_01 = sqrt(volts_01 * volts_01);
  Serial.print("vload: ");
  Serial.print("\t");                       
  Serial.println(volts_01, 6);
  float RTD = (volts_01 / I)-(0.0121*(volts_01 / I));
  Serial.print("\RTD: ");
  Serial.print("\t");
  Serial.println(RTD, 6);

  digitalWrite(rtdline, 0);                                 //////////////////////////////////////////////////////////////

  delay(50);

  digitalWrite(leadline, 1);                                ////////////////////////////////////////////////////////////////
  delay(2000);
  val_02 = ADS.readADC_Differential_0_2();
  volts_02 = ADS.toVoltage(val_02);
  // volts_02 = sqrt(volts_02 * volts_02);
  I = volts_02 / 300.0;
  Serial.print("\I: ");
  Serial.print("\t");
  Serial.println(I, 6);

  int16_t val_03 = ADS.readADC_Differential_0_3();
  float volts_03 = ADS.toVoltage(val_03);
  // volts_03 = sqrt(volts_03 * volts_03);
  Serial.print("v03: ");
  Serial.print("\t");
  Serial.println(volts_03, 6);
  float lead = (volts_03 / I)-(0.121*(volts_03 / I));
  Serial.print("\lead: ");
  Serial.print("\t");
  Serial.println(lead, 6);

  digitalWrite(leadline, 0);                                   ///////////////////////////////////////////////////

  Serial.print("\R: ");
  Serial.print("\t");
  float t = sqrt((RTD - lead) * (RTD - lead));
  Serial.println(t, 6);
  Serial.println();
  Serial.print("t: ");
  Serial.print("\t");
  Serial.println((((t - 100.0) / 100.0) / 0.00385055), 6);


////////////////////////////////////////////////////////////////////////// -------CH2-------- ///////////////////////////////////////////////////////////////////////////


  digitalWrite(rtdline2, 1);                               /////////////////////////////////////////////////////////////////
  delay(2000);
  int16_t val_02CH2 = ADS2.readADC_Differential_0_2();
  float volts_02CH2 = ADS2.toVoltage(val_02CH2);
  // volts_02CH2 = sqrt(volts_02CH2 * volts_02CH2);
  float ICH2 = volts_02CH2 / 300.0;
  Serial.print("vrtd: ");
  Serial.print("\t");
  Serial.println(volts_02CH2, 6);
  Serial.print("\I: ");
  Serial.print("\t");
  Serial.println(ICH2, 6);

  int16_t val_01CH2 = ADS2.readADC_Differential_0_1();                                                             
  float volts_01CH2 = ADS2.toVoltage(val_01CH2);
  // volts_01CH2 = sqrt(volts_01CH2 * volts_01CH2);
  Serial.print("vload: ");
  Serial.print("\t");                       
  Serial.println(volts_01CH2, 6);
  float RTDCH2 = (volts_01CH2 / ICH2)-(0.0121*(volts_01CH2 / ICH2));
  Serial.print("\RTD: ");
  Serial.print("\t");
  Serial.println(RTDCH2, 6);

  digitalWrite(rtdline2, 0);                                 //////////////////////////////////////////////////////////////

  delay(50);

  digitalWrite(leadline2, 1);                                ////////////////////////////////////////////////////////////////
  delay(2000);
  val_02CH2 = ADS2.readADC_Differential_0_2();
  volts_02CH2 = ADS2.toVoltage(val_02CH2);
  // volts_02CH2 = sqrt(volts_02CH2 * volts_02CH2);
  ICH2 = volts_02CH2 / 300.0;
  Serial.print("\I: ");
  Serial.print("\t");
  Serial.println(ICH2, 6);

  int16_t val_03CH2 = ADS2.readADC_Differential_0_3();
  float volts_03CH2 = ADS2.toVoltage(val_03CH2);
  // volts_03CH2 = sqrt(volts_03CH2 * volts_03CH2);
  Serial.print("v03: ");
  Serial.print("\t");
  Serial.println(volts_03CH2, 6);
  float leadCH2 = (volts_03CH2 / ICH2)-(0.121*(volts_03CH2 / ICH2));
  Serial.print("\lead: ");
  Serial.print("\t");
  Serial.println(leadCH2, 6);

  digitalWrite(leadline2, 0);                                   ///////////////////////////////////////////////////

  Serial.print("\R: ");
  Serial.print("\t");
  float tCH2 = sqrt((RTDCH2 - leadCH2) * (RTDCH2 - leadCH2));
  Serial.println(tCH2, 6);
  Serial.println();
  Serial.print("t: ");
  Serial.print("\t");
  Serial.println((((tCH2 - 100.0) / 100.0) / 0.00385055), 6);
  // int16_t val_13 = ADS.readADC_Differential_1_3();
  // int16_t val_23 = ADS.readADC_Differential_2_3();
  // float volts_13 = ADS.toVoltage(val_13);
  // float volts_23 = ADS.toVoltage(val_23);

  // Serial.print("\tval_01: ");
  // Serial.print(val_01);
  // Serial.print("\t");
  // Serial.println(volts_01, 6);
  // Serial.print("\tval_02: ");
  // Serial.print(val_02);
  // Serial.print("\t");
  // Serial.println(volts_02, 6);
  // Serial.print("\tval_03: ");
  // Serial.print(val_03);
  // Serial.print("\t");
  // Serial.println(volts_03, 6);
  lcd.clear();
  lcd.setCursor(5, 0);
  lcd.print("CH1");
  lcd.setCursor(13, 0);
  lcd.print("CH2");
  lcd.setCursor(0, 1);
  lcd.print("RTD");
  lcd.setCursor(0, 2);
  lcd.print("Lead");
  lcd.setCursor(0, 3);
  lcd.print("Temp");

  lcd.setCursor(5, 1);
  lcd.print(RTD, 2);
  lcd.setCursor(5, 2);
  lcd.print(lead, 2);
  lcd.setCursor(5, 3);
  lcd.print((((t - 100.0) / 100.0) / 0.00385055), 2);
  lcd.setCursor(13, 1);
  lcd.print(RTDCH2, 2);
  lcd.setCursor(13, 2);
  lcd.print(leadCH2, 2);
  lcd.setCursor(13, 3);
  lcd.print((((tCH2 - 100.0) / 100.0) / 0.00385055), 2);

  // // Serial.print("\tval_13: "); Serial.print(val_13); Serial.print("\t"); Serial.println(volts_13, 3);
  // // Serial.print("\tval_23: "); Serial.print(val_23); Serial.print("\t"); Serial.println(volts_23, 3);
  // Serial.println();
  delay(100);
}


//  -- END OF FILE --
