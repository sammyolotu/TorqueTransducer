/*
  Corrected ESP32 BLE Server Code
*/

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

BLEServer* pServer = NULL;
BLECharacteristic* dataFromServerCharacteristic = NULL; // Characteristic to send data to client
BLECharacteristic* dataToServerCharacteristic = NULL;   // Characteristic to receive data from client
bool deviceConnected = false;
bool oldDeviceConnected = false;

uint8_t dataToSend[14]; // Array of 14 bytes to send to the client

// UUIDs
#define SERVICE_UUID                      "19b10000-e8f2-537e-4f6c-d104768a1214"
#define DATA_FROM_SERVER_CHARACTERISTIC_UUID "19b10001-e8f2-537e-4f6c-d104768a1214"
#define DATA_TO_SERVER_CHARACTERISTIC_UUID   "19b10002-e8f2-537e-4f6c-d104768a1214"

class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
  };

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
  }
};

void processReceivedData(uint8_t* receivedData) {
  Serial.println("Processing received data...");

  // Extract the data
  uint8_t a = receivedData[0];
  uint8_t b = receivedData[1];
  uint8_t c = receivedData[2];
  uint8_t elem4 = receivedData[3];
  uint8_t elem5 = receivedData[4];
  uint8_t e = receivedData[5];
  uint8_t f = receivedData[6];

  // Verify that elem4 + elem5 equals the sum provided by the client (Input D)
  uint16_t sumOfElements = elem4 + elem5;
  Serial.print("Sum of elements 4 and 5: ");
  Serial.println(sumOfElements);

  // send recieved data to inverter via uart. For now, double the length of recieved data and send back to client
  // we'll create the 14-byte array by repeating the received data
  for (int i = 0; i < 7; i++) {
    dataToSend[i] = receivedData[i];
    dataToSend[i + 7] = receivedData[i]; // Duplicate data
  }

  // Send the data back to the client
  if (deviceConnected) {
    dataFromServerCharacteristic->setValue(dataToSend, 14);
    dataFromServerCharacteristic->notify();
    Serial.println("Sent data to client:");
    for (int i = 0; i < 14; i++) {
      Serial.print("Byte ");
      Serial.print(i);
      Serial.print(": ");
      Serial.println(dataToSend[i]);
    }
  }
}




class DataToServerCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* pCharacteristic) {
    uint8_t* data = pCharacteristic->getData();
    size_t length = pCharacteristic->getLength();
    if (length == 7) {
      // Received 7 bytes from the client
      uint8_t receivedData[7];
      memcpy(receivedData, data, 7);
      Serial.println("Received data from client:");
      for (int i = 0; i < 7; i++) {
        Serial.print("Byte ");
        Serial.print(i);
        Serial.print(": ");
        Serial.println(receivedData[i]);
      }
      // send data from client, back to client using function from ealier
      processReceivedData(receivedData);

    } else {
      Serial.println("Received incorrect data length from client.");
    }
  }
};

void setup() {
  Serial.begin(115200);

  // Initialize the dataToSend array
  memset(dataToSend, 0, sizeof(dataToSend));

  // Create the BLE Device
  BLEDevice::init("ESP32");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

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
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);  // Set value to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();
  Serial.println("Waiting for a client connection...");
}

void loop() {
  // Disconnecting
  if (!deviceConnected && oldDeviceConnected) {
    Serial.println("Device disconnected.");
    delay(500); // Give the bluetooth stack the chance to get things ready
    pServer->startAdvertising(); // Restart advertising
    Serial.println("Start advertising");
    oldDeviceConnected = deviceConnected;
  }
  // Connecting
  if (deviceConnected && !oldDeviceConnected) {
    // get data from uart
    // place it in 14 byte array: uint8_t dataToSend[14];
    //notify
    oldDeviceConnected = deviceConnected;
    Serial.println("Device Connected");
  }
}
