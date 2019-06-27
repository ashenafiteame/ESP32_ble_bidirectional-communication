/*
    Video: https://www.youtube.com/watch?v=oCMOYS71NIU
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleNotify.cpp
    Ported to Arduino ESP32 by Evandro Copercini

   Create a BLE server that, once we receive a connection, will send periodic notifications.
   The service advertises itself as: 6E400001-B5A3-F393-E0A9-E50E24DCCA9E
   Has a characteristic of: 6E400002-B5A3-F393-E0A9-E50E24DCCA9E - used for receiving data with "WRITE"
   Has a characteristic of: 6E400003-B5A3-F393-E0A9-E50E24DCCA9E - used to send data with  "NOTIFY"

   The design of creating the BLE server is:
   1. Create a BLE Server
   2. Create a BLE Service
   3. Create a BLE Characteristic on the Service
   4. Create a BLE Descriptor on the characteristic
   5. Start the service.
   6. Start advertising.

   In this example rxValue is the data received (only accessible inside that function).
   And txValue is the data to be sent, in this example just a byte incremented every second.
*/
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

BLECharacteristic *pCharacteristic;
bool deviceConnected = false;
float txValue = 0;
const int LED = 2; // Could be different depending on the dev board. I used the DOIT ESP32 dev board.
const int PushButtonPin=13;     // the number of the pushbutton pin
// variables will change:
int buttonState = 0;         // variable for reading the pushbutton status

//std::string rxValue; // Could also make this a global var to access it in loop()

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    // data(string) accepting from app
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();// data(string) accepting from app
      // std::string rxValue2 = pCharacteristic->getRssi();
      if (rxValue.length() > 0) {
        Serial.println("*********");
        Serial.print("Received Value: ");

        for (int i = 0; i < rxValue.length(); i++) {
          Serial.print(rxValue[i]);
        }
        Serial.println();

        // Do stuff based on the command received from the app
        if (rxValue.find("A") != -1) {
           Serial.println("Turning OFF!");
          digitalWrite(LED, LOW);
        }
        else if (rxValue.find("B") != -1) {
          digitalWrite(LED, HIGH);
          Serial.println("Turning ON!");
         
        }
        Serial.println();
        Serial.println("*********");
      }
    }
};



void setup() {
  Serial.begin(115200);
  pinMode(LED, OUTPUT);
  pinMode(PushButtonPin, INPUT);
 // Create the BLE Device
  BLEDevice::init("FindMe"); // Give it a name

  // 1. Create the BLE Server
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());// calling callbacks

  // 2. Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);
  // 3. Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_TX, BLECharacteristic::PROPERTY_NOTIFY);

  //4. Create the BLE Descriptor on the characteristic
  pCharacteristic->addDescriptor(new BLE2902());
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_RX, BLECharacteristic::PROPERTY_WRITE);
  pCharacteristic->setCallbacks(new MyCallbacks());

  // 5. Start the service
  pService->start();

  // 6. Start advertising
  pServer->getAdvertising()->start();
  Serial.println("Waiting a client connection to notify...");
}

void loop() {
  if (deviceConnected) {
    int Push_button_state = digitalRead(PushButtonPin);
     if ( Push_button_state == HIGH ) { // button pressed send one that rings the phone
          txValue =1;
     }
     else{ 
        txValue =0;// else stay silent
     }
      char txString[8]; // make sure this is big
      dtostrf(txValue, 1, 2, txString); // float_val, min_width, digits_after_decimal, char_buffer
      pCharacteristic->setValue(txString);// data sending to app
      pCharacteristic->notify(); // Send the value to the app! 
  }
  delay(1000);
}
