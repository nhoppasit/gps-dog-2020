/*
 * This is the Arduino code Ublox NEO-6M GPS module
 * this code extracts the GPS latitude and longitude so it can be used for other purposes

 * 
 * Written by Ahmad Nejrabi for Robojax Video
 * Date: Jan. 24, 2017, in Ajax, Ontario, Canada
 * Permission granted to share this code given that this
 * note is kept with the code.
 * Disclaimer: this code is "AS IS" and for educational purpose only.
 * 
 */

// written for RoboJax.com
String inputString = "";        // a string to hold incoming data
boolean stringComplete = false; // whether the string is complete
String signal = "$GPGLL";
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
    Serial2Event();

    // print the string when a newline arrives:
    if (stringComplete)
    {
        Serial.println("<<<GPS>>>");
        Serial.println(inputString);
        //
        String BB = inputString.substring(0, 6);
        Serial.println(BB);
        if (BB == signal)
        {
            String LAT = inputString.substring(7, 17);
            int LATperiod = LAT.indexOf('.');
            int LATzero = LAT.indexOf('0');
            if (LATzero == 0)
            {
                LAT = LAT.substring(1);
            }

            String LON = inputString.substring(20, 31);
            int LONperiod = LON.indexOf('.');
            int LONTzero = LON.indexOf('0');
            if (LONTzero == 0)
            {
                LON = LON.substring(1);
            }

            Serial.println(LAT);
            Serial.println(LON);
        }

        // clear the string:
        inputString = "";
        stringComplete = false;
    }
}

/*
Serial2Event occurs whenever a new data comes in the
hardware Serial2 RX. This routine is run between each
time loop() runs, so using delay inside loop can delay
response. Multiple bytes of data may be available.
*/
void Serial2Event()
{
    while (Serial2.available())
    {
        // get the new byte:
        char inChar = (char)Serial2.read();
        Serial.print(inChar);
        // if(inChar=='\r') Serial.print("<CR>");
        // if(inChar=='\n') Serial.print("<LF>");

        // add it to the inputString:
        inputString += inChar;
        // if the incoming character is a newline, set a flag
        // so the main loop can do something about it:
        if (inChar == '\n')
        {
            stringComplete = true;
        }
    }
}
