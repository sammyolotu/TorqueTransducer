#include <HardwareSerial.h>

HardwareSerial MySerial(1); // define a Serial for UART1
const int MySerialRX = 27;
const int MySerialTX = 26;
int receivedCounter = 0; // Variable to store received data

void setup() {
  Serial.begin(115200);
  MySerial.begin(115200, SERIAL_8N1, MySerialRX, MySerialTX);
}

void loop() {
  if (MySerial.available() >= sizeof(receivedCounter)) {
    MySerial.readBytes((char*)&receivedCounter, sizeof(receivedCounter)); // Read the incoming bytes into the integer
    Serial.println(receivedCounter); // Print the integer value
  }
}
