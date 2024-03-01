/**
 * Post request example
 * This isn't clean or error resistant, but it is a good demonstration of hosting a webpage off of an SD card and reading a post request
 * From the user. This is an alternative method for controlling the device, rather than using the serial monitor.
 * 
 * @author: Cody Roberson
 * 
 * @date: 2024-02-29
 * 
 * Referenced Material
 * https://www.arduino.cc/reference/en/libraries/ethernet/client.read/
 * https://www.arduino.cc/reference/en/language/functions/communication/stream/
 * https://www.arduino.cc/reference/en/language/functions/communication/stream/streamreadstringuntil/
 * https://github.com/bblanchon/ArduinoJson/blob/7.x/examples/JsonHttpClient/JsonHttpClient.ino
 * https://www.eso.org/~ndelmott/ascii.html
 * https://www.freecodecamp.org/news/javascript-post-request-how-to-send-an-http-post-request-in-js/
 * https://docs.arduino.cc/learn/programming/sd-guide/
 * https://developer.mozilla.org/en-US/docs/Web/API/Response/json
 * 
 * 
*/
#define SERVE_PAGE
#include <Arduino.h>

#include "request_handler.h"

uint8_t mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xEF, 0xED};
IPAddress ip(192, 168, 200, 177);
uint8_t dns[] = {8, 8, 8, 8};
uint8_t gateway[] = {192, 168, 200, 1};
uint8_t subnet[] = {255, 255, 255, 0};
double datten = 0.0;
double s_atten = 0.0;

EthernetServer server = EthernetServer(80);

File myFile;

void setup() {
    Serial.begin(115200);
    while(!Serial){};

    // Chipselect
    pinMode(10, OUTPUT); //Ethernet
    digitalWrite(10, HIGH); //PS - Active Low
    pinMode(9, OUTPUT); //SD
    digitalWrite(9, HIGH); //PS - Active Low
    pinMode(8, OUTPUT); //eth reset
    digitalWrite(8, HIGH); //active low
    Ethernet.begin(mac, ip, gateway, subnet, dns);
    Ethernet.init(10);
    server.begin();
    pinMode(6, INPUT);

    Serial.print("Initializing SD card...");
    if (!SD.begin(9)) {
        Serial.println("initialization failed!");
        while (1);
    }
    Serial.println("initialization done.");
}

String mycallback(http_request_t req) {
    JsonDocument datain;
    JsonDocument dataout;
    String resp = "";
    if (req.path == "/set_atten")
    {
        deserializeJson(datain, req.post_data);
        datten = datain["d_atten"];
        s_atten = datain["s_atten"];

        dataout["d_atten"] = datten;
        dataout["s_atten"] = s_atten;

        serializeJson(dataout, resp);
        return resp;
    } else {
        return "{\"error\": \"invalid path\"}";
    }

}

void loop(){
    EthernetClient client = server.available();
    char c;
    http_request_t req;

    if(client){
        JsonDocument doc;
        Serial.println("client connected");
        req = handle_client_request(client, mycallback);
        Serial.println(req.post_data);
 
        client.stop();
        
    }
}