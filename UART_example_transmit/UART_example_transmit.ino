#include <HardwareSerial.h>

HardwareSerial MySerial(1); // define a Serial for UART1
const int MySerialRX = 26;
const int MySerialTX = 27;
int counter = 0; // Initialize counter

void setup() {
  Serial.begin(115200);
  MySerial.begin(115200, SERIAL_8N1, MySerialRX, MySerialTX);
}

void loop() {
  MySerial.write((byte*)&counter, sizeof(counter)); // Send counter as bytes over UART
  Serial.println(counter); // Debug output to Serial Monitor
  counter++;
  delay(1000);
}
