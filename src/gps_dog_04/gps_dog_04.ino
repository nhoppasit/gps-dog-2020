/*
  26-05-2020
*/
#define DEVICE_NAME "SENSOR TAG PROJECT 2:45 PM 7/23/2020"
//
#define _S1_ETX_TRACE_ 0
#define _S2_ETX_TRACE_ 0
#define _S3_ETX_TRACE_ 0
//
#define EEPROM_LORA 0
#define EEPROM_RS485 100
#define EEPROM_TAG_ID 200
//
#define CMD_INFO "?"
#define CMD_TRACE "D"
#define CMD_BLINK_OFF "b"
#define CMD_BLINK_ON "B"
#define CMD_SET_EEPROM "@"
#define CMD_GET_EEPROM "&"
#define CMD_PRT_DHT "T"
#define CMD_OFF_DHT "t"
#define CMD_FORWORD "F"
#define CMD_QUERY "Q"
#define CMD_SET "S"
//
#define STX ':'
#define ETX '\n'
#define STX2 '$'
#define ETX2 '\n'
#define STX3 '$'
#define ETX3 '\n'
//
#include "DHT.h"
#include <EEPROM.h>
//
#define DHTPIN PB9
//#define DHTTYPE DHT11   // DHT 11
#define DHTTYPE DHT22 // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)
DHT dht(DHTPIN, DHTTYPE);
const int DHT_TIME = 2000;
unsigned long t0Dht = 0;
bool DHT_FLAG = true;
float RHumidity;
float Temperature;
float HeatIndex;
//
String inputString = "";     // a String to hold incoming data
bool stringComplete = false; // whether the string is complete
bool STX_COME = false;
//
String inputString2 = "";     // a String to hold incoming data
bool stringComplete2 = false; // whether the string is complete
bool STX_COME2 = false;
//
String inputString3 = "";     // a String to hold incoming data
bool stringComplete3 = false; // whether the string is complete
bool STX_COME3 = false;
//
unsigned long t0Blink = 0;
bool blinkState = 0;
bool blinkFlag = false;
int blinkTime = 500;
//
#define Tx485Control PB12
#define Tx485Control2 PB13
#define RS485Transmit HIGH
#define RS485Receive LOW
//
unsigned long t0A0A1 = 0;
bool A0A1_FLAG = true;
//
const int ADC_TIME = 100; /*ms*/
const int MAX_ADC_READINGS = 10;
float VREF = 0;
int A0A1ReadIndex = 0; // the index of the current reading
//
int Adc0Value = 0;
float Voltage0 = 0;
float V0Readings[MAX_ADC_READINGS]; // the Adc0Readings from the analog input
float V0Total = 0;                  // the running Adc0Total
//
int Adc1Value = 0;
float Voltage1 = 0;
float V1Readings[MAX_ADC_READINGS]; // the Adc0Readings from the analog input
float V1Total = 0;                  // the running Adc0Total
//
bool TRACE_BLINK = false;
bool TRACE_ADC = false;
bool TRACE_RS485 = false;
//
/*
  -----------------------------------------------------------------------------
  EEPROM
  -----------------------------------------------------------------------------
*/
#pragma region EEPROM

#define S3_SENDER_IDX 0
#define LEN_ADDR 3
#define S3_RECEIVER_IDX S3_SENDER_IDX + LEN_ADDR
#define S3_CMD_KEY_IDX S3_RECEIVER_IDX + LEN_ADDR
#define S3_DATA_IDX S3_CMD_KEY_IDX + 1
#define LEN_ID 10

char LORA_ADDRH = 0x00;
char LORA_ADDRL = 0x00;
char LORA_CHAN = 0x18;
String _ADDR_ = "FFF";
String _ID_ = "0000000000";
String SENDER_ADDR = "";
const int EEPROM_MIN_ADDR = 0;
const int EEPROM_MAX_ADDR = 4095;
boolean eeprom_is_addr_ok(int addr)
{
  return ((addr >= EEPROM_MIN_ADDR) && (addr <= EEPROM_MAX_ADDR));
}

boolean eeprom_write_bytes(int startAddr, const byte *array, int numBytes)
{
  int i;
  if (!eeprom_is_addr_ok(startAddr) || !eeprom_is_addr_ok(startAddr + numBytes))
  {
    return false;
  }
  for (i = 0; i < numBytes; i++)
  {
    EEPROM.write(startAddr + i, array[i]);
  }
  return true;
}

boolean eeprom_write_string(int addr, const char *string)
{
  int numBytes; // actual number of bytes to be written
  //write the string contents plus the string terminator byte (0x00)
  numBytes = strlen(string) + 1;
  return eeprom_write_bytes(addr, (const byte *)string, numBytes);
}

boolean eeprom_read_string(int addr, char *buffer, int bufSize)
{
  byte ch;       // byte read from eeprom
  int bytesRead; // number of bytes read so far
  if (!eeprom_is_addr_ok(addr))
  { // check start address
    return false;
  }

  if (bufSize == 0)
  { // how can we store bytes in an empty buffer ?
    return false;
  }
  // is there is room for the string terminator only, no reason to go further
  if (bufSize == 1)
  {
    buffer[0] = 0;
    return true;
  }
  bytesRead = 0;                      // initialize byte counter
  ch = EEPROM.read(addr + bytesRead); // read next byte from eeprom
  buffer[bytesRead] = ch;             // store it into the user buffer
  bytesRead++;                        // increment byte counter

  while ((ch != 0x00) && (bytesRead < bufSize) && ((addr + bytesRead) <= EEPROM_MAX_ADDR))
  {
    // if no stop condition is met, read the next byte from eeprom
    ch = EEPROM.read(addr + bytesRead);
    buffer[bytesRead] = ch; // store it into the user buffer
    bytesRead++;            // increment byte counter
  }
  // make sure the user buffer has a string terminator, (0x00) as its last byte
  if ((ch != 0x00) && (bytesRead >= 1))
  {
    buffer[bytesRead - 1] = 0;
  }
  return true;
}

#pragma endregion

/*
  -----------------------------------------------------------------------------
  SETUP
  -----------------------------------------------------------------------------
*/
void setup()
{
  // Debugger
  Serial.begin(115200);
  delay(250);

  // Lora
  Serial2.begin(9600);
  delay(250);

  // RS485
  pinMode(Tx485Control, OUTPUT);
  pinMode(Tx485Control2, OUTPUT);
  digitalWrite(Tx485Control, RS485Receive);
  digitalWrite(Tx485Control2, RS485Receive);
  delay(1);
  Serial3.begin(115200);
  delay(250);

  // DHT sensor
  dht.begin();

  // Analog pin mode
  ADC1->regs->CR2 |= ADC_CR2_TSVREFE; // enable VREFINT and temp sensor
  pinMode(PA0, INPUT_ANALOG);
  pinMode(PA1, INPUT_ANALOG);
  analogRead(PA0);
  analogRead(PA1);

  // EEPROM first read
  get485Address(true);
  getTagId(true);

  // LED on board
  pinMode(PC13, OUTPUT);
  digitalWrite(PC13, 1); // LED OFF
  t0Blink = millis();
  blinkTime = 500;
  blinkFlag = true;

  // Echo debuger
  Serial.print(DEVICE_NAME);
  Serial.println(" START.");

} // SETUP END.
/*
  -----------------------------------------------------------------------------
  LOOP
  -----------------------------------------------------------------------------
*/
void loop()
{
  serialEvent();
  serialEvent2();
  serialEvent3();
  ReadDht(DHT_FLAG);
  ReadA0A1(A0A1_FLAG);
  blink(blinkFlag);
  //------------------------------------------------------------------------------------
  // S1:MCU debuger ":<cmd><data>" incomming
  //------------------------------------------------------------------------------------
  // // LORA
  // setLoraHomeAddress(stringComplete &&
  //               inputString.substring(0, 1).equals(CMD_SET_EEPROM) &&
  //               inputString.substring(1, 2).equals("0"));
  // getLoraHomeAddress(stringComplete &&
  //               inputString.substring(0, 1).equals(CMD_GET_EEPROM) &&
  //               inputString.substring(1, 2).equals("0"));
  // RS485
  set485Address(stringComplete &&
                inputString.substring(0, 1).equals(CMD_SET_EEPROM) &&
                inputString.substring(1, 2).equals("2"));
  get485Address(stringComplete &&
                inputString.substring(0, 1).equals(CMD_GET_EEPROM) &&
                inputString.substring(1, 2).equals("2"));
  setTagId(stringComplete &&
           inputString.substring(0, 1).equals(CMD_SET_EEPROM) &&
           inputString.substring(1, 2).equals("3"));
  getTagId(stringComplete &&
           inputString.substring(0, 1).equals(CMD_GET_EEPROM) &&
           inputString.substring(1, 2).equals("3"));
  // LED on board
  blinkON(stringComplete && inputString.substring(0, 1).equals(CMD_BLINK_ON));
  blinkOFF(stringComplete && inputString.equals(CMD_BLINK_OFF));
  // MCU information
  info(stringComplete && inputString.equals(CMD_INFO));
  PrintQ0(stringComplete &&
          inputString.substring(0, 1).equals(CMD_QUERY) &&
          inputString.substring(1).equals("0"));
  PrintQ1(stringComplete &&
          inputString.substring(0, 1).equals(CMD_QUERY) &&
          inputString.substring(1).equals("1"));
  PrintQ2(stringComplete &&
          inputString.substring(0, 1).equals(CMD_QUERY) &&
          inputString.substring(1, 2).equals("2"));
  //info1to3(stringComplete && inputString.equals(CMD_INFO));
  // DHT: AM2302
  PrintDhtSensor(stringComplete && inputString.equals(CMD_PRT_DHT));
  OffDhtSensor(stringComplete && inputString.equals(CMD_OFF_DHT));
  ToggleTraceBlink(stringComplete &&
                   inputString.substring(0, 1).equals(CMD_TRACE) &&
                   inputString.substring(1, 2).equals("0"));
  ToggleTraceAdc(stringComplete &&
                 inputString.substring(0, 1).equals(CMD_TRACE) &&
                 inputString.substring(1, 2).equals("1"));
  ToggleRs485Mon(stringComplete &&
                 inputString.substring(0, 1).equals(CMD_TRACE) &&
                 inputString.substring(1, 2).equals("4"));

  //------------------------------------------------------------------------------------
  // S2:LORA "$<cmd><data>" incomming
  //------------------------------------------------------------------------------------
  PrintTo485_S3(STX_COME2 && stringComplete2 &&
                inputString2.substring(0, 1).equals(CMD_FORWORD));
  info_S2(STX_COME2 && stringComplete2 &&
          inputString2.equals(CMD_INFO));
  PrintQ0_S2(STX_COME2 && stringComplete2 &&
                 inputString2.substring(0, 1).equals(CMD_QUERY) &&
                 inputString2.substring(1, 2).equals("0"),
             inputString2.substring(2, 3).equals("A"));
  PrintQ1_S2(STX_COME2 && stringComplete2 &&
             inputString2.substring(0, 1).equals(CMD_QUERY) &&
             inputString2.substring(1).equals("1"));
  PrintQ2_S2(STX_COME2 && stringComplete2 &&
             inputString2.substring(0, 1).equals(CMD_QUERY) &&
             inputString2.substring(1, 2).equals("2"));

  //------------------------------------------------------------------------------------
  // S3:RS485 "$<sen><rec><cmd><data>" incomming
  //------------------------------------------------------------------------------------
  PrintToLoraHome_S2(STX_COME3 && stringComplete3 &&
                     inputString3.substring(S3_RECEIVER_IDX, S3_CMD_KEY_IDX).equals(_ADDR_) &&
                     inputString3.substring(S3_CMD_KEY_IDX, S3_DATA_IDX).equals(CMD_FORWORD));
  blinkON3(STX_COME3 && stringComplete3 &&
           inputString3.substring(S3_RECEIVER_IDX, S3_CMD_KEY_IDX).equals(_ADDR_) &&
           inputString3.substring(S3_CMD_KEY_IDX, S3_DATA_IDX).equals(CMD_BLINK_ON));
  blinkOFF3(STX_COME3 && stringComplete3 &&
            inputString3.substring(S3_RECEIVER_IDX, S3_CMD_KEY_IDX).equals(_ADDR_) &&
            inputString3.substring(S3_CMD_KEY_IDX).equals(CMD_BLINK_OFF));
  info3(STX_COME3 && stringComplete3 &&
        inputString3.substring(S3_RECEIVER_IDX, S3_CMD_KEY_IDX).equals(_ADDR_) &&
        inputString3.substring(S3_CMD_KEY_IDX, S3_DATA_IDX).equals(CMD_INFO));
  PrintQ0_S3(STX_COME3 && stringComplete3 &&
                 inputString3.substring(S3_RECEIVER_IDX, S3_CMD_KEY_IDX).equals(_ADDR_) &&
                 inputString3.substring(S3_CMD_KEY_IDX, S3_DATA_IDX).equals(CMD_QUERY) &&
                 inputString3.substring(S3_DATA_IDX, S3_DATA_IDX + 1).equals("0"),
             inputString3.substring(S3_DATA_IDX + 1, S3_DATA_IDX + 2).equals("A"));
  PrintQ1_S3(STX_COME3 && stringComplete3 &&
             inputString3.substring(S3_RECEIVER_IDX, S3_CMD_KEY_IDX).equals(_ADDR_) &&
             inputString3.substring(S3_CMD_KEY_IDX, S3_DATA_IDX).equals(CMD_QUERY) &&
             inputString3.substring(S3_DATA_IDX).equals("1"));
  PrintQ2_S3(STX_COME3 && stringComplete3 &&
             inputString3.substring(S3_RECEIVER_IDX, S3_CMD_KEY_IDX).equals(_ADDR_) &&
             inputString3.substring(S3_CMD_KEY_IDX, S3_DATA_IDX).equals(CMD_QUERY) &&
             inputString3.substring(S3_DATA_IDX, S3_DATA_IDX + 1).equals("2"));

  //
  ClearSerialEvent(stringComplete || stringComplete2 || stringComplete3);
} // LOOP END.
//

#pragma region LORA / 485

#pragma endregion

#pragma region Analog Signals
/*
  -----------------------------------------------------------------------------
  Analog: A0, A1
  -----------------------------------------------------------------------------
*/
void ReadA0A1(bool flag)
{
  if (flag)
  {
    if (ADC_TIME < (millis() - t0A0A1))
    {
      VREF = 1.2 * 4095.0 / (float)adc_read(ADC1, 17);

      Adc0Value = analogRead(PA0);
      float V0 = VREF * (float)Adc0Value / 4095.0;
      V0Total = V0Total - V0Readings[A0A1ReadIndex];
      V0Readings[A0A1ReadIndex] = V0;
      V0Total = V0Total + V0Readings[A0A1ReadIndex];
      Voltage0 = V0Total / (float)MAX_ADC_READINGS;

      Adc1Value = analogRead(PA1);
      float V1 = VREF * (float)Adc1Value / 4095.0;
      V1Total = V1Total - V1Readings[A0A1ReadIndex];
      V1Readings[A0A1ReadIndex] = V1;
      V1Total = V1Total + V1Readings[A0A1ReadIndex];
      Voltage1 = V1Total / (float)MAX_ADC_READINGS;

      A0A1ReadIndex = A0A1ReadIndex + 1;
      if (A0A1ReadIndex >= MAX_ADC_READINGS)
      {
        A0A1ReadIndex = 0;
        if (TRACE_ADC)
        {
          Serial.print("A0 = ");
          Serial.print(Adc0Value);
          Serial.print(", A1 = ");
          Serial.print(Adc1Value);
          Serial.print(", Vref = ");
          Serial.print(VREF, 3);
          Serial.print(", V0 (Volt) = ");
          Serial.print(Voltage0, 3);
          Serial.print(", V1 (Volt) = ");
          Serial.print(Voltage1, 3);
          Serial.println();
        }
      }
      t0A0A1 = millis();
    }
  }
}

#pragma endregion

#pragma region DHT SENSOR
/*
  -----------------------------------------------------------------------------
  DHT: AM2302
  -----------------------------------------------------------------------------
*/
void ReadDht(bool flag)
{
  if (flag)
  {
    if (DHT_TIME < (millis() - t0Dht))
    {
      RHumidity = dht.readHumidity();
      Temperature = dht.readTemperature();
      // Read temperature as Fahrenheit (isFahrenheit = true)
      // Temperature = dht.readTemperature(true);
      HeatIndex = dht.computeHeatIndex(Temperature, RHumidity, false);

      t0Dht = millis();
    }
  }
}

void PrintDhtSensor(bool flag)
{
  if (flag)
  {
    if (!DHT_FLAG)
      ReadDht(true);
    DHT_FLAG = true;
    Serial.print(F("Humidity: "));
    Serial.print(RHumidity);
    Serial.print(F("%  Temperature: "));
    Serial.print(Temperature);
    Serial.print(F("*C Heat index: "));
    Serial.print(HeatIndex);
    Serial.println(F("*C "));
  }
}

void OffDhtSensor(bool flag)
{
  if (flag)
  {
    DHT_FLAG = false;
    Serial.println("DHT sensor turned off.");
  }
}

#pragma endregion

#pragma region EEPROM
/*
  -----------------------------------------------------------------------------
  EEPROM
  -----------------------------------------------------------------------------
*/
String EEPROMread(int StartAddr, int StringLength)
{
  char buf[StringLength + 1];

  if (!eeprom_read_string(StartAddr, buf, StringLength + 1))
  {
    return "ERROR";
  }

  String dataStr = buf;
  return dataStr;
}
bool EEPROMwrite(int StartAddr, String DataString)
{
  int val = DataString.length() + 1;
  char StringChar[val];
  char buff[val];
  //convert string to char array
  DataString.toCharArray(StringChar, val);
  strcpy(buff, StringChar);
  return eeprom_write_string(StartAddr, buff);
}
#pragma endregion

#pragma region EEPROM CONFIGURATION
/*
  -----------------------------------------------------------------------------
  EEPROM: RS485 ADDRESS
  -----------------------------------------------------------------------------
*/
void set485Address(bool flag)
{
  if (flag)
  {
    String addr = inputString.substring(2, 2 + LEN_ADDR);
    Serial.print("Address = ");
    Serial.println(addr);

    if (!EEPROMwrite(EEPROM_RS485, addr))
    {
      Serial.println("EEPROM ERROR 01!");
      return;
    }

    Serial.println("EEPORM Write OK.");

    delay(100);
    get485Address(true);
  }
}
void get485Address(bool flag)
{
  if (flag)
  {
    Serial.print("RS485 Address: ");
    _ADDR_ = EEPROMread(EEPROM_RS485, LEN_ADDR);
    Serial.println(_ADDR_);
  }
}
void setTagId(bool flag)
{
  if (flag)
  {
    String addr = inputString.substring(2, 2 + LEN_ID);
    Serial.print("ID = ");
    Serial.println(addr);

    if (!EEPROMwrite(EEPROM_TAG_ID, addr))
    {
      Serial.println("EEPROM ERROR 01!");
      return;
    }

    Serial.println("EEPORM Write OK.");

    delay(100);
    getTagId(true);
  }
}
void getTagId(bool flag)
{
  if (flag)
  {
    Serial.print("ID: ");
    _ID_ = EEPROMread(EEPROM_TAG_ID, LEN_ID);
    Serial.println(_ID_);
  }
}
#pragma endregion

#pragma region BLINK
/*
  -----------------------------------------------------------------------------
  BLINK CONTROL
  -----------------------------------------------------------------------------
*/
void blinkON(bool flag)
{
  if (flag)
  {
    blinkTime = inputString.substring(1).toInt();
    Serial.println(blinkTime);
    if (blinkTime <= 0)
    {
      blinkTime = 500;
    }
    if (10e3 < blinkTime)
    {
      blinkTime = 10e3;
    }
    blinkFlag = true;
    Serial.print("BLINK ON with time of ");
    Serial.print(blinkTime);
    Serial.println(" ms.");
  }
}
void blinkON3(bool flag)
{
  if (flag)
  {
    blinkTime = inputString3.substring(S3_DATA_IDX).toInt();
    Serial.println(blinkTime);
    if (blinkTime <= 0)
    {
      blinkTime = 500;
    }
    if (10e3 < blinkTime)
    {
      blinkTime = 10e3;
    }
    blinkFlag = true;
    Serial.print("BLINK ON with time of ");
    Serial.print(blinkTime);
    Serial.println(" ms.");
  }
}
void blinkOFF(bool flag)
{
  if (flag)
  {
    digitalWrite(PC13, 1);
    blinkFlag = false;
    Serial.println("Blink OFF.");
  }
}
void blinkOFF3(bool flag)
{
  if (flag)
  {
    digitalWrite(PC13, 1);
    blinkFlag = false;
    Serial.println("Blink OFF.");
  }
}
//
void blink(bool flag)
{
  if (flag)
  {
    if (blinkTime == 0)
      blinkTime = 500;
    if (blinkTime < (millis() - t0Blink))
    {
      blinkState = !blinkState;
      if (blinkState)
      {
        digitalWrite(PC13, 1);
        if (TRACE_BLINK)
        {
          Serial.print(millis());
          Serial.println(" LED OFF");
        }
      }
      else
      {
        digitalWrite(PC13, 0);
        if (TRACE_BLINK)
        {
          Serial.print(millis());
          Serial.println(" LED ON");
        }
      }
      t0Blink = millis();
    }
  }
}
#pragma endregion
//
#pragma region SERIAL PORT / UART
/*
  -----------------------------------------------------------------------------
  S1: MCU Trace
  -----------------------------------------------------------------------------
*/
void ToggleTraceAdc(bool flag)
{
  if (flag)
  {
    TRACE_ADC = !TRACE_ADC;
    Serial.println("Toggle ADC trace.");
  }
}
void ToggleTraceBlink(bool flag)
{
  if (flag)
  {
    TRACE_BLINK = !TRACE_BLINK;
    Serial.println("Toggle blink trace.");
  }
}
void ToggleRs485Mon(bool flag)
{
  if (flag)
  {
    TRACE_RS485 = !TRACE_RS485;
    Serial.println("Toggle RS485/S3 monitor.");
  }
}

/*
  -----------------------------------------------------------------------------
  S1: INFO
  -----------------------------------------------------------------------------
*/
void info(bool flag)
{
  if (flag)
  {
    String Message = String(millis()) + " ";
    Message += DEVICE_NAME;
    Message += "\r\n";
    byte chkSum = CheckSumOf(Message);

    Serial.print(Message);
    PrintCheckSum_S1(chkSum);

    get485Address(true);
    getTagId(true);
  }
} // INFO END.

/*
  -----------------------------------------------------------------------------
  S1: Query tag all and send back to Lora
  -----------------------------------------------------------------------------
*/
void PrintQ0(bool flag)
{
  if (flag)
  {
    // Sensors
    if (!DHT_FLAG)
      ReadDht(true);
    DHT_FLAG = true;
    if (!A0A1_FLAG)
      ReadA0A1(true);
    A0A1_FLAG = true;
    //
    String Message = _ID_;
    Message += ",Humidity (%RH):" + String(RHumidity);
    Message += ",Temperature (C):" + String(Temperature);
    Message += ",Heat Index (C):" + String(HeatIndex);
    Message += ",A0 (Count):" + String(Adc0Value);
    Message += ",A1 (Count):" + String(Adc1Value);
    Message += ",Vref (Volt):" + String(VREF, 3);
    Message += ",V0 (Volt):" + String(Voltage0, 3);
    Message += ",V1 (Volt):" + String(Voltage1, 3);
    Message += "\r\n";
    byte chkSum = CheckSumOf(Message);
    //
    Serial.print(Message);
    PrintCheckSum_S1(chkSum);
  }
} // INFO END.
void PrintQ1(bool flag)
{
  if (flag)
  {
    // Sensors
    if (!DHT_FLAG)
      ReadDht(true);
    DHT_FLAG = true;
    if (!A0A1_FLAG)
      ReadA0A1(true);
    A0A1_FLAG = true;
    // Print
    String Message = String(millis()) + ",";
    Message += String(RHumidity) + ",";
    Message += String(Temperature) + ",";
    Message += String(HeatIndex) + ",";
    Message += String(Adc0Value) + ",";
    Message += String(Adc1Value) + ",";
    Message += String(VREF, 3) + ",";
    Message += String(Voltage0, 3) + ",";
    Message += String(Voltage1, 3);
    Message += "\r\n";
    byte chkSum = CheckSumOf(Message);
    //
    Serial.print(Message);
    PrintCheckSum_S1(chkSum);
  }
} //
void PrintQ2(bool flag)
{
  if (flag)
  {
    int param = inputString.substring(2).toInt();
    // Sensors
    if (!DHT_FLAG)
      ReadDht(true);
    DHT_FLAG = true;
    if (!A0A1_FLAG)
      ReadA0A1(true);
    A0A1_FLAG = true;
    // Print
    String Message = String(millis()) + ",";
    switch (param)
    {
    case 1:
      Message += String(RHumidity, 2);
      break;
    case 2:
      Message += String(Temperature, 2);
      break;
    case 3:
      Message += String(HeatIndex, 2);
      break;
    case 4:
      Message += String(Adc0Value);
      break;
    case 5:
      Message += String(Adc1Value);
      break;
    case 6:
      Message += String(VREF, 3);
      break;
    case 7:
      Message += String(Voltage0, 3);
      break;
    case 8:
      Message += String(Voltage1, 3);
      break;
    default:
      Message += String(_ID_);
      break;
    }
    Message += "\r\n";
    byte chkSum = CheckSumOf(Message);
    //
    Serial.print(Message);
    PrintCheckSum_S1(chkSum);
  }
} //
/*
  -----------------------------------------------------------------------------
  S2 / LoRa: This use for forwarding RS485 Via LoRa
  -----------------------------------------------------------------------------
*/
void PrintToLoraHome_S2(bool flag)
{
  if (flag)
  {
    Serial.print("S2 << ");
    Serial.print(millis());
    Serial.println(" Forwarded to LORA.");
    //
    String Message = inputString3.substring(S3_DATA_IDX);
    Message += "," + String(RHumidity);
    Message += "," + String(Temperature);
    Message += "," + String(HeatIndex);
    Message += "\r\n";
    byte chkSum = CheckSumOf(Message);
    //
    Serial.print(Message);
    PrintCheckSum_S1(chkSum);
    //
    Serial2.print(LORA_ADDRH);
    Serial2.print(LORA_ADDRL);
    Serial2.print(LORA_CHAN);
    //
    Serial2.print(Message);
    PrintCheckSum_S2(chkSum);
  }
}
void info_S2(bool flag)
{
  if (flag)
  {
    Serial.print("S2 << ");
    Serial.print(millis());
    Serial.println(", Print Info...");
    //
    String Message = String(millis()) + ",";
    Message += DEVICE_NAME;
    Message += "," + _ADDR_;
    Message += "," + _ID_;
    Message += "\r\n";
    byte chkSum = CheckSumOf(Message);
    //
    Serial.print(Message);
    PrintCheckSum_S1(chkSum);
    //
    Serial2.print(LORA_ADDRH);
    Serial2.print(LORA_ADDRL);
    Serial2.print(LORA_CHAN);
    //
    Serial2.print(Message);
    PrintCheckSum_S2(chkSum);
  }
} // INFO END.
void PrintQ0_S2(bool flag, bool flag2)
{
  if (flag)
  {
    // Sensors
    if (!DHT_FLAG)
      ReadDht(true);
    DHT_FLAG = true;
    if (!A0A1_FLAG)
      ReadA0A1(true);
    A0A1_FLAG = true;
    // Print
    Serial.print("S3 << ");
    Serial.print(millis());
    Serial.println(", Print tag all with command to send to LoRa.");
    //
    String Message = _ID_;
    Message += ",Humidity (%RH):" + String(RHumidity);
    Message += ",Temperature (C):" + String(Temperature);
    Message += ",Heat Index (C):" + String(HeatIndex);
    Message += ",A0 (Count):" + String(Adc0Value);
    Message += ",A1 (Count):" + String(Adc1Value);
    Message += ",Vref (Volt):" + String(VREF, 3);
    Message += ",V0 (Volt):" + String(Voltage0, 3);
    Message += ",V1 (Volt):" + String(Voltage1, 3);
    Message += "\r\n";
    byte chkSum = CheckSumOf(Message);
    //
    if (flag2)
    {
      Serial.print(STX3);
      Serial.print(Message);
      PrintCheckSum_S1(chkSum);
    }
    //
    Serial2.print(LORA_ADDRH);
    Serial2.print(LORA_ADDRL);
    Serial2.print(LORA_CHAN);
    //
    Serial2.print(Message);
    PrintCheckSum_S2(chkSum);
  }
} //
void PrintQ1_S2(bool flag)
{
  if (flag)
  {
    // Sensors
    if (!DHT_FLAG)
      ReadDht(true);
    DHT_FLAG = true;
    if (!A0A1_FLAG)
      ReadA0A1(true);
    A0A1_FLAG = true;
    // Print
    Serial.print("S3 << ");
    Serial.print(millis());
    Serial.println(", Print tag all by CSV with command to send to LoRa.");
    //
    String Message = "";
    Message += String(RHumidity) + ",";
    Message += String(Temperature) + ",";
    Message += String(HeatIndex) + ",";
    Message += String(Adc0Value) + ",";
    Message += String(Adc1Value) + ",";
    Message += String(VREF, 3) + ",";
    Message += String(Voltage0, 3) + ",";
    Message += String(Voltage1, 3);
    Message += "\r\n";
    byte chkSum = CheckSumOf(Message);
    //
    Serial.print(Message);
    PrintCheckSum_S1(chkSum);
    //
    Serial2.print(LORA_ADDRH);
    Serial2.print(LORA_ADDRL);
    Serial2.print(LORA_CHAN);
    //
    Serial2.print(Message);
    PrintCheckSum_S2(chkSum);
  }
} //
void PrintQ2_S2(bool flag)
{
  if (flag)
  {
    int param = inputString2.substring(2).toInt();
    // Sensors
    if (!DHT_FLAG)
      ReadDht(true);
    DHT_FLAG = true;
    if (!A0A1_FLAG)
      ReadA0A1(true);
    A0A1_FLAG = true;
    // Print
    Serial.print("S3 << ");
    Serial.print(millis());
    Serial.print(", Print as selected parameter --> ");
    Serial.println(param);
    //
    String Message = "";
    switch (param)
    {
    case 1:
      Message += String(RHumidity, 2);
      break;
    case 2:
      Message += String(Temperature, 2);
      break;
    case 3:
      Message += String(HeatIndex, 2);
      break;
    case 4:
      Message += String(Adc0Value);
      break;
    case 5:
      Message += String(Adc1Value);
      break;
    case 6:
      Message += String(VREF, 3);
      break;
    case 7:
      Message += String(Voltage0, 3);
      break;
    case 8:
      Message += String(Voltage1, 3);
      break;
    default:
      Message += String(_ID_);
      break;
    }
    Message += "\r\n";
    byte chkSum = CheckSumOf(Message);
    //
    Serial.print(Message);
    PrintCheckSum_S1(chkSum);
    //
    Serial2.print(LORA_ADDRH);
    Serial2.print(LORA_ADDRL);
    Serial2.print(LORA_CHAN);
    //
    Serial2.print(Message);
    PrintCheckSum_S2(chkSum);
  }
} //
/*
  -----------------------------------------------------------------------------
  S3 / RS485: This use for forwarding Lora Via RS485
  -----------------------------------------------------------------------------
*/
void PrintTo485_S3(bool flag)
{
  if (flag)
  {
    Serial.print("S3 << ");
    Serial.print(millis());
    Serial.println(" Forwarded to RS485.");
    //
    String Message = _ADDR_ + inputString2.substring(1) + "\r\n";
    byte chkSum = CheckSumOf(Message);
    //
    Serial.print(Message);
    PrintCheckSum_S1(chkSum);
    //
    Set485ToTransmit();
    Serial3.print(STX3);
    Serial3.print(Message);
    PrintCheckSum_S3(chkSum);
    Set458ToReceive(1);
  }
}

/*
  -----------------------------------------------------------------------------
  S3 / RS485: Info
  -----------------------------------------------------------------------------
*/
void info3(bool flag)
{
  if (flag)
  {
    Serial.print("S3 << ");
    Serial.print(millis());
    Serial.println(", Print Info.");
    //
    String Message = _ADDR_ + SENDER_ADDR + " ";
    Message += String(millis()) + ",";
    Message += DEVICE_NAME;
    Message += ", ID: " + _ID_;
    Message += "\r\n";
    byte chkSum = CheckSumOf(Message);
    //
    Serial.print(Message);
    PrintCheckSum_S1(chkSum);
    //
    Set485ToTransmit();
    Serial3.print(STX3);
    Serial3.print(Message);
    PrintCheckSum_S3(chkSum);
    Set458ToReceive(1);
  }
} // INFO END.

/*
  -----------------------------------------------------------------------------
  S3 / RS485: Query tag all and send back to Lora
  -----------------------------------------------------------------------------
*/
void PrintQ0_S3(bool flag, bool flag2)
{
  if (flag)
  {
    // Sensors
    if (!DHT_FLAG)
      ReadDht(true);
    DHT_FLAG = true;
    if (!A0A1_FLAG)
      ReadA0A1(true);
    A0A1_FLAG = true;
    // Print
    Serial.print("S3 << ");
    Serial.print(millis());
    Serial.println(", Print tag all with command to send to LoRa.");
    //
    String Message = _ADDR_ + SENDER_ADDR + CMD_FORWORD;
    Message += _ID_;
    Message += ",Humidity (%RH):" + String(RHumidity);
    Message += ",Temperature (C):" + String(Temperature);
    Message += ",Heat Index (C):" + String(HeatIndex);
    Message += ",A0 (Count):" + String(Adc0Value);
    Message += ",A1 (Count):" + String(Adc1Value);
    Message += ",Vref (Volt):" + String(VREF, 3);
    Message += ",V0 (Volt):" + String(Voltage0, 3);
    Message += ",V1 (Volt):" + String(Voltage1, 3);
    Message += "\r\n";
    byte chkSum = CheckSumOf(Message);
    //
    if (flag2)
    {
      Serial.print(STX3);
      Serial.print(Message);
      PrintCheckSum_S1(chkSum);
    }
    //
    Set485ToTransmit();
    Serial3.print(STX3);
    Serial3.print(Message);
    PrintCheckSum_S3(chkSum);
    Set458ToReceive(10);
  }
} //
void PrintQ1_S3(bool flag)
{
  if (flag)
  {
    // Sensors
    if (!DHT_FLAG)
      ReadDht(true);
    DHT_FLAG = true;
    if (!A0A1_FLAG)
      ReadA0A1(true);
    A0A1_FLAG = true;
    // Print
    Serial.print("S3 << ");
    Serial.print(millis());
    Serial.println(", Print tag all by CSV with command to send to LoRa.");
    //
    String Message = _ADDR_ + SENDER_ADDR + CMD_FORWORD;
    Message += String(RHumidity) + ",";
    Message += String(Temperature) + ",";
    Message += String(HeatIndex) + ",";
    Message += String(Adc0Value) + ",";
    Message += String(Adc1Value) + ",";
    Message += String(VREF, 3) + ",";
    Message += String(Voltage0, 3) + ",";
    Message += String(Voltage1, 3);
    Message += "\r\n";
    byte chkSum = CheckSumOf(Message);
    //
    Serial.print(Message);
    PrintCheckSum_S1(chkSum);
    //
    Set485ToTransmit();
    Serial3.print(STX3);
    Serial3.print(Message);
    PrintCheckSum_S3(chkSum);
    Set458ToReceive(10);
  }
} //
void PrintQ2_S3(bool flag)
{
  if (flag)
  {
    int param = inputString3.substring(S3_DATA_IDX + 1).toInt();
    // Sensors
    if (!DHT_FLAG)
      ReadDht(true);
    DHT_FLAG = true;
    if (!A0A1_FLAG)
      ReadA0A1(true);
    A0A1_FLAG = true;
    // Print
    Serial.print("S3 << ");
    Serial.print(millis());
    Serial.print(", Print as selected parameter --> ");
    Serial.println(param);
    //
    String Message = _ADDR_ + SENDER_ADDR + CMD_FORWORD;
    switch (param)
    {
    case 1:
      Message += String(RHumidity, 2);
      break;
    case 2:
      Message += String(Temperature, 2);
      break;
    case 3:
      Message += String(HeatIndex, 2);
      break;
    case 4:
      Message += String(Adc0Value);
      break;
    case 5:
      Message += String(Adc1Value);
      break;
    case 6:
      Message += String(VREF, 3);
      break;
    case 7:
      Message += String(Voltage0, 3);
      break;
    case 8:
      Message += String(Voltage1, 3);
      break;
    default:
      Message += String(_ID_);
      break;
    }
    Message += "\r\n";
    byte chkSum = CheckSumOf(Message);
    //
    Serial.print(Message);
    PrintCheckSum_S1(chkSum);
    //
    Set485ToTransmit();
    Serial3.print(STX3);
    Serial3.print(Message);
    PrintCheckSum_S3(chkSum);
    Set458ToReceive(3);
  }
} //
/*
  ------------------------------------------------------------------------
  Serial event
  ------------------------------------------------------------------------
*/
void serialEvent()
{
  if (Serial.available())
  {

    // get the new byte:
    char inChar = (char)Serial.read();

    // add it to the inputString:
    if (STX_COME)
    {
      if (inChar == ETX)
      {
        stringComplete = true;
        Serial.println();
#if _S1_ETX_TRACE_
        Serial.print("Input string = ");
        Serial.println(inputString);
#endif
        Serial.println("<<<ETX come>>>");
        Serial.println();
        return;
      }
      if (inChar != STX && inChar != '\r' && inChar != ETX)
      {
        inputString += inChar;
      }
      return;
    }

    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:
    if (inChar == STX)
    {
      STX_COME = true;
      stringComplete = false;
      inputString = "";
      Serial.println("<<<STX come>>>");
      return;
    }
  }
}
//
void serialEvent2() // LORA
{
  if (Serial2.available())
  {

    // get the new byte:
    char inChar = (char)Serial2.read();
    Serial.print("S2 >> ");
    Serial.println(inChar);

    // add it to the inputString:
    if (STX_COME2)
    {
      if (inChar == ETX2)
      {
        stringComplete2 = true;
        Serial.println();
#if _S2_ETX_TRACE_
        Serial.print("Input string = ");
        Serial.println(inputString2);
#endif
        Serial.println("<<<ETX2 come>>>");
        Serial.println();
        return;
      }
      if (inChar != STX2 && inChar != '\n' && inChar != '\r' && inChar != ETX2)
      {
        inputString2 += inChar;
      }
      return;
    }

    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:
    if (inChar == STX2)
    {
      STX_COME2 = true;
      stringComplete2 = false;
      inputString2 = "";
      Serial.println("<<<STX2 come>>>");
      return;
    }
  }
}
//
void serialEvent3() // 485
{
  if (Serial3.available())
  {

    // get the new byte:
    char inChar = (char)Serial3.read();
    if (TRACE_RS485)
    {
      Serial.print("S3 >> ");
      Serial.println(inChar);
    }
    // add it to the inputString:
    if (STX_COME3)
    {
      if (inChar == ETX3)
      {
        stringComplete3 = true;
        Serial.println();
        get485Address(true);
        SENDER_ADDR = inputString3.substring(S3_SENDER_IDX, S3_RECEIVER_IDX);
#if _S3_ETX_TRACE_
        Serial.print("Input string = ");
        Serial.println(inputString3);
        Serial.print("Sender Address = ");
        Serial.println(SENDER_ADDR);
        Serial.print("Receiver Address = ");
        Serial.println(inputString3.substring(S3_RECEIVER_IDX, S3_CMD_KEY_IDX));
        Serial.print("Message = ");
        Serial.println(inputString3.substring(S3_CMD_KEY_IDX));
        Serial.print("Command Byte = ");
        Serial.println(inputString3.substring(S3_CMD_KEY_IDX, S3_DATA_IDX));
        Serial.print("Data = ");
        Serial.println(inputString3.substring(S3_DATA_IDX));
#endif
        Serial.println("<<<ETX3 come>>>");
        Serial.println();
        return;
      }
      if (inChar != STX3 && inChar != '\n' && inChar != '\r' && inChar != ETX3)
      {
        inputString3 += inChar;
      }
      return;
    }

    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:
    if (inChar == STX3)
    {
      STX_COME3 = true;
      stringComplete3 = false;
      inputString3 = "";
      // Serial.println("STX3 come."); // !!! CAUTION THERE ARE LOSE BYTES
      return;
    }
  }
}
//
void ClearSerialEvent(bool flag)
{
  if (flag)
  {
    STX_COME = false;
    stringComplete = false;
    inputString = "";

    STX_COME2 = false;
    stringComplete2 = false;
    inputString2 = "";

    STX_COME3 = false;
    stringComplete3 = false;
    inputString3 = "";

    Serial.println("<<<Clear serial event>>>");
    Serial.println("-----------------------------------------------------------------");
    Serial.println();
  }
}
void Set485ToTransmit()
{
  digitalWrite(Tx485Control, RS485Transmit);
  digitalWrite(Tx485Control2, RS485Transmit);
}
void Set458ToReceive(int ms)
{
  delay(ms);
  digitalWrite(Tx485Control, RS485Receive);
  digitalWrite(Tx485Control2, RS485Receive);
}
byte CheckSumOf(String &s)
{
  int sumOfBytes = 0; // undefined by default
  const char *prt = s.c_str();
  // Serial.println(prt);
  // Serial.println();

  if (prt)
  {
    while (*prt)
    {
      // Serial.print("CHAR = ");
      // Serial.println(*prt);
      sumOfBytes += *prt++; // compute on a int
      // Serial.println(sumOfBytes);
    }
  }
  return (byte)(sumOfBytes % 256);
}
void PrintCheckSum_S1(byte chk)
{
  if (chk < 0x10)
    Serial.print('0');
  Serial.print(chk, HEX);
}
void PrintCheckSum_S2(byte chk)
{
  if (chk < 0x10)
    Serial2.print('0');
  Serial2.print(chk, HEX);
}
void PrintCheckSum_S3(byte chk)
{
  if (chk < 0x10)
    Serial3.print('0');
  Serial3.print(chk, HEX);
}
#pragma endregion