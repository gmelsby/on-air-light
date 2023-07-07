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
  client.println("Connection: close");
  client.println("");
  client.print(responseBody);
}

void handlePost(String& request, WiFiClient& client) {
  String route = getRoute(request);
  
  // only want to allow POST operations to /light
  if (route != "/light") {
    sendNotFoundError(client);
    return;
  }

  // isolate the request body (some assumpitons about valid JSON being made)
  String requestBody = request.substring(request.indexOf("\n{") + 1);
  Serial.println("isolated request body:");
  Serial.println(requestBody);

  // we only care about the attribute "state"
  int stateIndex = requestBody.indexOf("\"state\":");
  // if it's not here we just exit out
  if (stateIndex == -1) {
    sendBadRequestError(client);
    return;
  }

  // if there's no follow up " after the : after "state" request is malformed
  int openingQuoteIndex = requestBody.indexOf("\"", stateIndex + 8);
  if (openingQuoteIndex > stateIndex + 9) {
    sendBadRequestError(client);
    return;
  }

  String stateValue = requestBody.substring(openingQuoteIndex + 1, requestBody.indexOf("\"", openingQuoteIndex + 1));
  Serial.printf("state: %s", stateValue);

  // malformed request if state value is not equal to one of our specified states
  if (!(stateValue.equals("off") || stateValue.equals("on-air") || stateValue.equals("on-camera"))) {
    sendBadRequestError(client);
    return;
  }

  // since we have checked the state value we can assign it to state
  state = stateValue;
  
  sendJsonResponse(client, 201);
}

// returns the route from a properly-formatted http request
String getRoute(String& request) {
  int rootIndex = request.indexOf("/");
  return request.substring(rootIndex, request.indexOf(" ", rootIndex));
}

void sendNotFoundError(WiFiClient& client) {
    client.println("HTTP/1.1 404 Not Found");
    client.println("Connection: close");
    client.println("\n{\"Error\":\"404 Not Found\"}\n");
}

void sendBadRequestError(WiFiClient& client) {
    client.println("HTTP/1.1 400 Bad Request");
    client.println("Connection: close");
    client.println("\n{\"Error\":\"400 Bad Request\"}");
}

// sends JSON responses for successful GET /light and POST /light
void sendJsonResponse(WiFiClient& client, int code) {
  String responseBody = "{\"state\": \"" + state + "\"}\n";
  if (code == 201) {
    client.println("HTTP/1.1 201 Created");
  }
  else if (code == 200) {
    client.println("HTTP/1.1 200 OK");
  }
  else {
    Serial.print("Something went wrong! Invalid code.");
    return;
  }
  client.println("Content-Type: application/json");
  client.printf("Content-Length: %i\n", responseBody.length());
  client.println("Connection: close");
  client.println("");
  client.println(responseBody);
}





