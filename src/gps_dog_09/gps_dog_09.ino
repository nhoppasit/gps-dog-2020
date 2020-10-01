// Modify from RoboJax.com
// By Nhoppasit S.

#define DEVICE_NAME "GPS DOG TAG PROJECT 9:41 AM 9/16/2020"
//
#define _S1_ETX_TRACE_ 0
//
const byte buff_len = 90;
char CRCbuffer[buff_len];
//
#define CMD_INFO "?"
#define CMD_TRACE "D"
#define CMD_BLINK_OFF "b"
#define CMD_BLINK_ON "B"
//
#define STX ':'
#define ETX '\n'
#define STX2 '$'
#define ETX2 '\n'
#define STX3 '$'
#define ETX3 '\n'
//
#include <TinyGPS++.h>
#include <SPI.h>
#include <SD.h>
//
const int chipSelect = PA4;
bool CardPresented = false;
//
String inputString = "";     // a String to hold incoming data
bool stringComplete = false; // whether the string is complete
bool STX_COME = false;
//
String inputString2 = "";     // a String to hold incoming data
bool stringComplete2 = false; // whether the string is complete
bool STX_COME2 = false;
//
unsigned long t0Blink = 0;
bool blinkState = 0;
bool blinkFlag = false;
int blinkTime = 500;
////
bool TRACE_BLINK = false;
bool TRACE_GPS = false;
bool TRACE_CRC = false;
bool TRACE_UART = false;
//
String signal = "$GPGLL";
//
TinyGPSPlus gps;
//
bool IsWrote = false;
//
void setup()
{
  pinMode(PC13, OUTPUT);
  pinMode(PB12, OUTPUT);
  digitalWrite(PB12, LOW);

  digitalWrite(PB12, HIGH);
  delay(1000);
  digitalWrite(PB12, LOW);
  delay(1000);
  digitalWrite(PB12, HIGH);
  delay(1000);
  digitalWrite(PB12, LOW);
  delay(1000);
  digitalWrite(PB12, HIGH);
  delay(1000);
  digitalWrite(PB12, LOW);

  // initialize Serial:
  Serial.begin(115200);
  // initialize Serial2:
  Serial2.begin(9600);
  // reserve 200 bytes for the inputString:
  inputString.reserve(200);

  Serial.print("Initializing SD card...");

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect))
  {
    Serial.println("Card failed, or not present");
    CardPresented = false;
  }
  else
  {
    Serial.println("card initialized.");
    CardPresented = true;
  }
}

void loop()
{
  serialEvent();
  if (TRACE_GPS)
  {
    serialEvent2();
  }
  else
  {
    while (Serial2.available() > 0)
    {
      gps.encode(Serial2.read());
      if (gps.location.isUpdated())
      {
        Serial.print("Longitude= ");
        Serial.print(gps.location.lng(), 6);
        Serial.print(" Latitude= ");
        Serial.print(gps.location.lat(), 6);

        // open the file. note that only one file can be open at a time,
        // so you have to close this one before opening another.
        File dataFile = SD.open("location.csv", FILE_WRITE);

        // if the file is available, write to it:
        if (dataFile)
        {
          dataFile.print(gps.location.lng(), 6);
          dataFile.print(",");
          dataFile.print(gps.location.lat(), 6);
          dataFile.println(",0");
          dataFile.close();
          // print to the serial port too:
          Serial.println(" -W");
        }
        // if the file isn't open, pop up an error:
        else
        {
          Serial.println(" - error opening location.csv");
        }
      }
    }
  }

  blink(blinkFlag);

  //------------------------------------------------------------------------------------
  // S1:MCU debuger ":<cmd><data>" incomming
  //------------------------------------------------------------------------------------
  // LED on board
  blinkON(stringComplete && inputString.substring(0, 1).equals(CMD_BLINK_ON));
  blinkOFF(stringComplete && inputString.equals(CMD_BLINK_OFF));
  // MCU information
  info(stringComplete && inputString.equals(CMD_INFO));

  ToggleTraceUart(stringComplete &&
                  inputString.substring(0, 1).equals(CMD_TRACE) &&
                  inputString.substring(1, 2).equals("0"));
  ToggleTraceBlink(stringComplete &&
                   inputString.substring(0, 1).equals(CMD_TRACE) &&
                   inputString.substring(1, 2).equals("1"));
  ToggleTraceGps(stringComplete &&
                 inputString.substring(0, 1).equals(CMD_TRACE) &&
                 inputString.substring(1, 2).equals("2"));
  ToggleTraceCrc(stringComplete &&
                 inputString.substring(0, 1).equals(CMD_TRACE) &&
                 inputString.substring(1, 2).equals("3"));

  //PrintGps(TRACE_GPS && stringComplete2);

  ClearSerialEvent(stringComplete);
  ClearSerialEvent2(stringComplete2);
}
//
void PrintGps(bool flag)
{
  if (flag)
  {
    String BB = inputString2.substring(0, 6);
    // Serial.println("<<<GPS>>>");
    // Serial.println(inputString2);
    Serial.print(BB);
    //
    if (1) //(BB == signal)
    {
      String LAT = inputString2.substring(7, 17);
      int LATperiod = LAT.indexOf(',');
      int LATzero = LAT.indexOf('0');
      if (LATzero == 0)
      {
        LAT = LAT.substring(1);
      }

      String LON = inputString2.substring(20, 31);
      int LONperiod = LON.indexOf(',');
      int LONTzero = LON.indexOf('0');
      if (LONTzero == 0)
      {
        LON = LON.substring(1);
      }

      if (-1 == LATperiod && -1 == LONperiod)
      {
        Serial.print(", ");
        Serial.print(LAT);
        Serial.print(", ");
        Serial.print(LON);

        // open the file. note that only one file can be open at a time,
        // so you have to close this one before opening another.
        File dataFile = SD.open("location.csv", FILE_WRITE);

        // if the file is available, write to it:
        if (dataFile)
        {
          dataFile.print(LAT);
          dataFile.print(", ");
          dataFile.print(LON);
          dataFile.println(", 10");
          dataFile.close();
          // print to the serial port too:
          Serial.print(" -W");
          //
          if (IsWrote)
          {
            IsWrote = false;
            digitalWrite(PB12, LOW);//Toggle
          }
          else
          {
            IsWrote = true;
            digitalWrite(PB12, HIGH);//Toggle
          }
        }
        // if the file isn't open, pop up an error:
        else
        {
          Serial.print(" - error opening location.csv");
        }
      }
    }
    Serial.println();
  }
}
//
void ToggleTraceUart(bool flag)
{
  if (flag)
  {
    TRACE_UART = !TRACE_UART;
    Serial.println("Toggle UART trace.");
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
void ToggleTraceGps(bool flag)
{
  if (flag)
  {
    TRACE_GPS = !TRACE_GPS;
    Serial.println("Toggle GPS trace.");
  }
}
void ToggleTraceCrc(bool flag)
{
  if (flag)
  {
    TRACE_CRC = !TRACE_CRC;
    Serial.println("Toggle CRC trace.");
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
  }
} // INFO END.
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
void blinkOFF(bool flag)
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
/*
  Serial2Event occurs whenever a new data comes in the
  hardware Serial2 RX. This routine is run between each
  time loop() runs, so using delay inside loop can delay
  response. Multiple bytes of data may be available.
*/
void serialEvent2()
{
  if (Serial2.available())
  {
    // get the new byte:
    char inChar = (char)Serial2.read();

    // if(inChar=='\r') Serial.print("<CR>");
    // if(inChar=='\n') Serial.print("<LF>");

    // add it to the inputString:
    inputString2 += inChar;
    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
    if (inChar == '\n')
    {
      stringComplete2 = true;
      if (TRACE_GPS)
      {
        outputMsg(inputString2);
      }
    }
  }
}
void ClearSerialEvent(bool flag)
{
  if (flag)
  {
    STX_COME = false;
    stringComplete = false;
    inputString = "";

    if (TRACE_UART)
    {
      Serial.println("<<<Clear serial event>>>");
      Serial.println("-----------------------------------------------------------------");
      Serial.println();
    }
  }
}
void ClearSerialEvent2(bool flag)
{
  if (flag)
  {
    STX_COME2 = false;
    stringComplete2 = false;
    inputString2 = "";

    if (TRACE_UART)
    {
      Serial.println("<<<Clear serial event>>>");
      Serial.println("-----------------------------------------------------------------");
      Serial.println();
    }
  }
}
//
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
// -----------------------------------------------------------------------
void outputMsg(String msg)
{
  char c;
  byte i;
  byte start_with = 0;
  byte end_with = 0;

  Serial.print(msg);                             // omit CRC in console msg
  msg.toCharArray(CRCbuffer, sizeof(CRCbuffer)); // put complete string into CRCbuffer

  for (i = 0; i < buff_len; i++)
  {
    c = CRCbuffer[i];
    if (c == '$')
    {
      start_with = i;
    }
    if (c == '*')
    {
      end_with = i;
      break;
    }
  }
  //
  byte crc = convertToCRC(CRCbuffer, start_with, end_with);
  if (TRACE_CRC)
  {
    Serial.println();
    Serial.print("CRC = ");
    if (crc < 16)
      Serial.print("0"); // add leading 0 if needed
    Serial.println(crc, HEX);
    //
    Serial.print("CRC text = ");
    Serial.println(msg.substring(end_with + 1, end_with + 3));
  }
}

// -----------------------------------------------------------------------
byte convertToCRC(char *buff, byte start_with, byte end_with)
{
  byte i;
  byte crc;

  // NMEA CRC: XOR each byte with previous for all chars between '$' and '*'
  if (TRACE_CRC)
  {
    Serial.println();
    Serial.print("Start with = ");
    Serial.println(start_with);
    Serial.print("End with = ");
    Serial.println(end_with);
  }
  if (end_with > start_with)
  {
    for (i = start_with + 1; i < end_with; i++)
    { // XOR every character between '$' and '*'
      if (TRACE_CRC)
        Serial.print(buff[i]);
      crc = crc ^ buff[i]; // compute CRC
    }
  }
  else
  { // else if error, print a msg (to both ports)
    if (TRACE_CRC)
      Serial.println("CRC ERROR");
  }
  return crc;
  //based on code by Elimeléc López - July-19th-2013
}
