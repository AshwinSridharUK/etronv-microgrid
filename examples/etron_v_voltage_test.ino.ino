#define VOLTAGE_PIN A0 //Voltage Sensor Analog Pin
// VOLTAGE SENSOR
float adcVoltage = 0.0;
float inVoltage = 0.0;
float R1 = 30000.0;
float R2 = 7500.0;
float refVoltage = 5.0;
int adcValue = 0;

void setup(){
   // Setup Serial Monitor
   Serial.begin(9600);
   Serial.println("DC Voltmeter Test");
}
void loop() {

adcValue = analogRead(VOLTAGE_PIN);

      
        adcVoltage  = (adcValue * refVoltage) / 1024.0;  //Sensor reports ratiometric value which ranges between 5 to 1024 for anything less than 5V.

        
        inVoltage = adcVoltage / (R2 / (R1 + R2)); //Potential Divider Calculation 

       
        Serial.print("Input Voltage = "); //Prints values to serial monitor
        Serial.println(inVoltage, 2);

}
