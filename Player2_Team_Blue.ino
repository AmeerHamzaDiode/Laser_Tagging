#include <WiFi.h>
#include <HTTPClient.h>
#include <IRremote.h>
#include <ArduinoWebsockets.h>

using namespace websockets;

const char* ssid = "ES";           
const char* password = "Electro@2024"; 
const char* websocketServerURL = "ws://192.168.18.120:8080/"; 


const int IR_LED_PIN = 2;
const int BUTTON_PIN = 4;

const int irReceiverPin1 = 34;
// const int irReceiverPin2 = 35;
// const int irReceiverPin3 = 32;  


  
IRrecv irrecv1(irReceiverPin1);  
// IRrecv irrecv2(irReceiverPin2);  
// IRrecv irrecv3(irReceiverPin3); 

decode_results results;

volatile bool irSignalDetected = false;  
volatile int triggeringPin = -1; 
volatile bool processingIRSignal = false;  

int playerScore = 20;
bool playerOut = false;  
String playerID = "Player2";
String team = "Blue"; 


WebsocketsClient webSocket; 
bool webSocketConnected = false; 

unsigned long lastWebSocketAttemptTime = 0;  
const unsigned long reconnectInterval = 5000;  

  
void initializeWebSocket() {
  webSocket.onMessage([](WebsocketsMessage message) {
    Serial.printf("Received message: %s\n", message.data());
  });

  webSocket.onEvent([](WebsocketsEvent event, String data) {
    if (event == WebsocketsEvent::ConnectionOpened) {
      Serial.println("WebSocket Connected");
      webSocketConnected = true;
    } else if (event == WebsocketsEvent::ConnectionClosed) {
      Serial.println("WebSocket Disconnected");
      webSocketConnected = false;
    }
  });

  
  if (webSocket.connect(websocketServerURL)) {
    Serial.println("Connected to WebSocket server");
  } else {
    Serial.println("Failed to connect to WebSocket server");
  }
}


void sendPlayerOutMessage() {
  if (webSocketConnected) {
   
    String jsonPayload = "{\"playerID\":\"" + playerID + "\", \"status\":\"out\"}";

    Serial.print("Sending Player Out message with payload: ");
    Serial.println(jsonPayload);

    // Send message to WebSocket server
    webSocket.send(jsonPayload);
  } else {
    Serial.println("WebSocket not connected");
  }
}

void setup() {

  Serial.begin(9600);

  IrSender.begin(IR_LED_PIN);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

    delay(2000);




  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi");

 
  initializeWebSocket();

    delay(1000);


  
  irrecv1.enableIRIn();  
  // irrecv2.enableIRIn();  
  // irrecv3.enableIRIn();  


 
  attachInterrupt(digitalPinToInterrupt(irReceiverPin1), irISR, CHANGE);
  // attachInterrupt(digitalPinToInterrupt(irReceiverPin2), irISR, CHANGE);
  // attachInterrupt(digitalPinToInterrupt(irReceiverPin3), irISR, CHANGE);

}

void loop() {
    webSocket.poll();  

  
    if (digitalRead(BUTTON_PIN) == LOW) {
      IrSender.sendNEC(0x4000F00F, 32);
    delay(500);
    }                             

    
    if (irSignalDetected && !processingIRSignal) {
    processingIRSignal = true;  
    irSignalDetected = false;  

    if (triggeringPin == irReceiverPin1) {
      processIRSignal(irrecv1, irReceiverPin1);
    } 
    // else if (triggeringPin == irReceiverPin2) {
    //   processIRSignal(irrecv2, irReceiverPin2);
    // } 
    // else if (triggeringPin == irReceiverPin3) {
    //   processIRSignal(irrecv3, irReceiverPin3);
    // } 

    processingIRSignal = false; 
  }


}


void irISR() {
  if (!processingIRSignal) {  
    irSignalDetected = true;
    if (digitalRead(irReceiverPin1) == LOW) {
      triggeringPin = irReceiverPin1;
    } 
    // else if (digitalRead(irReceiverPin2) == LOW) {
    //   triggeringPin = irReceiverPin2;
    // } 
    // else if (digitalRead(irReceiverPin3) == LOW) {
    //   triggeringPin = irReceiverPin3;
    // }
  }
}
void processIRSignal(IRrecv &irrecv, int receiverPin) {
  if (irrecv.decode()) {
    Serial.print("Received IR signal from receiver on pin: ");
    Serial.println(receiverPin);
    Serial.print("Hex Code: ");
    Serial.println(irrecv.decodedIRData.decodedRawData, HEX);

    // Check if the IR code is a hit signal
    if (irrecv.decodedIRData.decodedRawData == 0xF00F0001 && !playerOut) {
      playerScore--;
      Serial.print("Player hit! New total score: ");
      Serial.println(playerScore);

      // Send hit data to WebSocket server
      sendHitDataToWebSocket();

      // Check if player is out
      if (playerScore <= 0) {
        playerOut = true;
        Serial.println("Player is out of the game!");

        // Send "Player Out" message to WebSocket server
        sendPlayerOutMessage();
      }
    }

    // Re-enable interrupts for all IR receivers after processing
    attachInterrupt(digitalPinToInterrupt(irReceiverPin1), irISR, CHANGE);
    // attachInterrupt(digitalPinToInterrupt(irReceiverPin2), irISR, CHANGE);
    // attachInterrupt(digitalPinToInterrupt(irReceiverPin3), irISR, CHANGE);


    irrecv.resume();  // Resume receiving the next IR signal
  }
}

void checkReceiver(int receiverPin) {
  // Switch to the correct receiver pin
  IrReceiver.begin(receiverPin, ENABLE_LED_FEEDBACK);

  if (IrReceiver.decode()) {
    Serial.print("Received IR signal from receiver on pin: ");
    Serial.println(receiverPin);
    Serial.print("Hex Code: ");
    Serial.println(IrReceiver.decodedIRData.decodedRawData, HEX);

    // Check if the received IR signal matches the expected code
    if (IrReceiver.decodedIRData.decodedRawData == 0xF00F0001) {
      processingIRSignal = true;

      playerScore--;
      Serial.print("Player hit! New total score: ");
      Serial.println(playerScore);

      delay(500);

      sendHitDataToWebSocket();



      if (playerScore <= 0) {
        Serial.println("Player is out of the game!");
      }

      processingIRSignal = false;
    }

    IrReceiver.resume();  // Resume receiving the next IR signal
  }
}

void sendHitDataToWebSocket() {
  if (webSocketConnected) { // Check if WebSocket is connected
    // Prepare the JSON payload
String jsonPayload = "{\"playerID\":\"" + playerID + "\", \"team\":\"" + team + "\", \"score\":" + String(playerScore) + "}";

    Serial.print("Sending WebSocket message with payload: ");
    Serial.println(jsonPayload);

    // Send message to WebSocket server
    webSocket.send(jsonPayload);
  } else {
    Serial.println("WebSocket not connected");
  }
}

