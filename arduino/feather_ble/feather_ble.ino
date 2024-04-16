#include <Wire.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define STEP_DATA_CHAR_UUID "beefcafe-36e1-4688-b7f5-00000000000b"

BLECharacteristic *pStepDataCharacteristic;

const int MPU_ADDR = 0x68; // I2C address of MPU-6050
int16_t accelX, accelY, accelZ;
int stepCount = 0;
float accMagnitudePrev = 0;

// Define MyServerCallbacks class before using it in setup()
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      BLEDevice::stopAdvertising(); // Optional: stop advertising when connected
      Serial.println("Client connected");
    }

    void onDisconnect(BLEServer* pServer) {
      BLEDevice::startAdvertising(); // Restart advertising
      Serial.println("Client disconnected, start advertising again");
    }
};

void setup() {
  Wire.begin();
  Serial.begin(9600);
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);  // PWR_MGMT_1 register
  Wire.write(0);     // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);
  delay(2000);

  BLEDevice::init("Step-Sense");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks()); // Set the server callbacks

  BLEService *pService = pServer->createService(SERVICE_UUID);
  pStepDataCharacteristic = pService->createCharacteristic(
    STEP_DATA_CHAR_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  pStepDataCharacteristic->addDescriptor(new BLE2902());

  pService->start();
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMaxPreferred(0x20);
  BLEDevice::startAdvertising();
  Serial.println("BLE device is ready to be connected");
}

void loop() {
  // put your main code here, to run repeatedly:
  readAccelerometerData();
  detectStep();
  delay(100);
}

void readAccelerometerData() {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B); // Starting register for accelerometer data
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 6, true);

  accelX = Wire.read() << 8 | Wire.read();
  accelY = Wire.read() << 8 | Wire.read();
  accelZ = Wire.read() << 8 | Wire.read();
  
  Serial.print("X: ");
  Serial.print(accelX);
  Serial.print(" Y: ");
  Serial.print(accelY);
  Serial.print(" Z: ");
  Serial.println(accelZ);
}

void detectStep() {
  float accX = accelX / 16384.0; 
  float accY = accelY / 16384.0;
  float accZ = accelZ / 16384.0;

  // Calculate the magnitude of acceleration
  // accX * accX is equivalent to pow(accX, 2)
  float accMagnitude = sqrt(accX * accX + accY * accY + accZ * accZ);
  // float accMagnitude = sqrt(accX * accX + accY * accY + accZ * accZ);: This line calculates the magnitude of the acceleration vector using the calculated acceleration values for the X, Y, and Z axes. The sqrt() function is used to calculate the square root of the sum of the squared acceleration values.

  // Peak detection
  if (accMagnitudePrev > accMagnitude + 0.1 && accMagnitudePrev > 1.5) {
    stepCount++;
    Serial.print("Step: ");
    Serial.println(stepCount);
    // Create a string containing the step count data
    String stepData = String(stepCount);
    // Update the characteristic value
    pStepDataCharacteristic->setValue(stepData.c_str());
    pStepDataCharacteristic->notify();
  }
  accMagnitudePrev = accMagnitude;
}
