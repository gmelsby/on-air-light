#include <WiFi.h>
#include "WiFiCredentials.h"

char* ssid = MYSSID;
char* password = MYPASSWORD;

WiFiServer server(80);
String request;  // stores text of HTTP request
String state;    // stores state of light

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    continue;
  }

  state = "off";

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

    while (client.available() != 0) {
      char nextChar = client.read();
      request += nextChar;
    }
    handleHTTP(request, client);
    // reset for next client
    client.stop();
    request = "";
  }
}

void handleHTTP(String& request, WiFiClient& client) {
  Serial.println(request);
  // case where we are dealing with GET request
  if (request.indexOf("GET") == 0) {
    Serial.println("Get request detected");
    handleGet(request, client);
  }

  // case where we are dealing with POST request
  else if (request.indexOf("POST") == 0) {
    Serial.println("Post request detected");
    handlePost(request, client);
  }

  // ideally we should be sending a "method not supported" or "resource not found" code
}

void handleGet(String& request, WiFiClient& client) {

  Serial.printf("Route: %s", getRoute(request));

  String responseBody = "<!DOCTYPE html>\n<html><head><title>ESP</title></head><body><h1>Hello world</h1>";
  responseBody += "<p>state: " + state + "</p>";
  responseBody += "</body></html>";
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.printf("Content-Length: %i\n", responseBody.length());
  client.println("Connection: Closed");
  client.println("");
  client.print(responseBody);
}

void handlePost(String& request, WiFiClient& client) {
  Serial.printf("Route: %s\n", getRoute(request));

  int requestBodyStart = request.indexOf("\n{") + 1;
  Serial.println("isolated request body:");
  Serial.println(request.substring(requestBodyStart));
  String responseBody = "{\"value1\": \"hello\", \"value2\": \"world\"}";
  client.println("HTTP/1.1 201 Created");
  client.println("Content-Type: application/json");
  client.printf("Content-Length: %i\n", responseBody.length());
  client.println("Connection: Closed");
  client.println("");
  client.print(responseBody);
}

// returns the route from a properly-formatted http request
String getRoute(String& request) {
  int rootIndex = request.indexOf("/");
  return request.substring(rootIndex, request.indexOf(" ", rootIndex));
} 





