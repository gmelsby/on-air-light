# IP-enabled On-Air Light
Microcontroller project to control LEDs from a webserver on ESP32-S2.

Inspired by [Hipster Brown](https://github.com/HipsterBrown)'s [on-air-light](https://github.com/HipsterBrown/on-air-light) project--seemed like a great way to get into microcontrollers!
I initially was going to use xs-dev but [my board (ESP32-S2)](https://learn.adafruit.com/adafruit-esp32-s2-feather) seemed to have issues with Moddable setup.

Right now the following endpoints work:
- `GET /` returns a barebones webpage with light state and buttons for changing state.
- `GET /api/light` returns JSON indicating the current state of the light.
- `POST /api/light` takes a JSON body and updates the state of the light.

The webserver conforms to the HTTP specification enough for well-behaved requests to work as intended. A common design pattern this project avoids is endpoints like `GET /togglelight` that are easier to implement but present issues with browser caching and preloading--no non-idempotent GET requests here!

The client folder contains a slightly more complex vanilla JS web app than the one served from the microcontroller itself. My intention is to polish it up and use it as the main way to interface with the controller, either served from the controller or from a different machine. In the event that it is served from a different machine, the appropriate headers for CORS are configured on the server.

To run it locally, you cannot just open `index.html` from the filesystem in the browser. Instead, it needs to be served from a web server. This can be easily accomlished if python is installed--just run `python3 -m http.server` at the client root.