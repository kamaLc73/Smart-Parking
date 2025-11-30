// Smart Parking System with ESP32 - MQTT Fixed
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

// Parking Place Pins (same as before)
#define TRIG1 5
#define ECHO1 18
#define LED1_GREEN 19
#define LED1_RED 4

#define TRIG2 21
#define ECHO2 22
#define LED2_GREEN 23
#define LED2_RED 2

#define TRIG3 32
#define ECHO3 33
#define LED3_GREEN 25
#define LED3_RED 15

#define TRIG4 26
#define ECHO4 27
#define LED4_GREEN 14
#define LED4_RED 12

#define THRESHOLD 50
#define TIMEOUT 30000
#define NUM_PLACES 4

// ==== CONFIGURATION WiFi ====
const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASSWORD = "";

// ==== CONFIGURATION MQTT HiveMQ Cloud ====
const char* MQTT_BROKER = "43b843e091064d22bd63b1edb91a947a.s1.eu.hivemq.cloud";
const int MQTT_PORT = 8883;

// Credentials MQTT HiveMQ Cloud
const char* MQTT_USERNAME = "smart_parking";
const char* MQTT_PASSWORD = "0123456789Aze_";

const char* MQTT_TOPIC = "smartparking/status";
const char* MQTT_TOPIC_SUBSCRIBE = "smartparking/control";

// Client ID unique basé sur MAC address
String clientId = "ESP32-";

WiFiClientSecure espClient;
PubSubClient mqttClient(espClient);

unsigned long lastPublish = 0;
const long publishInterval = 2000;

// Forward declarations
float getDistance(int trigPin, int echoPin);
void updateLEDs(int greenPin, int redPin, float distance, int placeNum);
void displayStatus(int placeNum, float distance);
void testAllLEDs();
void connectToWiFi();
void connectToMQTT();
void callback(char* topic, byte* payload, unsigned int length);

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  // Generate unique client ID from MAC
  uint8_t mac[6];
  WiFi.macAddress(mac);
  clientId += String(mac[0], HEX) + String(mac[1], HEX) + 
              String(mac[2], HEX) + String(mac[3], HEX);
  
  Serial.println("\n╔════════════════════════════════════╗");
  Serial.println("║   SMART PARKING SYSTEM v2.1        ║");
  Serial.println("╠════════════════════════════════════╣");
  Serial.print("║ Client ID: ");
  Serial.print(clientId);
  for(int i = clientId.length(); i < 20; i++) Serial.print(" ");
  Serial.println("║");
  Serial.print("║ Broker: ");
  Serial.print(MQTT_BROKER);
  Serial.println(" ║");
  Serial.println("╚════════════════════════════════════╝\n");
  
  // Setup pins
  pinMode(TRIG1, OUTPUT); pinMode(ECHO1, INPUT);
  pinMode(LED1_GREEN, OUTPUT); pinMode(LED1_RED, OUTPUT);
  pinMode(TRIG2, OUTPUT); pinMode(ECHO2, INPUT);
  pinMode(LED2_GREEN, OUTPUT); pinMode(LED2_RED, OUTPUT);
  pinMode(TRIG3, OUTPUT); pinMode(ECHO3, INPUT);
  pinMode(LED3_GREEN, OUTPUT); pinMode(LED3_RED, OUTPUT);
  pinMode(TRIG4, OUTPUT); pinMode(ECHO4, INPUT);
  pinMode(LED4_GREEN, OUTPUT); pinMode(LED4_RED, OUTPUT);
  
  // Initialize pins
  digitalWrite(TRIG1, LOW); digitalWrite(LED1_GREEN, LOW); digitalWrite(LED1_RED, LOW);
  digitalWrite(TRIG2, LOW); digitalWrite(LED2_GREEN, LOW); digitalWrite(LED2_RED, LOW);
  digitalWrite(TRIG3, LOW); digitalWrite(LED3_GREEN, LOW); digitalWrite(LED3_RED, LOW);
  digitalWrite(TRIG4, LOW); digitalWrite(LED4_GREEN, LOW); digitalWrite(LED4_RED, LOW);
  
  testAllLEDs();
  
  // Configure secure client for HiveMQ Cloud
  espClient.setInsecure(); // For development - use setCACert() in production
  
  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
  mqttClient.setCallback(callback);
  mqttClient.setBufferSize(512); // Increase buffer for JSON
  
  connectToWiFi();
  connectToMQTT();
}

void loop() {
  // Reconnect if needed
  if (!mqttClient.connected()) {
    connectToMQTT();
  }
  mqttClient.loop();
  
  // Read sensors
  float distance1 = getDistance(TRIG1, ECHO1);
  delay(100);
  float distance2 = getDistance(TRIG2, ECHO2);
  delay(100);
  float distance3 = getDistance(TRIG3, ECHO3);
  delay(100);
  float distance4 = getDistance(TRIG4, ECHO4);
  
  // Update LEDs
  updateLEDs(LED1_GREEN, LED1_RED, distance1, 1);
  updateLEDs(LED2_GREEN, LED2_RED, distance2, 2);
  updateLEDs(LED3_GREEN, LED3_RED, distance3, 3);
  updateLEDs(LED4_GREEN, LED4_RED, distance4, 4);
  
  // Calculate statistics
  int freePlaces = 0;
  int occupiedPlaces = 0;
  
  if (distance1 >= THRESHOLD) freePlaces++; else occupiedPlaces++;
  if (distance2 >= THRESHOLD) freePlaces++; else occupiedPlaces++;
  if (distance3 >= THRESHOLD) freePlaces++; else occupiedPlaces++;
  if (distance4 >= THRESHOLD) freePlaces++; else occupiedPlaces++;
  
  // Display on serial
  Serial.println("\n╔════════════════════════════════════╗");
  Serial.println("║      PARKING STATUS REPORT         ║");
  Serial.println("╠════════════════════════════════════╣");
  displayStatus(1, distance1);
  displayStatus(2, distance2);
  displayStatus(3, distance3);
  displayStatus(4, distance4);
  Serial.println("╠════════════════════════════════════╣");
  Serial.print("║ FREE: ");
  Serial.print(freePlaces);
  Serial.print(" | OCCUPIED: ");
  Serial.print(occupiedPlaces);
  Serial.print(" | TOTAL: ");
  Serial.print(NUM_PLACES);
  Serial.println("   ║");
  float availability = (freePlaces * 100.0) / NUM_PLACES;
  Serial.print("║ Availability: ");
  Serial.print(availability, 0);
  Serial.println("%                  ║");
  Serial.println("╚════════════════════════════════════╝");
  
  // Publish to MQTT (throttled)
  unsigned long now = millis();
  if (now - lastPublish >= publishInterval) {
    lastPublish = now;
    
    if (mqttClient.connected()) {
      // Build JSON payload
      String payload = "{";
      payload += "\"device\":\"" + clientId + "\",";
      payload += "\"free\":" + String(freePlaces) + ",";
      payload += "\"occupied\":" + String(occupiedPlaces) + ",";
      payload += "\"availability\":" + String((int)availability) + ",";
      payload += "\"places\":[";
      
      float distances[NUM_PLACES] = {distance1, distance2, distance3, distance4};
      for (int i = 0; i < NUM_PLACES; i++) {
        bool occupied = (distances[i] < THRESHOLD && distances[i] > 0);
        payload += "{\"place\":" + String(i + 1) + ",";
        payload += "\"distance\":" + String(distances[i], 1) + ",";
        payload += "\"occupied\":" + String(occupied ? "true" : "false") + "}";
        if (i < NUM_PLACES - 1) payload += ",";
      }
      payload += "]}";
      
      Serial.print("\n📤 Publishing to: ");
      Serial.println(MQTT_TOPIC);
      Serial.print("   Payload: ");
      Serial.println(payload);
      
      bool success = mqttClient.publish(MQTT_TOPIC, payload.c_str(), true);
      if (success) {
        Serial.println("   ✅ Published successfully!");
      } else {
        Serial.println("   ❌ Publish failed!");
      }
    } else {
      Serial.println("\n⚠️  Not connected to MQTT - skipping publish");
    }
  }
  
  delay(500);
}

void connectToWiFi() {
  Serial.print("\n🌐 Connecting to WiFi: ");
  Serial.println(WIFI_SSID);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ WiFi Connected!");
    Serial.print("   IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.print("   Gateway: ");
    Serial.println(WiFi.gatewayIP());
    Serial.print("   DNS: ");
    Serial.println(WiFi.dnsIP());
  } else {
    Serial.println("\n❌ WiFi Connection Failed!");
    Serial.println("   Restarting in 5 seconds...");
    delay(5000);
    ESP.restart();
  }
}

void connectToMQTT() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("❌ WiFi not connected - cannot connect to MQTT");
    connectToWiFi();
    return;
  }
  
  int attempts = 0;
  while (!mqttClient.connected() && attempts < 5) {
    Serial.print("\n🔌 Attempting MQTT connection to ");
    Serial.print(MQTT_BROKER);
    Serial.print(":");
    Serial.print(MQTT_PORT);
    Serial.println("...");
    Serial.print("   Client ID: ");
    Serial.println(clientId);
    Serial.print("   Username: ");
    Serial.println(MQTT_USERNAME);
    
    // Attempt connection
    if (mqttClient.connect(clientId.c_str(), MQTT_USERNAME, MQTT_PASSWORD)) {
      Serial.println("✅ MQTT Connected!");
      
      // Subscribe to control topic
      if (mqttClient.subscribe(MQTT_TOPIC_SUBSCRIBE)) {
        Serial.print("   Subscribed to: ");
        Serial.println(MQTT_TOPIC_SUBSCRIBE);
      }
    } else {
      Serial.print("❌ MQTT Connection Failed! State: ");
      int state = mqttClient.state();
      Serial.println(state);
      
      // Decode error state
      switch(state) {
        case -4: Serial.println("   → MQTT_CONNECTION_TIMEOUT"); break;
        case -3: Serial.println("   → MQTT_CONNECTION_LOST"); break;
        case -2: Serial.println("   → MQTT_CONNECT_FAILED"); break;
        case -1: Serial.println("   → MQTT_DISCONNECTED"); break;
        case 1: Serial.println("   → MQTT_CONNECT_BAD_PROTOCOL"); break;
        case 2: Serial.println("   → MQTT_CONNECT_BAD_CLIENT_ID"); break;
        case 3: Serial.println("   → MQTT_CONNECT_UNAVAILABLE"); break;
        case 4: Serial.println("   → MQTT_CONNECT_BAD_CREDENTIALS"); break;
        case 5: Serial.println("   → MQTT_CONNECT_UNAUTHORIZED"); 
                Serial.println("   ⚠️  VÉRIFIEZ VOS IDENTIFIANTS MQTT!"); 
                break;
      }
      
      attempts++;
      if (attempts < 5) {
        Serial.println("   Retrying in 3 seconds...");
        delay(3000);
      }
    }
  }
  
  if (!mqttClient.connected()) {
    Serial.println("\n⚠️  Could not connect to MQTT broker");
    Serial.println("   Continuing without MQTT...");
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("\n📨 Message received [");
  Serial.print(topic);
  Serial.print("]: ");
  
  String message = "";
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println(message);
}

float getDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  long duration = pulseIn(echoPin, HIGH, TIMEOUT);
  
  if (duration == 0) return 999.0;
  
  float distance = duration * 0.01715;
  return distance;
}

void updateLEDs(int greenPin, int redPin, float distance, int placeNum) {
  if (distance < THRESHOLD && distance > 0) {
    digitalWrite(redPin, HIGH);
    digitalWrite(greenPin, LOW);
  } else {
    digitalWrite(greenPin, HIGH);
    digitalWrite(redPin, LOW);
  }
}

void displayStatus(int placeNum, float distance) {
  Serial.print("║ Place ");
  Serial.print(placeNum);
  Serial.print(": ");
  
  if (distance < THRESHOLD && distance > 0) {
    Serial.print("[🔴 OCCUPIED]");
  } else {
    Serial.print("[🟢 FREE    ]");
  }
  
  Serial.print(" - ");
  if (distance >= 999) {
    Serial.print("No Object   ");
  } else {
    if (distance < 10) Serial.print(" ");
    if (distance < 100) Serial.print(" ");
    Serial.print(distance, 1);
    Serial.print(" cm");
  }
  Serial.println("  ║");
}

void testAllLEDs() {
  Serial.println("╔════════════════════════════════════╗");
  Serial.println("║         LED SYSTEM TEST            ║");
  Serial.println("╚════════════════════════════════════╝\n");
  
  Serial.println("✓ Testing GREEN LEDs");
  digitalWrite(LED1_GREEN, HIGH); digitalWrite(LED2_GREEN, HIGH);
  digitalWrite(LED3_GREEN, HIGH); digitalWrite(LED4_GREEN, HIGH);
  delay(1000);
  
  Serial.println("✓ Testing RED LEDs");
  digitalWrite(LED1_GREEN, LOW); digitalWrite(LED2_GREEN, LOW);
  digitalWrite(LED3_GREEN, LOW); digitalWrite(LED4_GREEN, LOW);
  digitalWrite(LED1_RED, HIGH); digitalWrite(LED2_RED, HIGH);
  digitalWrite(LED3_RED, HIGH); digitalWrite(LED4_RED, HIGH);
  delay(1000);
  
  Serial.println("✓ Test Complete\n");
  digitalWrite(LED1_RED, LOW); digitalWrite(LED2_RED, LOW);
  digitalWrite(LED3_RED, LOW); digitalWrite(LED4_RED, LOW);
  delay(500);
}