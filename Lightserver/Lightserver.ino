#include <WiFi.h>
#include "WiFiCredentials.h"

char* ssid = MYSSID;
char* password = MYPASSWORD;

int status = WL_IDLE_STATUS;

WiFiServer server(80);
String requestHeader;
String requestBody;

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ;
  }

  Serial.printf("\nconnecting to SSID \"%s\"...\n", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    ;
  }
  Serial.println("Connected to Wifi");
  Serial.printf("IP Address: %s", WiFi.localIP().toString());
  server.begin();
}

void loop() {
  WiFiClient client = server.available();
  if (client) {
    Serial.println("client detected");
  }
}