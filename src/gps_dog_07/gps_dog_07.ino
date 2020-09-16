

// Modify from RoboJax.com
// By Nhoppasit S.

#define DEVICE_NAME "GPS DOG TAG PROJECT 9:41 AM 9/16/2020"

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
//
//
void setup()
{
    // initialize Serial:
    Serial.begin(115200);
    // initialize Serial2:
    Serial2.begin(9600);
    // reserve 200 bytes for the inputString:
    inputString.reserve(200);
}

void loop()
{
    serialEvent();
    serialEvent2();

    blink(blinkFlag);

    //------------------------------------------------------------------------------------
    // S1:MCU debuger ":<cmd><data>" incomming
    //------------------------------------------------------------------------------------
    // LED on board
    blinkON(stringComplete && inputString.substring(0, 1).equals(CMD_BLINK_ON));
    blinkOFF(stringComplete && inputString.equals(CMD_BLINK_OFF));
    // MCU information
    info(stringComplete && inputString.equals(CMD_INFO));

    ToggleTraceBlink(stringComplete &&
                     inputString.substring(0, 1).equals(CMD_TRACE) &&
                     inputString.substring(1, 2).equals("0"));

    // print the string when a newline arrives:
    if (stringComplete2)
    {
        Serial.println("<<<GPS>>>");
        Serial.println(inputString2);
        //
        String BB = inputString2.substring(0, 6);
        Serial.println(BB);
        if (1)
        {
            String LAT = inputString2.substring(7, 17);
            int LATperiod = LAT.indexOf('.');
            int LATzero = LAT.indexOf('0');
            if (LATzero == 0)
            {
                LAT = LAT.substring(1);
            }

            String LON = inputString2.substring(20, 31);
            int LONperiod = LON.indexOf('.');
            int LONTzero = LON.indexOf('0');
            if (LONTzero == 0)
            {
                LON = LON.substring(1);
            }

            Serial.println(LAT);
            Serial.println(LON);
        }
    }

    ClearSerialEvent(stringComplete || stringComplete2);
}
void ToggleTraceBlink(bool flag)
{
    if (flag)
    {
        TRACE_BLINK = !TRACE_BLINK;
        Serial.println("Toggle blink trace.");
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
        Serial.print(inChar);
        // if(inChar=='\r') Serial.print("<CR>");
        // if(inChar=='\n') Serial.print("<LF>");

        // add it to the inputString:
        inputString2 += inChar;
        // if the incoming character is a newline, set a flag
        // so the main loop can do something about it:
        if (inChar == '\n')
        {
            stringComplete2 = true;
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

        STX_COME2 = false;
        stringComplete2 = false;
        inputString2 = "";

        Serial.println("<<<Clear serial event>>>");
        Serial.println("-----------------------------------------------------------------");
        Serial.println();
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