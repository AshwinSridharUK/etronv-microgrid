#include <dht.h>


#define DHT_PIN 6 // DHT11 Sensor Digital Pin
 
dht DHT;

enum: byte {IDLE,DHT11ALERT} currentState= IDLE;


void setup() {
   Serial.begin(9600);
  delay(500);//Delay to let system boot
  Serial.println("DHT11 Humidity & temperature Sensor\n\n");
  delay(1000);//Wait before accessing Sensor

}

void loop() {
  switch (currentState){
    case IDLE:
    DHT.read11(DHT_PIN);
   
    Serial.print("Current humidity = ");
    Serial.print(DHT.humidity);
    Serial.print("%  ");
    Serial.print("temperature = ");
    Serial.print(DHT.temperature); 
    Serial.println("C  ");
    delay (5000);

    if (DHT.humidity >60){
      Serial.println("High Humidity");
      currentState = DHT11ALERT;
      
      
    }
    break;
    
    case DHT11ALERT:
    DHT.read11(DHT_PIN);
    Serial.println("Too Hot");
    Serial.println(DHT.humidity);
    delay(5000);
    if(DHT.humidity <55){
    currentState= IDLE;
    }
    break;
    
  }
  

}
