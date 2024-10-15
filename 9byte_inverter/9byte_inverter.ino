// ESP32 Inverter Code

#include <HardwareSerial.h>

HardwareSerial MySerial(1); // define a Serial for UART1

const int MySerialRX = 27;
const int MySerialTX = 26;

uint8_t receivedData[9] = {0}; // Buffer to store received 9-element array
uint8_t transmitData[14] = {0}; // Buffer to store 14-element array for transmission

void setup() {
  Serial.begin(115200);
  MySerial.begin(250000, SERIAL_8N1, MySerialRX, MySerialTX);
}

void loop() {
  // Check if we have received at least 9 bytes
  if (MySerial.available() >= 9) {
    // Read 9 bytes into the receivedData array
    MySerial.readBytes(receivedData, 9);

    // Process received data (Example: Generate 14 bytes from 9 bytes)
    // For demonstration, we'll fill transmitData with some computed values
    for (int i = 0; i < 9; i++) {
    transmitData[i] = receivedData[i]; // Example operation
    transmitData[i + 9] = i; // Fill remaining bytes with index values 
    }

    // Transmit 14-element array back
    MySerial.write(transmitData, sizeof(transmitData));

    // Debug output to Serial Monitor
    Serial.print("Received Data: ");
    for (int i = 0; i < 9; i++) {
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

  delay(100); // Adjust delay as needed
}
