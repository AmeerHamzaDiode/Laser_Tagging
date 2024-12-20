#include <WiFi.h>
#include <HTTPClient.h>
#include <IRremote.h>
#include <WebSocketsClient.h>
#include <map>


// using namespace websockets;

const char* ssid = "ES";           
const char* password = "Electro@2024"; 

const char* ws_server = "192.168.18.43";
const uint16_t ws_port = 1880;
const char* ws_path = "/ws";

WebSocketsClient webSocket;

const int IR_LED_PIN = 2;
const int BUTTON_PIN = 4;
const int MOTOR_PIN = 33;
const int irReceiverPin1 = 35;
const int irReceiverPin2 = 21;
const int irReceiverPin3 = 19;
const int irReceiverPin4 = 34;


IRrecv irrecv1(irReceiverPin1);
IRrecv irrecv2(irReceiverPin2);
IRrecv irrecv3(irReceiverPin3); 
IRrecv irrecv3(irReceiverPin4); 

decode_results results;

volatile bool irSignalDetected = false;  
volatile int triggeringPin = -1; 
volatile bool processingIRSignal = false;  

int playerScore = 50;
bool playerOut = false;
String playerID_1 = "1"; // Player ID for 0xF00F0002
String playerID_9 = "9"; // Player ID for 0x7D43196F
String team = "Red";

std::map<uint32_t, String> hexCodeToPlayerID = {
    {0x7D43196F, "9"},
    {0x7C7CB94, "9"},
    {0xCB2BE83C, "9"},
    {0x8240D5F8, "9"},
    {0x6BB3F63E, "9"},
    {0x5DDD50DC, "9"},
    {0xC5C3CCEA, "9"},
    {0xAB80729D, "9"},
    {0xA4D4714B, "9"}
};


bool gameStarted = false;
unsigned long gameTimer = 800000;  // 10 minutes in milliseconds
unsigned long gameStartTime = 0;

void webSocketEvent(WStype_t type, uint8_t *payload, size_t length) {
    switch (type) {
        case WStype_DISCONNECTED:
            Serial.println("WebSocket disconnected");
            break;
        case WStype_CONNECTED:
            Serial.println("WebSocket connected");
            webSocket.sendTXT("ESP32 connected to the game server!");
            break;
        case WStype_TEXT:
            Serial.printf("WebSocket message received: %s\n", payload);
            break;
        case WStype_BIN:
            Serial.println("Binary data received");
            break;
        default:
            break;
    }
}


void sendPlayerIDToNodeRed(const String &playerID) {
    if (webSocket.isConnected()) {
        // String jsonPayload = "{\"playerID\":\"" + playerID + "\"}";  
        String jsonPayload = "{\"playerID\":" + playerID + "}"; // Assumes `playerID` contains a numeric string like "1"
        Serial.print("Sending player ID to Node-RED: ");
        Serial.println(jsonPayload);
        webSocket.sendTXT(jsonPayload);
    } else {
        Serial.println("WebSocket not connected");
    }
}


void setup() {
    // Initialize Serial
    Serial.begin(115200);

    // Initialize IR
    IrSender.begin(IR_LED_PIN);
    irrecv1.enableIRIn();
    irrecv2.enableIRIn();
    irrecv3.enableIRIn();

    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(MOTOR_PIN, OUTPUT);

    attachInterrupt(digitalPinToInterrupt(irReceiverPin1), irISR, FALLING);
    attachInterrupt(digitalPinToInterrupt(irReceiverPin2), irISR, FALLING);
    attachInterrupt(digitalPinToInterrupt(irReceiverPin3), irISR, FALLING);

    // Connect to WiFi
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnected to WiFi");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    // Configure WebSocket connection
    webSocket.begin(ws_server, ws_port, ws_path);
    webSocket.onEvent(webSocketEvent);

    // startGameTimer();  // Start the game timer
}

void loop() {
    // Maintain WebSocket connection
    webSocket.loop();



    if (irSignalDetected && !processingIRSignal) {
        noInterrupts();
        processingIRSignal = true;
        irSignalDetected = false;
        int currentPin = triggeringPin;
        interrupts();

        if (currentPin == irReceiverPin1) {
            processIRSignal(irrecv1, irReceiverPin1);
        } else if (currentPin == irReceiverPin2) {
            processIRSignal(irrecv2, irReceiverPin2);
        } else if (currentPin == irReceiverPin3) {
            processIRSignal(irrecv3, irReceiverPin3);
        }

        noInterrupts();
        processingIRSignal = false;
        interrupts();
    }
  if (digitalRead(BUTTON_PIN) == LOW && !playerOut ) {
    IrSender.sendNEC(0x8000F00F, 32);
    digitalWrite(MOTOR_PIN, HIGH);
    // Serial.println("Fire Is Shout");
    // delay(500);
  }
  else
  digitalWrite(MOTOR_PIN, LOW);
}

void irISR() {
    irSignalDetected = true;
    if (digitalRead(irReceiverPin1) == LOW) {
        triggeringPin = irReceiverPin1;
    } else if (digitalRead(irReceiverPin2) == LOW) {
        triggeringPin = irReceiverPin2;
    } else if (digitalRead(irReceiverPin3) == LOW) {
        triggeringPin = irReceiverPin3;
    }
}
// Function to process IR signals
void processIRSignal(IRrecv &irrecv, int receiverPin) {
    if (irrecv.decode()) {
        Serial.print("Received IR signal from receiver on pin: ");
        Serial.println(receiverPin);
        Serial.print("Hex Code: ");
        Serial.println(irrecv.decodedIRData.decodedRawData, HEX);

        uint32_t receivedCode = irrecv.decodedIRData.decodedRawData;

        // Check if the received code exists in the map
        if (hexCodeToPlayerID.find(receivedCode) != hexCodeToPlayerID.end()) {
            String playerID = hexCodeToPlayerID[receivedCode]; // Get the corresponding Player ID
            Serial.println("Special signal detected! Sending Player ID to Node-RED.");
            sendPlayerIDToNodeRed(playerID); // Send the appropriate Player ID
        } else if (receivedCode == 0xF00F0002) { // Example for another specific condition
            playerScore--;
            Serial.print("Player hit! New total score: ");
            Serial.println(playerScore);
            sendPlayerIDToNodeRed("1"); // Send Player ID 1

            if (playerScore <= 0) {
                playerOut = true;
                Serial.println("Player is out of the game!");
            }
        }

        irrecv.resume();  // Prepare for the next signal
    }
}


void endGame() {
    Serial.println("Game over! No further scores counted.");

    if (webSocket.isConnected()) {
        String jsonPayload = "{\"event\":\"gameEnd\"}";
        webSocket.sendTXT(jsonPayload);
    }
}