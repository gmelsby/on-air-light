#include <WiFi.h>
#include "WiFiCredentials.h"

#define OFF "off"
#define ON_AIR "on-air"
#define ON_CAMERA "on-camera"

#define ON_AIR_PIN 15
#define ON_CAMERA_PIN 16

// define DEBUG to use Serial connection
#undef DEBUG
#define DEBUG 1

const char* ssid = MYSSID;
const char* password = MYPASSWORD;

WiFiServer server(80);
String request;  // stores text of HTTP request
String state;    // stores state of light
String lightRoute = "/api/light";

void setup() {
  #ifdef DEBUG
  Serial.begin(115200);
  while (!Serial) {
    continue;
  }
  #endif

  pinMode(ON_AIR_PIN, OUTPUT);
  pinMode(ON_CAMERA_PIN, OUTPUT);

  state = "off";

  // print wifi connection
  #ifdef DEBUG
  Serial.printf("\nconnecting to SSID \"%s\"...\n", ssid);
  #endif

    // on_air on until wifi connected, then on_camera blink
  digitalWrite(ON_AIR_PIN, HIGH);

  WiFi.begin(ssid, password);
  //wait until wifi connected
  while (WiFi.status() != WL_CONNECTED) {
    ;
  }

  digitalWrite(ON_AIR_PIN, LOW);
  
  digitalWrite(ON_CAMERA_PIN, HIGH);
  delay(500);
  digitalWrite(ON_CAMERA_PIN, LOW);

  #ifdef DEBUG
  Serial.println("Connected to Wifi");
  Serial.printf("IP Address: %s", WiFi.localIP().toString());
  #endif
  server.begin();
}

void loop() {
  WiFiClient client = server.available();
  if (client) {
    #ifdef DEBUG
    Serial.println("client detected");
    #endif

    while (client.available() != 0) {
      char nextChar = client.read();
      request += nextChar;
    }
    #ifdef DEBUG
    Serial.printf("Read request: \n%s\n", request);
    #endif
    handleHTTP(request, client);
    // reset for next client
    client.stop();
    request = "";
  }
}

void handleHTTP(String& request, WiFiClient& client) {
  #ifdef DEBUG
  Serial.println(request);
  #endif
  // case where we are dealing with GET request
  if (request.indexOf("GET") == 0) {
    #ifdef DEBUG
    Serial.println("Get request detected");
    #endif
    handleGet(request, client);
  }

  // case where we are dealing with POST request
  else if (request.indexOf("POST") == 0) {
    #ifdef DEBUG
    Serial.println("Post request detected");
    #endif
    handlePost(request, client);
  }

  // only provides options for the only operational POST route
  else if (request.indexOf("OPTIONS") == 0 && getRoute(request).equals(lightRoute)) {
    #ifdef DEBUG
    Serial.println("Options request detected");
    #endif
    client.println("HTTP/1.1 200 OK");
    client.println("Access-Control-Allow-Origin: *");
    client.println("Access-Control-Allow-Methods: POST, GET, OPTIONS");
    client.println("Access-Control-Allow-Headers: *");
    client.println("Allow: application/json");
    client.println("Connection: close");
    client.println("");
  }

  // ideally we should be sending a "method not supported" or "resource not found" code
}

void handleGet(String& request, WiFiClient& client) {

  String route = getRoute(request);

  // if we have an API request, send back JSON
  if (route.equals(lightRoute)) {
    sendJsonResponse(client, 200);
    return;
  }

  // otherwise regardless of path send back html
  String responseBody = "<!DOCTYPE html>\n<html><head><title>ESP</title></head><body>";
  responseBody += "<h1>" + state + "</h1>\
  </body></html><button onclick=\"postState('off')\">Turn Off</button>\
  <button onclick=\"postState('on-air')\">On Air</button>\
  <button onclick=\"postState('on-camera')\">On Camera</button></body></html>\
  <script>const postState=(newState)=>{fetch('";
  responseBody += lightRoute;
  responseBody += "',{method:'POST',headers:{'Accept':'application/json',\
  'Content-Type':'application/json'},body:JSON.stringify({state: newState})})\
  .then(()=>window.location.reload()).catch(e=>{console.log(e.message)})}</script>";
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.printf("Content-Length: %i\n", responseBody.length());
  client.println("Connection: close");
  client.println("");
  client.print(responseBody);
}

void handlePost(String& request, WiFiClient& client) {
  String route = getRoute(request);
  
  // only want to allow POST operations to /api/light
  if (!route.equals(lightRoute)) {
    sendNotFoundError(client);
    return;
  }

  // isolate the request body (some assumpitons about valid JSON being made)
  String requestBody = request.substring(request.indexOf("\n{") + 1);
  #ifdef DEBUG
  Serial.println("isolated request body:");
  Serial.println(requestBody);
  #endif

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

  String requestedState = requestBody.substring(openingQuoteIndex + 1, requestBody.indexOf("\"", openingQuoteIndex + 1));
  #ifdef DEBUG
  Serial.printf("state: %s\n", requestedState);
  #endif

  // malformed request if state value is not equal to one of our specified states
  if (!(updateState(requestedState))) {
    sendBadRequestError(client);
    return;
  }

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
    client.println("Access-Control-Allow-Origin: *");
    client.println("\n{\"Error\":\"404 Not Found\"}\n");
}

void sendBadRequestError(WiFiClient& client) {
    client.println("HTTP/1.1 400 Bad Request");
    client.println("Connection: close");
    client.println("Access-Control-Allow-Origin: *");
    client.println("\n{\"Error\":\"400 Bad Request\"}\n");
}

// sends JSON responses for successful GET /api/light and POST /api/light
void sendJsonResponse(WiFiClient& client, int code) {
  String responseBody = "{\"state\": \"" + state + "\"}\n";
  if (code == 201) {
    client.println("HTTP/1.1 201 Created");
  }
  else if (code == 200) {
    client.println("HTTP/1.1 200 OK");
  }
  else {
    #ifdef DEBUG
    Serial.print("Something went wrong! Invalid code.");
    #endif
    return;
  }
  client.println("Content-Type: application/json");
  client.printf("Content-Length: %i\n", responseBody.length());
  // allow fetch requests in browser
  client.println("Access-Control-Allow-Origin: *");
  client.println("Connection: close");
  client.println("");
  client.println(responseBody);
}

// turns on and off appropriate lights, returns false if state is not a valid value
bool updateState(String requestedState) {
  bool isStateValid = false;
  if (requestedState.equals(OFF)) {
    digitalWrite(ON_AIR_PIN, LOW);
    digitalWrite(ON_CAMERA_PIN, LOW);
    isStateValid = true;
  } else if (requestedState.equals(ON_AIR)) {
    digitalWrite(ON_AIR_PIN, HIGH);
    digitalWrite(ON_CAMERA_PIN, LOW);
    isStateValid = true;
  } else if (requestedState.equals(ON_CAMERA)) {
    digitalWrite(ON_CAMERA_PIN, HIGH);
    digitalWrite(ON_AIR_PIN, LOW);
    isStateValid = true;
  }
  if (isStateValid) {
    state = requestedState;
    #ifdef DEBUG
    Serial.println("state updated");
    #endif
  }
  return isStateValid;
}





