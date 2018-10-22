
#include <Wire.h>
#include <SPI.h>
#include <Ethernet.h>
#include "DS3231.h"
#include <LiquidCrystal_I2C.h>

const int buzzerPin = 9;
const int ledPin1 = 2;
const int ledPin2 = 3;

int alarm = 0;
int alarmSet = 0;
int i_Hourb;
int i_Minuteb;
int stoperica = 0;
int sekundeStoperica = 0;
int i_Houro = 0;
int i_Minuteo = 0;
int i_Secondo = 0;
int odbrojavanjeSet = 0;
int stopericaNot = 0;


DS3231 rtc(SDA, SCL);
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
IPAddress ip(192, 168, 10, 177);
EthernetServer server(80);

String readString;

void setup()
{
    Serial.begin(9600);


    while (!Serial) { }
    // start the Ethernet connection and the server:
    Ethernet.begin(mac, ip);
    server.begin();
    pinMode(buzzerPin, OUTPUT);
    pinMode(ledPin1, OUTPUT);
    pinMode(ledPin2, OUTPUT);
    rtc.begin();
    rtc.setTime(16, 0, 0);
    lcd.begin(16, 2);


}


void loop()
{

    lcd.clear();
    lcd.print(rtc.getTimeStr());
    lcd.setCursor(0, 1);
    EthernetClient client = server.available();
    if (client)
    {
        boolean currentLineIsBlank = true;
        while (client.connected())
        {
            if (client.available())
            {
                char c = client.read();

                if (readString.length() < 100)
                {
                    //store characters to string 
                    readString += c;
                }

                if (c == '\n' && currentLineIsBlank)
                {
                    // send a standard http response header
                    client.println("HTTP/1.1 200 OK");
                    client.println("Content-Type: text/html");
                    client.println("Connection: close");  // the connection will be closed after completion of the response
                    client.println();
                    client.println("<!DOCTYPE HTML>");
                    client.println("<html>");

                    // Form for budilica
                    client.println("<form action='/' method=get>");
                    client.println("Budilica: ");
                    client.println("<input type=time name='budilica' value=''>");
                    client.println("<input type=submit name='submit' value='submit'>");
                    client.println("</form><br>");

                    //Form for start/stop stoperica
                    client.println("Stoperica (start or stop)");
                    client.println("<form action='/' method=get>");
                    client.println("<input type=text name='stoerica' value=''>");
                    client.println("<input type=submit name='submit' value='submit'>");
                    client.println("</form>");
                    client.println(sekundeStoperica);
                    client.println("<br>");

                    // Form for odbrojavanje
                    client.println("<form action='/' method=get>");
                    client.println("Odbrojavanje: ");
                    client.println("<input type=time name='odbrojavanje' value=''>");
                    client.println("<input type=submit name='submit' value='submit'>");
                    client.println("</form>");
                    client.println("</html>");
                    break;
                }
                if (c == '\n')
                {
                    // you're starting a new line
                    currentLineIsBlank = true;
                }
                else if (c != '\r')
                {
                    // you've gotten a character on the current line
                    currentLineIsBlank = false;
                }
            }
        }
        // give the web browser time to receive the data
        delay(1);
        // close the connection:
        client.stop();

        // read input for budilica
        if (readString.indexOf("budilica") > 0)
        {
            String budilica = readString;
            String b_Hour;
            String b_Minute;
            budilica.remove(0, budilica.indexOf("=") + 1);
            budilica.remove(budilica.indexOf("&"));

            //get hour
            b_Hour = budilica;
            b_Hour.remove(b_Hour.indexOf("%"));
            i_Hourb = b_Hour.toInt();

            //get minute
            b_Minute = budilica;
            b_Minute.remove(0, b_Minute.indexOf("A") + 1);
            i_Minuteb = b_Minute.toInt();

            alarmSet = 1;

        }


        // read input for stoperica
        if (readString.indexOf("stoerica") > 0)
        {
            if (readString.indexOf("start") > 0)
            {
                // set a variable to current time
                Serial.println("START stoperica");
                stoperica = 1;
            }
            if (readString.indexOf("stop") > 0)
            {
                // set a variable to current time
                // stop_time - start_time = stoperica time ??? 
                Serial.println("STOP stoperica");
                stoperica = 0;
                sekundeStoperica = 0;
                stopericaNot = 15;
            }
        }

        // read input for odbrojavanje
        if (readString.indexOf("odbrojavanje") > 0)
        {
            String odbrojavanje = readString;
            String o_Hour;
            String o_Minute;

            odbrojavanje.remove(0, odbrojavanje.indexOf("=") + 1);
            odbrojavanje.remove(odbrojavanje.indexOf("&"));

            //get hour
            o_Hour = odbrojavanje;
            o_Hour.remove(o_Hour.indexOf("%"));
            i_Houro = o_Hour.toInt();

            //get minute
            o_Minute = odbrojavanje;
            o_Minute.remove(0, o_Minute.indexOf("A") + 1);
            i_Minuteo = o_Minute.toInt();

            i_Secondo = i_Houro * 3600 + i_Minuteo * 60;


            odbrojavanjeSet = 1;

        }


        //clearing string for next read
        readString = "";
    }

    if (alarmSet == 1)
    {
        if (rtc.getTime().hour == i_Hourb)
        {
            if (rtc.getTime().min == i_Minuteb)
            {
                digitalWrite(ledPin1, HIGH);
                digitalWrite(ledPin2, HIGH);
                tone(buzzerPin, 500);
                alarm = 1;
                alarmSet = 0;
            }
        }
    }
    if (alarm != 0)
    {
        alarm++;


        if (alarm > 15)
        {
            digitalWrite(ledPin1, LOW);
            digitalWrite(ledPin2, LOW);
            noTone(buzzerPin);
            alarm = 0;

        }
    }
    if (alarm > 0 && alarm < 16)
    {
        lcd.print("BUDILICA");
    }
    if (stoperica == 1)
    {
        sekundeStoperica++;
        lcd.print(sekundeStoperica);
    }

    if (stopericaNot > 0)
    {
        stopericaNot--;
        lcd.print("STOPERICA STOP");
    }

    if (odbrojavanjeSet == 1 && i_Secondo >= 0)
    {
        lcd.print(i_Secondo);

        i_Secondo--;
    }
    if (odbrojavanjeSet == 1 && i_Secondo > -15 && i_Secondo < 0)
    {
        tone(buzzerPin, 500);
        i_Secondo--;
        lcd.print("VRIJEME ISTEKLO");
    }
    if (odbrojavanjeSet == 1 && i_Secondo == -15)
    {
        noTone(buzzerPin);
        odbrojavanjeSet = 0;
        i_Secondo = 0;
    }



    delay(1000);



}
