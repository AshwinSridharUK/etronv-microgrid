#include <dht.h> //DHT11 Sensor Library
#include <SoftwareSerial.h> //ESP-01 Communication via TX RX Library
#include <SDHCI.h>
#include <File.h>
#include <GNSS.h>
static SpGnss Gnss;
SpGnssTime current_time;

SDClass SD;  /**< SDClass object */
File myFile;

#define DHT_PIN 6 // DHT11 Sensor Digital Pin
#define VOLTAGE_PIN A0 //Voltage Sensor Analog Pin
#define LIGHT_PIN A1 //Light Sensor (KY-018) Pin

//ESP-01 WIFI DETAILS
String ssid = "YOURSSID";                 //  Network SSID
String pwd = "YOURPWD";           // Network Pasword


int rxPin = 0;                                               //ESP8266 RX Pin
int txPin = 1;                                               //ESP8266 TX Pin


String ip = "184.106.153.149";                                //Thingspeak IP Address
float tempDht, humDht;



// KY-018 PHOTORESISTOR SENSOR
int lightSensorValue;

// VOLTAGE SENSOR
float adcVoltage = 0.0;
float inVoltage = 0.0;
float R1 = 30000.0;
float R2 = 7500.0;
float refVoltage = 5.0;
int adcValue = 0;
String dataSend;

// DHT11 HUMIDITY AND TEMP. SENSOR
dht DHT;

float navLat;
float navLon;
int navNumSat;
String navTime ;



//STATE CONTROL
enum : byte {IDLE, DHT11, VOLTAGE, LIGHT, RECORD, GPSLOCATE, PUBLISH} currentState = IDLE;






SoftwareSerial esp(rxPin, txPin);







void setup() {

  Serial.begin(9600);
  pinMode(LIGHT_PIN, INPUT);
  delay(500);
  Serial.println("DHT11 Humidity & temperature Sensor\n\n");
  delay(1000);
  Serial.println("Started");
  esp.begin(115200);
  esp.println("AT");
  Serial.println("AT  sent ");
  while (!esp.find("OK")) {
    esp.println("AT");
    Serial.println("ESP8266 Not Found.");
  }
  Serial.println("OK Command Received");
  esp.println("AT+CWMODE=1");
  while (!esp.find("OK")) {
    esp.println("AT+CWMODE=1");
    Serial.println("Setting is ....");
  }
  Serial.println("Set as client");
  Serial.println("Connecting to the Network ...");
  esp.println("AT+CWJAP=\"" + ssid + "\",\"" + pwd + "\"");
  while (!esp.find("OK"));
  Serial.println("Connected to the network.");
  delay(1000);
  Serial.println("Insert SD card.");
  while (!SD.begin()) {
    ; /* wait until SD card is mounted. */
  }

  /* Create a new directory */
  SD.mkdir("dir/");

  /* Open the file. Note that only one file can be open at a time,
     so you have to close this one before opening another. */
  myFile = SD.open("dir/test.txt", FILE_WRITE);
  Gnss.begin();
  Gnss.select(GPS);
  double lat = 51.593830;
  double lon = -0.264000;
  Gnss.start(HOT_START);




}

void loop() {
  switch (currentState) {
    case IDLE: {
        esp.println("AT+CIPSTART=\"TCP\",\"" + ip + "\",80");
        if (esp.find("Error")) {
          Serial.println("AT+CIPSTART Error");
        }
        DHT.read11(DHT_PIN);
        tempDht = DHT.temperature;
        humDht = DHT.humidity;

        Serial.print("Current humidity = ");
        Serial.print(DHT.humidity);
        Serial.print("%  ");
        Serial.print("temperature = ");
        Serial.print(DHT.temperature);
        Serial.println("C  ");
        delay (5000);

        if (DHT.temperature > 60) {
          Serial.println("High Temp");



        }
        else {
          currentState = VOLTAGE;
        }
        break;
      }



    case VOLTAGE: {

        adcValue = analogRead(VOLTAGE_PIN);

        // Determine voltage at ADC input
        adcVoltage  = (adcValue * refVoltage) / 1024.0;

        // Calculate voltage at divider input
        inVoltage = adcVoltage / (R2 / (R1 + R2));

        // Print results to Serial Monitor to 2 d.p.
        Serial.print("Input Voltage = ");
        Serial.println(inVoltage, 2);
        currentState = LIGHT;
        break;
      }

    case LIGHT: {
        lightSensorValue = analogRead(LIGHT_PIN);
        // put your main code here, to run repeatedly:
        Serial.println(lightSensorValue, DEC);
        currentState = GPSLOCATE;
        break;
      }
    case GPSLOCATE : {
        if (Gnss.waitUpdate(-1))
        {
          /* Get navigation data. */
          SpNavData NavData;
          Gnss.getNavData(&NavData);

          /* Print position and satellite count. */

          navLat = (NavData.latitude, 6);
     

          navLon = (NavData.longitude, 6);
        
          navNumSat = NavData.numSatellites;
          
          navTime = current_time.hour;
          
          
          currentState= RECORD;


          break;
        }
      }
    case RECORD: {

        if (myFile) {
          myFile.println(humDht);
          myFile.println(tempDht);
          myFile.println(inVoltage);
          myFile.println(lightSensorValue);
          myFile.println(navLon);
          myFile.println(navLat);
          myFile.println(navNumSat);
          myFile.println(navTime);
          myFile.close();
          Serial.println("done.");
        } else {
          /* If the file didn't open, print an error */
          Serial.println("error opening test.txt");
        }

        /* Re-open the file for reading */
        myFile = SD.open("dir/test.txt");

        if (myFile) {
          Serial.println("test.txt:");

          /* Read from the file until there's nothing else in it */
          while (myFile.available()) {
            Serial.write(myFile.read());
          }
          /* Close the file */
          myFile.close();
        } else {
          /* If the file didn't open, print an error */
          Serial.println("error opening test.txt");
        }
        delay(5000);

        currentState = PUBLISH;




        break;
      }
    


    case PUBLISH: {
        dataSend = "GET https://api.thingspeak.com/update?api_key=YOURKEY";   // API Thingspeak Key- Replace with your own key
        dataSend += "&field1=";
        dataSend += String(tempDht);
        dataSend += "&field2=";
        dataSend += String(humDht);
        dataSend += "&field3=";
        dataSend += String(lightSensorValue);
        dataSend += "&field4=";
        dataSend += String(inVoltage);

        dataSend += "\r\n\r\n";
        esp.print("AT+CIPSEND=");
        esp.println(dataSend.length() + 2);
        delay(2000);
        if (esp.find(">")) {
          esp.print(dataSend);
          Serial.println(dataSend);
          Serial.println("Data sent.");
          delay(1000);
        }
        Serial.println("Connection Closed.");
        esp.println("AT+CIPCLOSE");
        delay(1000);
        currentState = IDLE;
        break;
      }





  }


}
