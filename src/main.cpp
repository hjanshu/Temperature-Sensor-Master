#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal.h>
#include <EEPROM.h>
#include <BigNumbers.h>
#include <Adafruit_MLX90614.h>
#include <I2C_Anything.h>

const byte SLAVE_ADDRESS = 42;

#define btnNONE 0
#define btnRIGHT 1
#define btnUP 2
#define btnDOWN 3
#define btnLEFT 4
#define btnSELECT 5

char buffer[50];

//const char MENU1[] PROGMEM = "SET DATE & TIME";
const char MENU1[] PROGMEM = "SET MAX TEMP   ";
const char MENU2[] PROGMEM = "SET TEMP UNIT  ";
const char MENU3[] PROGMEM = "SET BUZZER TIME";
//const char MENU6[] PROGMEM = "SET MOTOR MODE";

const char *const mainMenu[] PROGMEM = {MENU1, MENU2, MENU3};

//RTC_DS3231 rtc;
//TEST
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
BigNumbers bigNum(&lcd);

void showHomeScreen(void);
byte key_press(void);
byte read_LCD_buttons(void);
void check_button_input(void);
void displayMainMenu(char);
void scrollMainMenu(void);
void selectMenuOption(void);
void checkIRTrig(void);
void changeMaxTempScreen(void);
void changeMaxTemp(void);
void changeUnitScreen(void);
void changeUnit(void);
void changeBuzzerScreen(void);
void changeBuzzer(void);

boolean mainMenuFlag = false;
boolean setTempFlag = false;
boolean setUnitFlag = false;
boolean setBuzzFlag = false;

int mainMenupos = 0;
int cursor = 0;

float temperature;
int tempA;
int tempB;
float maxTemp;
float maxTempVar;
int tempUnit;
int tempUnitVar;
int buzzerTime;
int buzzerTimeVar;

#define buzzerPin 3
#define IRPin 2
int ledPin = 11;

#define maxTempAddr 0
#define tempUnitAddr 3
#define buzzerTimeAddr 5

//////////////////////////////////////////////////////////////////////////
// debounce a button
//////////////////////////////////////////////////////////////////////////
int counter = 0;          // how many times we have seen new value
long previous_time = 0;   // the last time the output pin was sampled
byte debounce_count = 50; // number of millis/samples to consider before declaring a debounced input
byte current_state = 0;   // the debounced input value

void setup()
{
  pinMode(buzzerPin, OUTPUT);
  pinMode(IRPin, INPUT);
  pinMode(ledPin, OUTPUT);

  TCCR1B = TCCR1B & 0b11111000 | 0x01;
  Serial.begin(115200);
  Wire.begin();
  lcd.begin(16, 2);
  mlx.begin();
  bigNum.begin();

  //   for (int i = 0; i < 1024; i++)
  //  {
  //    EEPROM.update(i, 0);
  //  }
  //
  //  EEPROM.update(maxTempAddr, 95.1);

  lcd.setCursor(0, 0);
  lcd.print("  CAMSOL ENGG  ");
  lcd.setCursor(0, 1);
  lcd.print("   SOLUTIONS   ");
  delay(1000);
  lcd.clear();
}

void loop()
{
  checkIRTrig();
  if (!mainMenuFlag && !setTempFlag && !setUnitFlag && !setBuzzFlag)
  {
    showHomeScreen();
  }
  check_button_input();
}

void checkIRTrig()
{
  maxTempVar = EEPROM.read(maxTempAddr);
  // Serial.println(maxTemp);
  tempUnitVar = EEPROM.read(tempUnitAddr);
  buzzerTimeVar = EEPROM.read(buzzerTimeAddr);
  float currTemp;
  if (digitalRead(IRPin) == LOW)
  {
    lcd.clear();
    delay(500);
    digitalWrite(buzzerPin, HIGH);
    temperature = mlx.readObjectTempF();
    if (tempUnitVar == 0)
    {
      currTemp = mlx.readObjectTempC();
    }
    else if (tempUnitVar == 1)
    {
      currTemp = mlx.readObjectTempF();
    }

    tempA = floor(currTemp);
    tempB = (currTemp - tempA) * pow(10, 2);

    bigNum.displayLargeInt(tempA, 0, 3, false);
    lcd.write(".");
    lcd.print(tempB);
    lcd.write(" ");
    lcd.print((char)223);
    if (tempUnitVar == 0)
      lcd.print("C");
    else if (tempUnitVar == 1)
      lcd.print("F");
    //String tempT = String(currTemp, 1);
    Wire.beginTransmission(SLAVE_ADDRESS);
    I2C_writeAnything(temperature);
    I2C_writeAnything(currTemp);
    I2C_writeAnything(tempUnitVar);
    I2C_writeAnything(maxTempVar);
    I2C_writeAnything(buzzerTimeVar);
    Wire.endTransmission();

    delay(200);
    digitalWrite(buzzerPin, LOW);
    if (temperature > maxTempVar)
    {
      Wire.write('x');
      Wire.write(buzzerTimeVar);
      digitalWrite(buzzerPin, HIGH);
      digitalWrite(ledPin, HIGH);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("   HIGH TEMP   ");
      lcd.setCursor(0, 1);
      lcd.print("    DETECTED   ");
      delay(buzzerTimeVar * 1000);
      digitalWrite(buzzerPin, LOW);
      digitalWrite(ledPin, LOW);
    }

    delay(2000);
  }
}

void showHomeScreen()
{
  lcd.setCursor(0, 0);
  lcd.print("  CAMSOL ENGG  ");
  lcd.setCursor(0, 1);
  lcd.print("   SOLUTIONS   ");
}

byte read_LCD_buttons()
{                                 // read the buttons
  int adc_key_in = analogRead(0); // read the value from the sensor

  //value read: 0(0V), 130(0.64V), 306(1.49V), 479(2.33V), 722(3.5V), 1023(4.97V)
  /* if (adc_key_in > 1000)
    return btnNONE;
  if (adc_key_in < 60)
    return btnRIGHT;
  if (adc_key_in < 200)
    return btnDOWN;
  if (adc_key_in < 400)
    return btnUP;
  if (adc_key_in < 600)
    return btnLEFT;
  if (adc_key_in < 800)
    return btnSELECT;
  return btnNONE; */

  //TEST
  if (adc_key_in > 1000)
    return btnNONE;
  if (adc_key_in < 580 && adc_key_in > 480)
    return btnRIGHT;
  if (adc_key_in < 280 && adc_key_in > 150)
    return btnDOWN;
  if (adc_key_in < 380 && adc_key_in > 280)
    return btnUP;
  if (adc_key_in < 640 && adc_key_in > 580)
    return btnLEFT;
  if (adc_key_in < 1000 && adc_key_in > 900)
    return btnSELECT;
  return btnNONE;
}

byte key_press()
{
  // If we have gone on to the next millisecond
  if (millis() != previous_time)
  {
    byte this_button = read_LCD_buttons();

    if (this_button == current_state && counter > 0)
      counter--;

    if (this_button != current_state)
      counter++;

    // If the Input has shown the same value for long enough let's switch it
    if (counter >= debounce_count)
    {
      counter = 0;
      current_state = this_button;
      return this_button;
    }
    previous_time = millis();
  }
  return 0;
}

void check_button_input()
{

  if (!mainMenuFlag && !setTempFlag && !setUnitFlag && !setBuzzFlag)
  {
    byte lcd_key = key_press();
    switch (lcd_key)
    {
    case btnLEFT:
    {
      mainMenuFlag = true;
      displayMainMenu('H');
      break;
    }
    case btnRIGHT:
    {
      break;
    }

    default:
      break;
    }
  }
  else if (mainMenuFlag)
  {
    scrollMainMenu();
  }

  else if (setTempFlag)
  {
    changeMaxTemp();
  }
  else if (setUnitFlag)
  {
    //setSchedBell('W');
    changeUnit();
  }
  else if (setBuzzFlag)
  {
    //etSchedBell('E');
    changeBuzzer();
  }

  return;
}

void scrollMainMenu()
{
  byte lcd_key = key_press(); // read the buttons
  switch (lcd_key)
  {
  case btnLEFT:
  {
    selectMenuOption();
    break;
  }
  case btnDOWN:
  {
    displayMainMenu('D');
    break;
  }
  case btnUP:
  {
    displayMainMenu('U');
    break;
  }
  case btnRIGHT:
  {
    mainMenuFlag = false;
    mainMenupos = 0;
    lcd.clear();
    break;
  }
  }
  return;
}

void selectMenuOption()
{
  if (mainMenupos == 0)
  {
    setTempFlag = true;
    mainMenuFlag = false;
    cursor = 0;
    maxTemp = EEPROM.read(maxTempAddr);
    changeMaxTempScreen();
  }
  else if (mainMenupos == 1)
  {
    setUnitFlag = true;
    mainMenuFlag = false;
    tempUnit = EEPROM.read(tempUnitAddr);
    changeUnitScreen();
  }
  else if (mainMenupos == 2)
  {
    setBuzzFlag = true;
    mainMenuFlag = false;
    buzzerTime = EEPROM.read(buzzerTimeAddr);
    changeBuzzerScreen();
  }

  return;
}

void displayMainMenu(char c)
{
  int fisrtLinePos;
  int scndLinePos;
  if (c == 'H')
  {
    fisrtLinePos = mainMenupos;
    scndLinePos = fisrtLinePos + 1;
  }
  else if (c == 'D')
  {
    mainMenupos++;
    if (mainMenupos <= 2)
    {
      fisrtLinePos = mainMenupos;
      scndLinePos = fisrtLinePos + 1;
    }
    else if (mainMenupos > 2)
    {
      mainMenupos = 0;
      fisrtLinePos = mainMenupos;
      scndLinePos = fisrtLinePos + 1;
    }
  }
  else if (c == 'U')
  {
    mainMenupos--;
    if (mainMenupos >= 0)
    {
      fisrtLinePos = mainMenupos;
      scndLinePos = fisrtLinePos + 1;
    }
    else if (mainMenupos < 0)
    {
      mainMenupos = 2;
      fisrtLinePos = mainMenupos;
      scndLinePos = fisrtLinePos + 1;
    }
  }

  lcd.clear();
  lcd.noCursor();
  lcd.noBlink();
  lcd.setCursor(0, 0);
  lcd.print(">");
  lcd.setCursor(1, 0);
  strcpy_P(buffer, (char *)pgm_read_word(&(mainMenu[fisrtLinePos])));
  lcd.print(buffer);
  lcd.setCursor(0, 1);
  lcd.print(" ");
  lcd.setCursor(1, 1);
  if (scndLinePos == 3)
  {
    lcd.print("                ");
  }
  else
  {
    strcpy_P(buffer, (char *)pgm_read_word(&(mainMenu[scndLinePos])));
    lcd.print(buffer);
  }
}

void changeMaxTempScreen()
{
  float tempC = (maxTemp - 32) * 0.55;
  lcd.noCursor();
  lcd.noBlink();
  lcd.setCursor(0, 0);
  lcd.print("SET MAX TEMP  ");
  lcd.setCursor(0, 1);
  lcd.print(maxTemp);
  lcd.print((char)223);
  lcd.print("F  ");
  lcd.setCursor(9, 1);
  lcd.print(tempC);
  lcd.print((char)223);
  lcd.print("C  ");
  lcd.blink();
}

void changeMaxTemp()
{
  byte lcd_key = key_press(); // read the buttons
  switch (lcd_key)
  {
  case btnUP:
  {
    maxTemp = maxTemp + 0.1;
    changeMaxTempScreen();
    break;
  }
  case btnDOWN:
  {
    if (maxTemp > 0)
    {
      maxTemp = maxTemp - 0.1;
    }
    changeMaxTempScreen();
    break;
  }

  case btnRIGHT:
  {
    setTempFlag = false;
    mainMenuFlag = true;
    displayMainMenu('H');
    //changeModeScreen();
    break;
  }
  case btnLEFT:
  {

    EEPROM.update(maxTempAddr, maxTemp);

    setTempFlag = false;
    mainMenuFlag = true;
    lcd.clear();
    lcd.noBlink();
    lcd.noCursor();
    lcd.setCursor(0, 0);
    lcd.print("******DATA******");
    lcd.setCursor(0, 1);
    lcd.print("****UPDATED*****");
    delay(1000);
    displayMainMenu('H');
  }

  default:
    break;
  }
}

void changeUnitScreen()
{
  lcd.noCursor();
  lcd.noBlink();
  lcd.setCursor(0, 0);
  lcd.print("SET TEMP UNIT");
  lcd.setCursor(0, 1);
  lcd.print(" ");
  if (tempUnit == 0)
  {
    lcd.print((char)223);
    lcd.print("C   CELSIUS   ");
  }
  else if (tempUnit == 1)
  {
    lcd.print((char)223);
    lcd.print("F   FARENHEITE ");
  }
  lcd.setCursor(cursor, 1);
  lcd.blink();
}

void changeUnit()
{
  byte lcd_key = key_press(); // read the buttons
  switch (lcd_key)
  {
  case btnUP:
  {
    if (tempUnit == 1)
    {
      tempUnit = 0;
    }
    else if (tempUnit == 0)
    {
      tempUnit = 1;
    }
    changeUnitScreen();
    break;
  }
  case btnDOWN:
  {
    if (tempUnit == 1)
    {
      tempUnit = 0;
    }
    else if (tempUnit == 0)
    {
      tempUnit = 1;
    }
    changeUnitScreen();
    break;
  }

  case btnRIGHT:
  {
    setUnitFlag = false;
    mainMenuFlag = true;
    displayMainMenu('H');
    changeUnitScreen();
    break;
  }
  case btnLEFT:
  {

    EEPROM.update(tempUnitAddr, tempUnit);

    setUnitFlag = false;
    mainMenuFlag = true;
    lcd.clear();
    lcd.noBlink();
    lcd.noCursor();
    lcd.setCursor(0, 0);
    lcd.print("******DATA******");
    lcd.setCursor(0, 1);
    lcd.print("****UPDATED*****");
    delay(1000);
    displayMainMenu('H');
  }

  default:
    break;
  }
}
void changeBuzzerScreen()
{
  lcd.noCursor();
  lcd.noBlink();
  lcd.setCursor(0, 0);
  lcd.print("SET Buzzer Time");
  lcd.setCursor(0, 1);
  lcd.print(buzzerTime);
  lcd.print(" SEC           ");
  lcd.setCursor(cursor, 1);
  lcd.blink();
}

void changeBuzzer()
{
  byte lcd_key = key_press(); // read the buttons
  switch (lcd_key)
  {
  case btnUP:
  {
    buzzerTime++;
    changeBuzzerScreen();
    break;
  }
  case btnDOWN:
  {
    if (buzzerTime > 0)
    {
      buzzerTime--;
    }
    changeBuzzerScreen();
    break;
  }

  case btnRIGHT:
  {
    setBuzzFlag = false;
    mainMenuFlag = true;
    displayMainMenu('H');
    changeBuzzerScreen();
    break;
  case btnLEFT:
  {

    EEPROM.update(buzzerTimeAddr, buzzerTime);

    setBuzzFlag = false;
    mainMenuFlag = true;
    lcd.clear();
    lcd.noBlink();
    lcd.noCursor();
    lcd.setCursor(0, 0);
    lcd.print("******DATA******");
    lcd.setCursor(0, 1);
    lcd.print("****UPDATED*****");
    delay(1000);
    displayMainMenu('H');
  }
  }

  default:
    break;
  }
}
