*****************************************************************
*                        VinoJV                                 *
*****************************************************************
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define MQTT_VERSION MQTT_VERSION_3_1_1

// Wifi: SSID and password
const char* WIFI_SSID = "Your SSID";
const char* WIFI_PASSWORD = "Your Password";

// MQTT: ID, server IP, port, username and password
const PROGMEM char* MQTT_CLIENT_ID = "office_light1";
const PROGMEM char* MQTT_SERVER_IP = "192.168.1.102"; //IP Address of your MQTT server
const PROGMEM uint16_t MQTT_SERVER_PORT = 1883; //Default
const PROGMEM char* MQTT_USER = ""; //MQTT Username (use this if your server runs encrypted service)
const PROGMEM char* MQTT_PASSWORD = ""; //MQTT password (use this if your server runs encrypted service)

// MQTT: topics
const char* MQTT_LIGHT_STATE_TOPIC = "office/light1/status";
const char* MQTT_LIGHT_COMMAND_TOPIC = "office/light1/switch";

// payloads by default (on/off)
const char* LIGHT_ON = "ON";
const char* LIGHT_OFF = "OFF";

const PROGMEM uint8_t LED_PIN = 14; // GPIO 14 of Nodemcu
boolean m_light_state = false; // light is turned off by default

WiFiClient wifiClient;
PubSubClient client(wifiClient);

// function called to publish the state of the light (on/off)
void publishLightState() {
  if (m_light_state) {
    client.publish(MQTT_LIGHT_STATE_TOPIC, LIGHT_ON, true);
  } else {
    client.publish(MQTT_LIGHT_STATE_TOPIC, LIGHT_OFF, true);
  }
}

// function called to turn on/off the light
void setLightState() {
  if (m_light_state) {
    digitalWrite(LED_PIN, HIGH);
    Serial.println("INFO: Turn light on...");
  } else {
    digitalWrite(LED_PIN, LOW);
    Serial.println("INFO: Turn light off...");
  }
}

// function called when a MQTT message arrived
void callback(char* p_topic, byte* p_payload, unsigned int p_length) {
  // concat the payload into a string
  String payload;
  for (uint8_t i = 0; i < p_length; i++) {
    payload.concat((char)p_payload[i]);
  }
  
  // handle message topic
  if (String(MQTT_LIGHT_COMMAND_TOPIC).equals(p_topic)) {
    // test if the payload is equal to "ON" or "OFF"
    if (payload.equals(String(LIGHT_ON))) {
      if (m_light_state != true) {
        m_light_state = true;
        setLightState();
        publishLightState();
      }
    } else if (payload.equals(String(LIGHT_OFF))) {
      if (m_light_state != false) {
        m_light_state = false;
        setLightState();
        publishLightState();
      }
    }
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.println("INFO: Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASSWORD)) {
      Serial.println("INFO: connected");
      // Once connected, publish an announcement...
      publishLightState();
      // ... and resubscribe
      client.subscribe(MQTT_LIGHT_COMMAND_TOPIC);
    } else {
      Serial.print("ERROR: failed, rc=");
      Serial.print(client.state());
      Serial.println("DEBUG: try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  // init the serial
  Serial.begin(115200);

  // init the led
  pinMode(LED_PIN, OUTPUT);
  analogWriteRange(255);
  setLightState();

  // init the WiFi connection
  Serial.println();
  Serial.println();
  Serial.print("INFO: Connecting to ");
  WiFi.mode(WIFI_STA);
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("INFO: WiFi connected");
  Serial.print("INFO: IP address: ");
  Serial.println(WiFi.localIP());

  // init the MQTT connection
  client.setServer(MQTT_SERVER_IP, MQTT_SERVER_PORT);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
