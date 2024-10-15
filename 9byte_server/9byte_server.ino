// ESP32 BLE Server Code

#include <HardwareSerial.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// UART Configuration
HardwareSerial MySerial(1); // Define a Serial for UART1
const int MySerialRX = 26;
const int MySerialTX = 27;

// BLE Configuration
BLEServer* pServer = NULL;
BLECharacteristic* dataFromServerCharacteristic = NULL; // Characteristic to send data to client
BLECharacteristic* dataToServerCharacteristic = NULL;   // Characteristic to receive data from client
bool deviceConnected = false;
bool oldDeviceConnected = false;

// UUIDs
#define SERVICE_UUID                          "19b10000-e8f2-537e-4f6c-d104768a1214"
#define DATA_FROM_SERVER_CHARACTERISTIC_UUID  "19b10001-e8f2-537e-4f6c-d104768a1214"
#define DATA_TO_SERVER_CHARACTERISTIC_UUID    "19b10002-e8f2-537e-4f6c-d104768a1214"

// Callback class for BLE server
class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
  }

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
  }
};

// Function to process received data from BLE client
void processReceivedData(uint8_t* receivedData, size_t length) {
  Serial.println("Processing received data...");
  // Send the received data over UART
  MySerial.write(receivedData, length);
  Serial.println("Sent data over UART:");
  for (int i = 0; i < length; i++) {
    Serial.print("Byte ");
    Serial.print(i);
    Serial.print(": ");
    Serial.println(receivedData[i], HEX);
  }

  // Wait for response from UART (14 bytes)
  const size_t expectedResponseLength = 14;
  size_t bytesRead = 0;
  uint8_t uartReceivedData[expectedResponseLength];
  unsigned long startTime = millis();
  const unsigned long timeout = 2000; // 2 seconds timeout

  // Read data from UART with timeout
  while (bytesRead < expectedResponseLength && (millis() - startTime) < timeout) {
    if (MySerial.available()) {
      uartReceivedData[bytesRead++] = MySerial.read();
    }
  }

  if (bytesRead == expectedResponseLength) {
    Serial.println("Received data from UART:");
    for (int i = 0; i < expectedResponseLength; i++) {
      Serial.print("Byte ");
      Serial.print(i);
      Serial.print(": ");
      Serial.println(uartReceivedData[i], HEX);
    }

    // Send the data back to the BLE client
    if (deviceConnected) {
      dataFromServerCharacteristic->setValue(uartReceivedData, expectedResponseLength);
      dataFromServerCharacteristic->notify();
      Serial.println("Notified BLE client with UART data.");
    }
  } else {
    Serial.println("Failed to receive complete data from UART within timeout.");
  }
}

// Callback class for data received from BLE client
class DataToServerCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* pCharacteristic) {
    uint8_t* data = pCharacteristic->getData();
    size_t length = pCharacteristic->getLength();
    if (length == 9) {
      // Process the received data
      processReceivedData(data, length);
    } else {
      Serial.println("Received incorrect data length from client.");
    }
  }
};

void setup() {
  Serial.begin(115200);

  // Initialize UART
  MySerial.begin(250000, SERIAL_8N1, MySerialRX, MySerialTX);

  // Create the BLE Device
  BLEDevice::init("ESP32");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService* pService = pServer->createService(SERVICE_UUID);

  // Create the characteristic to send data to client
  dataFromServerCharacteristic = pService->createCharacteristic(
    DATA_FROM_SERVER_CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_NOTIFY
  );

  // Create the characteristic to receive data from client
  dataToServerCharacteristic = pService->createCharacteristic(
    DATA_TO_SERVER_CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_WRITE
  );

  // Register the callback for data received from client
  dataToServerCharacteristic->setCallbacks(new DataToServerCallbacks());

  // Add descriptors
  dataFromServerCharacteristic->addDescriptor(new BLE2902());
  dataToServerCharacteristic->addDescriptor(new BLE2902());

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x00); // Avoid issues with iOS devices
  BLEDevice::startAdvertising();
  Serial.println("Waiting for a client connection...");
}

void loop() {
  // Handle device connection status
  if (deviceConnected && !oldDeviceConnected) {
    // Device just connected
    oldDeviceConnected = deviceConnected;
    Serial.println("Device connected.");
  }
  if (!deviceConnected && oldDeviceConnected) {
    // Device just disconnected
    oldDeviceConnected = deviceConnected;
    Serial.println("Device disconnected.");
    delay(500); // Allow time for the BLE stack to reset
    pServer->startAdvertising(); // Restart advertising
    Serial.println("Restarted advertising.");
  }

  // Small delay to prevent task flooding
  delay(20);
}
