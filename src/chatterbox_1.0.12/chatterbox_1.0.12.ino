/*
    Chatterbox

    a voice-like sound generator

    see http://github.com/danja/chatterbox

    unless otherwise stated: MIT license, attribution appreciated

    Danny Ayers 2020 | danny.ayers@gmail.com | #danja | https://hyperdata.it
*/


#include <Arduino.h>
#include <driver/adc.h> // depends on Espressif ESP32 libs
#include "i2sdac.h" // see src/lib - based on https://github.com/wjslager/esp32-dac 
#include "SvfLinearTrapOptimised2.hpp";
#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include "SPIFFS.h"

#define SERIAL_RATE 115200

#define SAMPLERATE 22050

// I2C DAC interface
#define GPIO_DAC_DATAPORT 0
#define GPIO_DAC_BCLK 26
#define GPIO_DAC_WSEL 25
#define GPIO_DAC_DOUT 27

#define ADC_TOP 4096

#define TABLESIZE 1024

#define ADC_SAMPLES 64 // pot reading takes mean over this number of values

#define INPUT_LOCAL 0
#define INPUT_WEB 1

/* CONTROLS */
#define N_POTS 6

#define POT_F1F     0
#define POT_F2F     1
#define POT_F3F   2
#define POT_F3Q   3
#define POT_PITCH  5
#define POT_LARYNX 4

#define N_SWITCHES 12

#define SWITCH_VOICED 3
#define SWITCH_ASPIRATED 4
#define SWITCH_SF1 0 // Sibilance Filter 1
#define SWITCH_SF2 1
#define SWITCH_SF3 2

#define SWITCH_X5 5
#define SWITCH_X6 6
#define SWITCH_X7 7

#define TOGGLE_T0 0
#define TOGGLE_T1 1
#define TOGGLE_T2 2
#define TOGGLE_T3 3

// Variable parameter ranges
#define PITCH_MIN 20
#define PITCH_MAX 500
#define LARYNX_MIN 5 // % of wave is larynx open
#define LARYNX_MAX 95

#define F1F_LOW  150 // formant filter 1 centre frequency lowest value
#define F1F_HIGH  1400
#define F2F_LOW  500
#define F2F_HIGH  5000
#define F3F_LOW  50 // F3 is auxiliary filter, may or may not be a formant
#define F3F_HIGH  7000
#define F3Q_MIN  1
#define F3Q_MAX  10

// Fixed parameter values
#define F1Q 10
#define F2Q 15

#define ATTACK_TIME 0.01f // switch envelope 
#define DECAY_TIME 0.01f

#define SF1F  2100 // Sibilance filter 1 centre frequency
#define SF2F  3700
#define SF3F  5600

#define SF1Q  10
#define SF2Q  12
#define SF3Q  14

/*** WEB COMMS ***/
#define HTTP_PORT 80

/**** Local Network specific - TODO must pull out ****/
const char* ssid = "TP-LINK_AC17DC"; // HIDE ME!
const char* password =  "33088297"; // HIDE ME!

IPAddress localIP(192, 168, 0, 142);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);   //optional
IPAddress secondaryDNS(8, 8, 4, 4); //optional
/***********************************************/

/* KEY COMPONENTS */
I2sDAC dac;
TaskHandle_t AudioTask;
AsyncWebServer server(HTTP_PORT);
AsyncWebSocket ws("/ws");

// save a bit of casting
float samplerate = (float)SAMPLERATE;
float tablesize = (float)TABLESIZE;

float wavetable[TABLESIZE];
float tableStep = 1;

int bufferIndex = 0;

// Used by Web Interface
String potID[N_POTS];
String switchID[N_SWITCHES];

// for scaling ADC readings to required values
float inputScale[N_POTS];
int inputOffset[N_POTS];

// Inputs TODO move to defines
int potChannel[] = {36, 39, 32, 33, 34, 35};
int potValue[N_POTS];
int previousPotValue[N_POTS];

// TODO move to defines
char switchChannel[] = {19, 23, 12, 13, 14,    17, 18, 5,     16, 15, 2, 4}; // TODO move this to a define,
// is N_SWITCHES
// char nswitches =  (int)(sizeof(switchChannel) / sizeof(switchChannel[0]));

float switchGain[N_SWITCHES];
float switchValue[] = {false, false, false, false, false, false, false, false, false, false, false, false};  // TODO move these to a loop
float previousSwitchValue[] = {false, false, false, false, false, false, false, false, false, false, false, false};

float attackTime = ATTACK_TIME; // 10mS
float attackStep = (float)ADC_TOP / (samplerate*attackTime);

float decayTime = DECAY_TIME; // 10mS
float decayStep = (float)ADC_TOP / (samplerate*decayTime);

void loop() { // do nothing
  // vTaskDelay(1);
};


/* *** START SETUP() *** */
void setup()
{
  Serial.begin(SERIAL_RATE);
  Serial.println("\n*** Starting Chatterbox ***\n");

  dac.begin(SAMPLERATE, GPIO_DAC_DATAPORT, GPIO_DAC_BCLK, GPIO_DAC_WSEL, GPIO_DAC_DOUT);

  initWavetable();
  // Serial.println("portTICK_RATE_MS = " + portTICK_RATE_MS);

  // Try to start the DAC
  if (dac.begin()) {
    Serial.println("DAC init success");
  } else {
    Serial.println("DAC init fail");
  }

  // Highest priority audio thread
  xTaskCreatePinnedToCore(
    OutputDAC,
    "audio",
    2048, // was 4096
    NULL,
    1,
    &AudioTask,
    0);

  // Lower priority input thread
  xTaskCreatePinnedToCore(
    ControlInput,
    "ControlInput",
    2048,
    NULL,
    2, // priority
    NULL,
    1); // core

  potID[0] = "f1f";
  potID[1] = "f2f";
  potID[2] = "f3f";
  potID[3] = "f3q";
  potID[4] = "larynx";
  potID[5] = "pitch";

  switchID[0] = "sf1";
  switchID[1] = "sf2";
  switchID[2] = "sf3";
  switchID[3] = "voiced";
  switchID[4] = "aspirated";

  switchID[5] = "x5";
  switchID[6] = "x6";
  switchID[7] = "x7";

  switchID[8] = "T0";
  switchID[9] = "T1";
  switchID[10] = "T2";
  switchID[11] = "T3";

  startWebServer();
}
/*** END SETUP() ***/

/* INITIALIZE WAVETABLE */
int larynx = 1024; // point in wavetable corresponding to closed larynx

void initWavetable() { // creates wavetable of this shape, approximating : /\__

  for (unsigned int i = 0; i < larynx / 2; i++) { // up slope /
    wavetable[i] = 2.0f * (float)i / (float)larynx - 1.0f;
  }
  for (unsigned int i = larynx / 2; i < larynx; i++) { // down slope \
    wavetable[i] = 1.0f - 2.0f * (float)i / (float)larynx ;
  }

  for (unsigned int i = larynx; i < TABLESIZE; i++) { // flat section __
    wavetable[i] = -0.99;
  }
}
/* END INITIALIZE WAVETABLE */

char controlSource = INPUT_LOCAL;

/* INITIALISE INPUTS */
void initInputs() {

  // init ADC inputs (pots)
  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_11);

  for (char i = 0; i < N_POTS; i++) {
    adcAttachPin(potChannel[i]);
    potValue[i] = 0;
    previousPotValue[i] = 0;
    inputScale[i] = 1.0f;
  }

  for (char i = 0; i < N_SWITCHES; i++) {
    switchGain[i] = 0;
    pinMode (switchChannel[i], INPUT);
    pinMode(switchChannel[i], INPUT_PULLDOWN);
  }

  // calculate values for offset/scaling from pots to parameters
  inputOffset[POT_PITCH] =  PITCH_MIN;
  inputScale[POT_PITCH] = (float)(PITCH_MAX - PITCH_MIN) / (float)ADC_TOP;
  inputScale[POT_LARYNX] =  tablesize * (float)(LARYNX_MAX - LARYNX_MIN) / 100.0f;
  inputOffset[POT_LARYNX] = tablesize * (float)LARYNX_MIN / 100.0f;

  inputOffset[POT_F1F] = F1F_LOW;
  inputScale[POT_F1F] = (float)(F1F_HIGH - F1F_LOW) / (float)ADC_TOP;
  inputOffset[POT_F2F] = F2F_LOW;
  inputScale[POT_F2F] = (float)(F2F_HIGH - F2F_LOW) / (float)ADC_TOP;
  inputOffset[POT_F3F] =  F3F_LOW;
  inputScale[POT_F3F] = (float)(F3F_HIGH - F3F_LOW) / (float)ADC_TOP;
  inputOffset[POT_F3Q] =  F3Q_MIN;
  inputScale[POT_F3Q] = (float)(F3Q_MAX - F3Q_MIN) / (float)ADC_TOP;
}
/* END INITIALISE INPUTS */

/*
   tanh approximation
   see https://mathr.co.uk/blog/2017-09-06_approximating_hyperbolic_tangent.html
*/
float softClip(float x) {
  float x2 = x * x;
  float x4 = x2 * x2;
  return x * (10395 + 1260 * x2 + 21 * x4) / (10395 + 4725 * x2 + 210 * x4 + x2 * x4);
}

// *** Initialise filters ***

// Variable formant filters
SvfLinearTrapOptimised2 f1;
SvfLinearTrapOptimised2 f2;
SvfLinearTrapOptimised2 f3;

SvfLinearTrapOptimised2::FLT_TYPE f1Type = SvfLinearTrapOptimised2::BAND_PASS_FILTER;
SvfLinearTrapOptimised2::FLT_TYPE f2Type = SvfLinearTrapOptimised2::BAND_PASS_FILTER;
SvfLinearTrapOptimised2::FLT_TYPE f3Type = SvfLinearTrapOptimised2::LOW_PASS_FILTER;

// fixed sibilant/fricative filters
SvfLinearTrapOptimised2 sf1;
SvfLinearTrapOptimised2 sf2;
SvfLinearTrapOptimised2 sf3;

SvfLinearTrapOptimised2::FLT_TYPE sf1Type = SvfLinearTrapOptimised2::HIGH_PASS_FILTER;
SvfLinearTrapOptimised2::FLT_TYPE sf2Type = SvfLinearTrapOptimised2::HIGH_PASS_FILTER;
SvfLinearTrapOptimised2::FLT_TYPE sf3Type = SvfLinearTrapOptimised2::HIGH_PASS_FILTER;

float pitch;
// larynx is initialized at wavetable
float f1f;
float f2f;
float f3f;
float f3q;

/* *** INPUT THREAD *** */
void ControlInput(void *pvParameter)
{
  initInputs();

  float filterGaindB = 0; // Gain to boost or cut the cutoff

  while (1) {
    // vTaskDelay(1000 / portTICK_RATE_MS); // was 1000
    vTaskDelay(2);

    // take mean of ADC readings, they are prone to noise

    for (char pot = 0; pot < N_POTS; pot++) {
      int sum = 0;
      for (int j = 0; j < ADC_SAMPLES; j++) {
        sum +=  analogRead(potChannel[pot]); // get value from pot / ADC

      }
      potValue[pot] = sum / ADC_SAMPLES;
    }

    // pitch control
    pitch = ((float)inputOffset[POT_PITCH] + inputScale[POT_PITCH] *  (float)potValue[POT_PITCH]);
    if (controlSource == INPUT_LOCAL)
      tableStep = pitch * tablesize / samplerate; // tableStep aka delta

    // larynx control
    if (abs(larynx - potValue[POT_LARYNX]) > 8) {

      float potFraction = (float)potValue[POT_LARYNX] / (float)ADC_TOP;

      if (controlSource == INPUT_LOCAL)
        larynx = inputOffset[POT_LARYNX] + potFraction * inputScale[POT_LARYNX];

      initWavetable();
    }

    if (controlSource == INPUT_LOCAL) {
      f1f = inputOffset[POT_F1F] + (float)potValue[POT_F1F] * inputScale[POT_F1F];
      f2f = inputOffset[POT_F2F] + (float)potValue[POT_F2F] * inputScale[POT_F2F];
      f3f = inputOffset[POT_F3F] + (float)potValue[POT_F3F] * inputScale[POT_F3F];
      f3q = inputOffset[POT_F3Q] + (float)potValue[POT_F3Q] * inputScale[POT_F3Q];
    }

    f1.updateCoefficients(f1f, F1Q, f1Type, samplerate); // TODO allow variable Q?
    f2.updateCoefficients(f2f, F2Q, f2Type, samplerate);
    f3.updateCoefficients(f3f, f3q, f3Type, samplerate);
    // updateCoefficients(double cutoff, double q = 0.5, FLT_TYPE type = LOW_, double sampleRate = 44100)

    for (char i = 0; i < N_SWITCHES; i++) { // switch envelope generator
      switchValue[i] = digitalRead(switchChannel[i]);
      if (switchValue[i] == 1) {
        switchGain[i] += attackStep;
        if (switchGain[i] > 1) switchGain[i] = 1;
      } else {
        switchGain[i] -= decayStep;
        if (switchGain[i] < 1) switchGain[i] = 0;
      }
    }

    pushToWebSocket();
  }
}
/* END INPUT THREAD */

const int potResolution = 32; // TODO sort out noise!!!!

void pushToWebSocket() {
  for (int i = 0; i < N_POTS; i++) {
    if (abs(potValue[i] - previousPotValue[i]) >= potResolution) {
      convertAndPush(potID[i], potChannel[i]);
      convertAndPush(potID[i], potValue[i]);
      previousPotValue[i] = potValue[i];
    }
  }

  for (char i = 0; i < N_SWITCHES; i++) {
    if (switchValue[i] != previousSwitchValue[i]) {
      convertAndPush(switchID[i], switchChannel[i]);
      if (switchValue[i]) {
        convertAndPush(switchID[i], 1);
      } else {
        convertAndPush(switchID[i], 0);
      }
      previousSwitchValue[i] = switchValue[i];
    }
  }

}

void convertAndPush(String id, int value) {
  char snum[5];
  itoa(value, snum, 10);
  String message = id + ":" + snum;
  Serial.println(message);
  ws.textAll(message);
  //   ws.textAll((char*)text);
}

void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len) {

  if (type == WS_EVT_CONNECT) {

    Serial.println("Websocket client connection received");
    client->text("Hello from ESP32 Server");

  } else if (type == WS_EVT_DISCONNECT) {
    Serial.println("WS Client disconnected");
  }  else if (type == WS_EVT_DATA) {

    Serial.println("Data received: ");

    for (int i = 0; i < len; i++) {
      Serial.print((char) data[i]);
    }
    Serial.println();
  }
}


/* OUTPUT THREAD */
void OutputDAC(void *pvParameter)
{
  unsigned int frameCount = 0;
  int16_t sample[2];
  float sampleFloat[2];

  Serial.print("Audio thread started at core: ");
  Serial.println(xPortGetCoreID());

  float filterGaindB = -12;
  sf1.setGain(filterGaindB);
  sf1.updateCoefficients(SF1F, SF1Q, sf1Type, samplerate); // TODO make SF1F etc variable?
  sf2.setGain(filterGaindB);
  sf2.updateCoefficients(SF2F, SF2Q, sf2Type, samplerate);
  sf3.setGain(filterGaindB);
  sf3.updateCoefficients(SF3F, SF3Q, sf3Type, samplerate);

  int pointer = 0;

  while (1) {

    // *** Read wavetable voice ***
    pointer = pointer + tableStep;

    if (pointer >= tablesize) pointer = pointer - (float)tablesize;

    float err = 0;

    // interpolate between neighbouring values
    err = pointer - floor(pointer);

    int lower = (int)floor(pointer);
    int upper = ((int)ceil(pointer)) % TABLESIZE;

    float voice = wavetable[lower] * err + wavetable[upper] * (1 - err);

    float noise = random(-32768, 32767) / 32768.0f;
    noise = noise / 4.0f;

    // *** System block connections ***
    voice = switchGain[SWITCH_VOICED] * voice;
    float aspiration = switchGain[SWITCH_ASPIRATED] * noise;

    float f1f_in = (voice + aspiration) / 2.0f;

    float s1 = sf1.tick(noise) * switchGain[SWITCH_SF1] / 8.0f;
    float s2 = sf2.tick(noise) * switchGain[SWITCH_SF2] / 4.0f;
    float s3 = sf3.tick(noise) * switchGain[SWITCH_SF3] / 2.0f;

    float sibilants = softClip(s1 + s2 + s3);

    float f3_in = softClip(f1.tick(f1f_in) + sibilants);
    float f2f_in = softClip(f3.tick(f3_in));

    float  valL = softClip(f2.tick(f2f_in));
    float  valR = softClip(voice);


    dac.writeSample(valL, valR);

    // Pause thread after delivering 64 samples so that other threads can do stuff
    if (frameCount++ % 64 == 0) vTaskDelay(1); // was 64, 1
  }
}
// END OUTPUT THREAD

/* WEB SERVER THREAD */

void startWebServer()
{
  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  // Configures static IP address
  if (!WiFi.config(localIP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("STA Failed to configure");
  }

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  Serial.print("Connected to : ");
  Serial.println(WiFi.localIP());

  ws.onEvent(onWsEvent);
  server.addHandler(&ws);

  server.begin();

  server.on("/index.html", HTTP_GET, [](AsyncWebServerRequest * request) {
    // request->send(200, "text/plain", "Hello from Chatterbox!");
    request->send(SPIFFS, "/index.html", String(), false, pageProcessor);
  });

  server.on("/chatterbox.html", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/chatterbox.html", String(), false, pageProcessor);
  });
  server.on("/chatterbox.css", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/chatterbox.css", String(), false, pageProcessor);
  });

  server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/favicon.ico", String(), false, pageProcessor);
  });
}
/* END WEB SERVER */

String pageProcessor(const String& var) {
  char snum[5];

  if (var == "pitch") {
    itoa((int)pitch, snum, 10);
    return snum;
  }
  if (var == "larynx") {
    itoa(larynx, snum, 10);
    return snum;
  }
  if (var == "f1f") {
    itoa(f1f, snum, 10);
    return snum;
  }
  if (var == "f2f") {
    itoa(f2f, snum, 10);
    return snum;
  }
  if (var == "f3f") {
    itoa(f3f, snum, 10);
    return snum;
  }
  if (var == "f3q") {
    itoa(f3q, snum, 10);
    return snum;
  }

  for (char i = 0; i < N_SWITCHES; i++) {
    if (var.compareTo(switchID[i]) == 0) {
      if (switchValue[i]) {
        return "on";
      } else {
        return "off";
      }
    }

  }

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
  return String();
}
