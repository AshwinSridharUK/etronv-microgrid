#define LIGHT_PIN A1
int lightSensorValue;

void setup() {
  pinMode(Sensor,INPUT);
  Serial.begin(115200);
  // put your setup code here, to run once:

}

void loop() {
  sensorValue=analogRead(LIGHT_PIN);
  
  Serial.println(lightSensorValue,DEC);
  delay(5000);
}
