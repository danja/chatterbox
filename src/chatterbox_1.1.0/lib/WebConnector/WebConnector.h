#ifndef WebConnector_h
#define WebConnector_h

#include <WString.h>
#include <EventReceiver.h>

#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include "SPIFFS.h"

class WebConnector : public EventReceiver
{
  static void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);

public:
  WebConnector();
  void registerCallback(Dispatcher<EventType, String, float> &dispatcher);
  void startWebServer();
  void convertAndPush(String id, int value);

private:
  void listener(const EventType &type, const String &name, const float &value);
  static String& pageProcessor(const String &var);
};

#endif