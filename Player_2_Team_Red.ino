#include <WiFi.h>
#include <HTTPClient.h>
#include <IRremote.h>
#include <WebSocketsClient.h>

const char* ssid = "ES";           
const char* password = "Electro@2024"; 

const char* ws_server = "192.168.18.43";
const uint16_t ws_port = 1880;
const char* ws_path = "/ws";

WebSocketsClient webSocket;

const int IR_LED_PIN = 2;
const int BUTTON_PIN = 4;
const int MOTOR_PIN = 33;
const int irReceiverPin = 19;

IRrecv irrecv(irReceiverPin);
decode_results results;

bool isIRSent = false;           // Tracks whether IR has been sent
unsigned long irStartTime = 0;   // Tracks the start time of IR delay
const unsigned long irDelay = 500; // 500ms delay for IR transmission

int playerScore = 50;
bool playerOut = false;

// Unique send hex code for this player
const uint32_t sendHex = 0x8001F00F; // Replace with this player's unique hex code

// Array of opponent hex codes and their corresponding player IDs
const uint32_t receiveHex[] = {0xF00F0002, 0xF00F8002, 0xF00F4002, 0xF00FC002, 0xF00F2002, 0xF00FA002};
const uint8_t opponentIDs[] = {7, 8, 9, 10, 11, 12};

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

void sendPlayerIDToNodeRed(const uint8_t opponentID) {
    if (webSocket.isConnected()) {
        String jsonPayload = "{\"playerID\":" + String(opponentID) + "}";
                // String jsonPayload = "{\"playerID\":" + playerID + "}"; // Assumes `playerID` contains a numeric string like "1"
        // String jsonPayload = "{\"playerID\":\"" + playerID + "\"}";

        Serial.print("Sending player ID to Node-RED: ");
        Serial.println(jsonPayload);
        webSocket.sendTXT(jsonPayload);
    } else {
        Serial.println("WebSocket not connected");
    }
}

void setup() {
    Serial.begin(115200);

    IrSender.begin(IR_LED_PIN);
    irrecv.enableIRIn();
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(MOTOR_PIN, OUTPUT);

    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnected to WiFi");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    webSocket.begin(ws_server, ws_port, ws_path);
    webSocket.onEvent(webSocketEvent);
}

void loop() {
    webSocket.loop();

    checkIRReceiver();

    if (digitalRead(BUTTON_PIN) == LOW) {
        // Turn the motor on immediately
        digitalWrite(MOTOR_PIN, HIGH);

        // Handle IR transmission
        if (!isIRSent) {
            IrSender.sendNEC(sendHex, 32); // Send this player's unique IR code
            isIRSent = true;              // Mark IR as sent
            irStartTime = millis();       // Record the time IR was sent
        }
    } else {
        // Turn the motor off immediately when button is released
        digitalWrite(MOTOR_PIN, LOW);
        isIRSent = false; // Reset IR sent status for the next button press
    }

    // Ensure 500ms delay before allowing another IR transmission
    if (isIRSent && millis() - irStartTime >= irDelay) {
        isIRSent = false; // Allow IR to be sent again
    }
}

void checkIRReceiver() {
    if (irrecv.decode()) {
        uint32_t receivedCode = irrecv.decodedIRData.decodedRawData;
        for (int i = 0; i < sizeof(receiveHex) / sizeof(receiveHex[0]); i++) {
            if (receivedCode == receiveHex[i]) {
                uint8_t opponentID = opponentIDs[i];
                Serial.print("Hit detected from opponent ID: ");
                Serial.println(opponentID);
                sendPlayerIDToNodeRed(opponentID); // Send the ID to Node-RED
                break;
            }
        }
        irrecv.resume(); // Prepare for the next code
    }
}
