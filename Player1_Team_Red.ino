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

const int irReceiverPin1 = 35;
const int irReceiverPin2 = 21;
const int irReceiverPin3 = 19;

IRrecv irrecv1(irReceiverPin1);
IRrecv irrecv2(irReceiverPin2);
IRrecv irrecv3(irReceiverPin3);

decode_results results;

volatile bool irSignalDetected = false;
volatile int triggeringPin = -1;
volatile bool processingIRSignal = false;

int playerScore = 50;
bool playerOut = false;
String playerID = "Player1";
String team = "Red";

WebsocketsClient webSocket;
bool webSocketConnected = false;

unsigned long lastWebSocketAttemptTime = 0;
const unsigned long reconnectInterval = 5000;

bool gameStarted = false;
unsigned long gameTimer = 800000;  // 10 minutes in milliseconds
unsigned long gameStartTime = 0;

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
  irrecv2.enableIRIn();
  // irrecv3.enableIRIn();

  attachInterrupt(digitalPinToInterrupt(irReceiverPin1), irISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(irReceiverPin2), irISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(irReceiverPin3), irISR, CHANGE);

  startGameTimer();  // Start the game timer
}

void loop() {
  webSocket.poll();

  if (gameStarted && millis() - gameStartTime >= gameTimer) {
    endGame();
  }

  if (irSignalDetected && !processingIRSignal && gameStarted) {
    processingIRSignal = true;
    irSignalDetected = false;

    if (triggeringPin == irReceiverPin1) {
      processIRSignal(irrecv1, irReceiverPin1);
    } 
    else if (triggeringPin == irReceiverPin2) {
      processIRSignal(irrecv2, irReceiverPin2);
    } 
    else if (triggeringPin == irReceiverPin3) {
      processIRSignal(irrecv3, irReceiverPin3);
    }

    processingIRSignal = false;
  }

  if (digitalRead(BUTTON_PIN) == LOW && !playerOut && gameStarted) {
    IrSender.sendNEC(0x8000F00F, 32);
    // Serial.println("Fire Is Shout");
    delay(500);
  }
}

void irISR() {
    irSignalDetected = true;
    if (digitalRead(irReceiverPin1) == LOW) {
      triggeringPin = irReceiverPin1;
    } 
    else if (digitalRead(irReceiverPin2) == LOW) {
      triggeringPin = irReceiverPin2;
    } 
    else if (digitalRead(irReceiverPin3) == LOW) {
      triggeringPin = irReceiverPin3;
    }
  }


void processIRSignal(IRrecv &irrecv, int receiverPin) {
  if (irrecv.decode()) {
    Serial.print("Received IR signal from receiver on pin: ");
    Serial.println(receiverPin);
    Serial.print("Hex Code: ");
    Serial.println(irrecv.decodedIRData.decodedRawData, HEX);

    if (irrecv.decodedIRData.decodedRawData == 0xF00F0002 && !playerOut) {
      playerScore--;
      Serial.print("Player hit! New total score: ");
      Serial.println(playerScore);

      sendHitDataToWebSocket();

      if (playerScore <= 0) {
        playerOut = true;
        Serial.println("Player is out of the game!");
        sendPlayerOutMessage();
      }
    }

    // attachInterrupt(digitalPinToInterrupt(irReceiverPin1), irISR, CHANGE);
    // // attachInterrupt(digitalPinToInterrupt(irReceiverPin2), irISR, CHANGE);
    // // attachInterrupt(digitalPinToInterrupt(irReceiverPin3), irISR, CHANGE);

    // irrecv.resume();
  }
}

void startGameTimer() {
  gameStartTime = millis();
  gameStarted = true;
  Serial.println("Game started! Timer set for 10 minutes.");

  String jsonPayload = "{\"event\":\"gameStart\", \"timer\":\"10:00\"}";
  if (webSocketConnected) {
    webSocket.send(jsonPayload);
  }
}

void endGame() {
  gameStarted = false;
  Serial.println("Game over! No further scores counted.");

  String jsonPayload = "{\"event\":\"gameEnd\"}";
  if (webSocketConnected) {
    webSocket.send(jsonPayload);
  }
}

void sendHitDataToWebSocket() {
  if (webSocketConnected) {
    String jsonPayload = "{\"playerID\":\"" + playerID + "\", \"team\":\"" + team + "\", \"score\":" + String(playerScore) + "}";
    Serial.print("Sending WebSocket message with payload: ");
    Serial.println(jsonPayload);
    webSocket.send(jsonPayload);
  } else {
    Serial.println("WebSocket not connected");
  }
}