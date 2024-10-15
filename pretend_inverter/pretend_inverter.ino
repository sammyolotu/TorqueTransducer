//code for inverter

#include <HardwareSerial.h>

HardwareSerial MySerial(1); // define a Serial for UART1

const int MySerialRX = 27;
const int MySerialTX = 26;

uint8_t receivedData[7] = {0}; // Buffer to store received 8-element array
uint8_t transmitData[14] = {0}; // Buffer to store 14-element array for transmission

void setup() {
  Serial.begin(115200);
  MySerial.begin(250000, SERIAL_8N1, MySerialRX, MySerialTX);
}

void loop() {
  // Check if we have received at least 7 bytes
  if (MySerial.available() >= 7) {
    // Read 7 bytes into the receivedData array
    MySerial.readBytes(receivedData, 7);

    // Process received data ourselves since we have nothing to send back (example: double the received array)
    for (int i = 0; i < 7; i++) {
    transmitData[i] = receivedData[i];
    transmitData[i + 7] = receivedData[i]; // Duplicate data
    }

    // Transmit 14-element array back
    MySerial.write(transmitData, sizeof(transmitData));

    // Debug output to Serial Monitor
    Serial.print("Received Data: ");
    for (int i = 0; i < 7; i++) {
      Serial.print(receivedData[i], HEX);
      Serial.print(" ");
    }
    Serial.println();

    Serial.print("Transmitted Data: ");
    for (int i = 0; i < 14; i++) {
      Serial.print(transmitData[i], HEX);
      Serial.print(" ");
    }
    Serial.println();
  }

  delay(1000); // Add delay for readability
}
