
#include <WString.h>
#include <WebConnector.h>

const int http_port = 80;

/**** Local Network specific - TODO must pull out ****/
const char *ssid = "TP-LINK_AC17DC"; // HIDE ME!
const char *password = "33088297";   // HIDE ME!

IPAddress localIP(192, 168, 0, 142);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);   //optional
IPAddress secondaryDNS(8, 8, 4, 4); //optional

AsyncWebServer server(http_port); // 
    AsyncWebSocket ws("/ws");

WebConnector::WebConnector(){}

/* WEB SERVER THREAD */

void WebConnector::startWebServer()
{
  if (!SPIFFS.begin(true))
  {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  // Configures static IP address
  if (!WiFi.config(localIP, gateway, subnet, primaryDNS, secondaryDNS))
  {
    Serial.println("STA Failed to configure");
  }

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  Serial.print("Connected to : ");
  Serial.println(WiFi.localIP());

  ws.onEvent(onWsEvent);
  server.addHandler(&ws);

  server.begin();

/*
  server.on("/index.html", HTTP_GET, [](AsyncWebServerRequest *request) {
    // request->send(200, "text/plain", "Hello from Chatterbox!");
    request->send(SPIFFS, "/index.html", String(), false, WebConnector::pageProcessor);
  });
*/

  server.on("/index.html", HTTP_GET, [](AsyncWebServerRequest *request) {
    // request->send(200, "text/plain", "Hello from Chatterbox!");
    request->send(SPIFFS, "/index.html", String(), false);
  });

  server.on("/ws.html", HTTP_GET, [](AsyncWebServerRequest *request) {
    // request->send(200, "text/plain", "Hello from Chatterbox!");
    request->send(SPIFFS, "/ws.html", String(), false);
  });

  server.on("/chatterbox.html", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/chatterbox.html", String(), false);
  });
  server.on("/chatterbox.css", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/chatterbox.css", String(), false);
  });

  server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/favicon.ico", String(), false);
  });
}
/* END WEB SERVER */

/*
String WebConnector::pageProcessor(const String &var)
{
  char snum[5];

  if (var == "pitch")
  {
    itoa((int)pitch, snum, 10);
    return snum;
  }
  if (var == "larynx")
  {
    itoa(larynx, snum, 10);
    return snum;
  }
  if (var == "f1f")
  {
    itoa(f1f, snum, 10);
    return snum;
  }
  if (var == "f2f")
  {
    itoa(f2f, snum, 10);
    return snum;
  }
  if (var == "f3f")
  {
    itoa(f3f, snum, 10);
    return snum;
  }
  if (var == "f3q")
  {
    itoa(f3q, snum, 10);
    return snum;
  }

  for (int i = 0; i < N_SWITCHES; i++)
  {
    if (var.compareTo(switches[i].id()) == 0)
    {
      if (switches[i].on())
      {
        return "on";
      }
      else
      {
        return "off";
      }
    }
  }
  return String();
}
*/
  /*
      if (var == "Q") return "QWERT";
      if (var == "RR") return F("RRRRR");
      int x = 1024;
      char snum[5];
      itoa(x, snum, 10);
      if (var == "X") return snum;
      float f = 1.123456789;
      char c[50]; //size of the number
      sprintf(c, "%g", f);
      if (var == "Y") return c;
      */

///// WebSocket bits

void WebConnector::convertAndPush(String id, int value)
{
  char snum[5];
  itoa(value, snum, 10);
  String message = id + ":" + snum;
  // Serial.println(message);
  ws.textAll(message);
  //   ws.textAll((char*)text);
}

void WebConnector::onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{

  if (type == WS_EVT_CONNECT)
  {
    Serial.println("Websocket client connection received");
    client->text("Hello from ESP32 Server");
  }
  else if (type == WS_EVT_DISCONNECT)
  {
    Serial.println("WS Client disconnected");
  }
  else if (type == WS_EVT_DATA)
  {

    Serial.println("Data received: ");

    // for (int i = 0; i < len; i++) {
    //   Serial.print((char) data[i]);
    // }
    Serial.println();
    char command[len + 1];
    int split = 0;
    for (int i = 0; i < len; i++)
    {
      Serial.print((char)data[i]);
      command[i] = (char)data[i];
      if (command[i] == ':')
        split = i;
    }
    command[len] = '\0';
    /*
        Serial.println();
        Serial.println("LEN");
        Serial.println(len);
        Serial.println(command);
        Serial.println(split, DEC);
        Serial.println();
        int test1 = strcmp("pitch", command);
        Serial.println("test");
        Serial.println(test1, DEC);
        */
    char name[split + 1];
    for (int i = 0; i < split; i++)
    {
      name[i] = command[i];
    }
    name[split] = '\0';

    char value[len - split + 1];
    for (int i = 0; i < len - split - 1; i++)
    {
      value[i] = command[split + i + 1];
    }
    value[len - split] = '\0';
    int pitchValue = atoi(value);
    if (strcmp("pitch", name) == 0 && pitchValue != 0)
    {
      ///// pitch = pitchValue;
     //// tableStep = pitch * tablesize / samplerate;
      Serial.println("SETTING PITCH");
    }

    Serial.println("name");
    Serial.print("\"");
    Serial.print(name);
    Serial.println("\"");

    Serial.println("value");
    Serial.print("\"");
    Serial.println(value);
    Serial.println("\"");
    Serial.println("#value");
    Serial.println(atoi(value), DEC);
    Serial.println("strcmp()");
    Serial.println(strcmp("pitch", name), DEC);
  }
}
