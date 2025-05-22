#include <WiFi.h>
#include <NetworkClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <vector>
#include "pitches.h"

#define RED_BUTTON_PIN 21
#define BLUE_BUTTON_PIN 22
#define WHITE_BUTTON_PIN 23
#define BUZZZER_PIN 18
#define LED_PIN 5

const char *ssid = "RA";
const char *password = "55555333";

WebServer server(80);

const int MELODIES = 1;

std::vector<int> melody[] = { { NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4 } };

std::vector<int> noteDurations[] = { { 4, 8, 8, 4, 4, 4, 4, 4 } };

bool server_on = true;
int last_music=0;
String playMusic(int idx) {
  if (idx >= MELODIES) return "Error in music index!";
  last_music=idx;
  for (int thisNote = 0; thisNote < 8; thisNote++) {
    int noteDuration = 1000 / noteDurations[idx][thisNote];
    tone(BUZZZER_PIN, melody[idx][thisNote], noteDuration);

    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    noTone(BUZZZER_PIN);
    if (digitalRead(WHITE_BUTTON_PIN)) {
      return "Music stopped!";
    }
  }
  return "Music played!";
}

void handleRoot() {
  server.send(200, "text/plain", "hello from esp32!");
}
void handleURLForMusic(int a) {
  server.send(200, "text/plain", "Playing music!");
  playMusic(a);
}
void handleNotFound() {
  server.send(404, "text/plain", "Only domain is /music?music=idx");
}
void setupServer() {
  server.on("/", handleRoot);

  server.on("/music", []() {
    String message;
    for (uint8_t i = 0; i < server.args(); ++i) {
      if (server.argName(i) == "music"||server.argName(i) == "Music") {
        message = playMusic(server.arg(i).toInt());
        break;
      }
    }
    server.send(200, "text/plain", message);
  });

  server.onNotFound(handleNotFound);

  server.begin();
}
void setup(void) {
  pinMode(BLUE_BUTTON_PIN, INPUT);
  pinMode(WHITE_BUTTON_PIN, INPUT);
  pinMode(RED_BUTTON_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);

  digitalWrite(LED_PIN, 0);
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp32")) {
    Serial.println("MDNS responder started");
  }
  digitalWrite(LED_PIN, 1);
  setupServer();
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
  if (digitalRead(RED_BUTTON_PIN)) {
    Serial.print("RED\n");
    server_on = !server_on;
    if (server_on) {
      digitalWrite(LED_PIN, 1);
      server.begin();
      setupServer();
    } else {
      digitalWrite(LED_PIN, 0);
      server.stop();
    }
    delay(1000);
  }
  if (digitalRead(BLUE_BUTTON_PIN) and server_on) {
    playMusic(last_music);
    delay(10);
  }
  delay(10);  //allow the cpu to switch to other tasks
}
