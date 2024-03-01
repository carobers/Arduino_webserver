#include <Ethernet.h>
#include <SD.h>
#include <ArduinoJson.h>


typedef struct http_request_t {
    String method;
    String path;
    String protocol;
    String post_data;
} http_request_t;

int serve_webpage_from_sdcard(EthernetClient client, String filename) {
    File webpage = SD.open(filename);
    if (!webpage) {
        Serial.println("File not found");
        client.println("HTTP/1.1 404");
        client.println();
        return 0;
    }
    char c;
    client.println("HTTP/1.1 200 OK\rContent-Type: text/html\rConnection: close");
    client.println();
    if (webpage) {
        while (webpage.available()){
            c = webpage.read();
            client.print(c);
        }
    }
    webpage.close();
    return 1;
}


int serve_page(http_request_t req, EthernetClient client) {
    // Serial.println("serve_page() called");
    // Serial.println(req.path);
    if(req.path == "/") {
        Serial.println("Serving index.htm");
        return serve_webpage_from_sdcard(client, "index.htm");
    } else if(req.path != "") {
        Serial.println("Serving " + req.path);
        String path = req.path;
        path.remove(0, 1); // remove leading '/'
        return serve_webpage_from_sdcard(client, req.path);
    } else {
        return 0;
    }
}



http_request_t handle_client_request(EthernetClient client, String (*post_callback)(http_request_t)) {
    http_request_t req;

    String request_firstline = client.readStringUntil('\n');

    if(request_firstline.indexOf("GET") != -1) {
        request_firstline.replace("GET ", "");
        request_firstline.replace(" HTTP/1.1", "");
        req.method = "GET";
        request_firstline.trim();
        req.path = request_firstline;
        req.protocol = "HTTP/1.1";

        while (client.available())
            client.read();
        
        serve_page(req, client);

    } else if(request_firstline.indexOf("POST") != -1) {
        request_firstline.replace("POST ", "");
        request_firstline.replace(" HTTP/1.1", "");
        req.method = "POST";
        request_firstline.trim();
        req.path = request_firstline;
        req.protocol = "HTTP/1.1";

        if (client.find("\r\n\r\n")) {
            String post_data = client.readString();
            req.post_data = post_data;
            
            client.println("HTTP/1.1 200 OK");
            client.println();
            if (post_callback != NULL) {
                client.print(post_callback(req));
            }
            
        }
    } else {
        Serial.println("Unknown request");
    }
    return req;
}

