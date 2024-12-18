#include <WiFi.h>
#include <IRremote.h>
#include <WebSocketsClient.h>


const char* ssid = "ES";
const char* password = "Electro@2024";

const char* ws_server = "192.168.18.43";
const uint16_t ws_port = 1880;
const char* ws_path = "/ws";

WebSocketsClient webSocket;

// IR configuration
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

// Game state
bool gameStarted = false;
unsigned long gameTimer = 800000;  // 10 minutes in milliseconds
unsigned long gameStartTime = 0;

// WebSocket event handler
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

void sendPlayerIDToNodeRed() {
    if (webSocket.isConnected()) {
        String jsonPayload = "{\"playerID\":\"" + playerID + "\"}";
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

    startGameTimer();  // Start the game timer
}

void loop() {
    // Maintain WebSocket connection
    webSocket.loop();

    if (gameStarted && millis() - gameStartTime >= gameTimer) {
        endGame();
    }

    if (irSignalDetected && !processingIRSignal && gameStarted) {
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

    if (digitalRead(BUTTON_PIN) == LOW && !playerOut && gameStarted) {
        IrSender.sendNEC(0x8000F00F, 32);
        Serial.println("Fired");
        delay(500);
    }
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

void processIRSignal(IRrecv &irrecv, int receiverPin) {
    if (irrecv.decode()) {
        Serial.print("Received IR signal from receiver on pin: ");
        Serial.println(receiverPin);
        Serial.print("Hex Code: ");
        Serial.println(irrecv.decodedIRData.decodedRawData, HEX);

        if (irrecv.decodedIRData.decodedRawData == 0xF00F0001 && !playerOut) {
            playerScore--;
            Serial.print("Player hit! New total score: ");
            Serial.println(playerScore);

            sendPlayerIDToNodeRed();

            if (playerScore <= 0) {
                playerOut = true;
                Serial.println("Player is out of the game!");
            }
        }

        irrecv.resume();  // Prepare for the next signal
    }
}

void startGameTimer() {
    gameStartTime = millis();
    gameStarted = true;
    Serial.println("Game started! Timer set for 10 minutes.");

    if (webSocket.isConnected()) {
        String jsonPayload = "{\"event\":\"gameStart\", \"timer\":\"10:00\"}";
        webSocket.sendTXT(jsonPayload);
    }
}

void endGame() {
    gameStarted = false;
    Serial.println("Game over! No further scores counted.");

    if (webSocket.isConnected()) {
        String jsonPayload = "{\"event\":\"gameEnd\"}";
        webSocket.sendTXT(jsonPayload);
    }
}
