#pragma once

#include <WString.h>
#include <Receiver.h>

#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include "SPIFFS.h"

 // 


class WebConnector: public Receiver
{
  static void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);

public:
    WebConnector();
    void registerCallback(Dispatcher<EventType, String, float> &dispatcher);
    void startWebServer();
    void convertAndPush(String id, int value);

private:
    void listener(EventType type, String name, float value);
    // String pageProcessor(const String &var);
    // AsyncWebServer server;
   // AsyncWebSocket ws;
   static String pageProcessor(const String &var);
    // AsyncWebServer server(http_port);
    // AsyncWebSocket ws("/ws");
};