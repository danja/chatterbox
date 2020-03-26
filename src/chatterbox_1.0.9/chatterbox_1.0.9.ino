/*
    Chatterbox

    a voice-like sound generator

    see http://github.com/danja/chatterbox

    unless otherwise stated, MIT license for code, attribution appreciated

    Danny Ayers 2020 | danny.ayers@gmail.com | #danja | http://hyperdata.it

*/

#include <Arduino.h>
#include <driver/adc.h> // depends on Espressif ESP32 libs
#include "i2sdac.h" // see src/lib - based on https://github.com/wjslager/esp32-dac 
// #include "Biquad.h";
#include "SvfLinearTrapOptimised2.hpp";

#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include "SPIFFS.h"

#define SAMPLERATE 22050

// I2C DAC interface
#define GPIO_DAC_DATAPORT 0
#define GPIO_DAC_BCLK 26
#define GPIO_DAC_WSEL 25
#define GPIO_DAC_DOUT 27

#define ADC_TOP 4096

#define TABLESIZE 1024

#define NWAVES 7

#define LARYNX_WAVE 0
#define SINE_WAVE 1
#define SQUARE_WAVE 2
#define SAWTOOTH_WAVE 3
#define TRIANGLE_WAVE 5
#define MAX_WAVE 6

#define ADC_SAMPLES 64 // pot reading takes mean over this number of values

#define INPUT_LOCAL 0
#define INPUT_WEB 1

#define POTS 6
#define SWITCHES 5

#define POT_F1     0
#define POT_F2     1
#define POT_F3_F   2
#define POT_F3_Q   3
#define POT_PITCH  5
#define POT_LARYNX 4

#define SWITCH_VOICED 3
#define SWITCH_ASPIRATED 4
#define SWITCH_SF1 0
#define SWITCH_SF2 1
#define SWITCH_SF3 2

#define SWITCH_WAVEFORM 0

// Variable parameter ranges
#define F_MIN 20
#define F_MAX 500
#define LARYNX_MIN 5 // % of wave is larynx open
#define LARYNX_MAX 95
#define F1_LOW  150
#define F1_HIGH  1400
#define F2_LOW  500
#define F2_HIGH  5000
#define F3_LOW  50 //
#define F3_HIGH  7000 //
#define F3_Q_MIN  1
#define F3_Q_MAX  5

// Fixed parameter values
#define F1_Q 10
#define F2_Q 15

#define SF1_F  2100
#define SF2_F  3700
#define SF3_F  5600

#define SF1_Q  10
#define SF2_Q  12
#define SF3_Q  14

#define HTTP_PORT 80

I2sDAC dac;
TaskHandle_t AudioTask;
AsyncWebServer server(HTTP_PORT);


const char* ssid = "TP-LINK_AC17DC"; // HIDE ME!
const char* password =  "33088297"; // HIDE ME!

IPAddress local_IP(192, 168, 0, 142);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);   //optional
IPAddress secondaryDNS(8, 8, 4, 4); //optional

// Used by Web Interface
String potID[POTS];
String switchID[SWITCHES];

float inputScale[POTS];
int inputOffset[POTS];

const int WAVEFORML = LARYNX_WAVE;
// const int WAVEFORMR = FILTERED_WAVE;

char currentWave = WAVEFORML;

int pitchRaw = 0;

float samplerate = (float)SAMPLERATE;
float tablesize = (float)TABLESIZE; // save a bit of casting

float wavetableL[TABLESIZE];
float wavetableR[TABLESIZE];
float tableStep = 1;

int bufferIndex = 0;

// Inputs
int potChannel[POTS]; // adc1_channel_t
int potValue[POTS];

char switchChannel[] = {22, 23, 12, 13, 14};
float switchGain[] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
char nswitches =  (int)(sizeof(switchChannel) / sizeof(switchChannel[0]));

float attackTime = 0.01f; // 10mS
float attackStep = (float)ADC_TOP / (samplerate*attackTime);

float decayTime = 0.01f; // 10mS
float decayStep = (float)ADC_TOP / (samplerate*decayTime);

void loop() {
  // vTaskDelay(1);
};

//   Biquad(int type, float Fc, float Q, float peakGainDB);
// Biquad *formant1 = new Biquad(LOWPASS, fc, 3.0, 0);

// ==== ==== ==== ==== ==== ==== ==== ==== ==== ==== ==== ====
void setup()
{
  Serial.begin(115200);
  Serial.println("\n*** Starting Chatterbox ***\n");

  dac.begin(SAMPLERATE, GPIO_DAC_DATAPORT, GPIO_DAC_BCLK, GPIO_DAC_WSEL, GPIO_DAC_DOUT);

  initWavetable(wavetableL, WAVEFORML);
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


  // Higher priority web thread
  /*
    xTaskCreatePinnedToCore(
    WebServer,
    "web",
    2048,
    NULL,
    1,
    NULL,
    1);
  */

  potID[0] = "F1";
  potID[1] = "F2";
  potID[2] = "F3_F";
  potID[3] = "F3_Q";
  potID[4] = "WAVEFORM";
  potID[5] = "PITCH";

  switchID[0] = "SF1";
  switchID[1] = "SF2";
  switchID[2] = "SF3";
  switchID[3] = "VOICED";
  switchID[4] = "ASPIRATED";

  startWebServer();
}

/* INITIALIZE WAVETABLE */
int bottomSize = TABLESIZE / 2;
int tableSplit = 1024;

void initWavetable(float wavetable[], char waveform) {
  switch (waveform) {
    case LARYNX_WAVE:
      {
        // Serial.println("Init larynx wavetable");

        for (unsigned int i = 0; i < tableSplit / 2; i++) { // up slope /
          wavetable[i] = 2.0f * (float)i / (float)tableSplit - 1.0f;
        }
        for (unsigned int i = tableSplit / 2; i < tableSplit; i++) { // down slope
          wavetable[i] = 1.0f - 2.0f * (float)i / (float)tableSplit ;
        }

        for (unsigned int i = tableSplit; i < TABLESIZE; i++) { // flat section
          wavetable[i] = -0.99;
        }

        break;
      }

    case SINE_WAVE:
      {
        Serial.println("Init sine wavetable");
        for (unsigned int i = 0; i < TABLESIZE; i++) {
          wavetable[i] = sin(2.0 * PI * (float)i / (float)TABLESIZE);
        }
        break;
      }

    case SQUARE_WAVE:
      {
        Serial.println("Init square wavetable");
        int bottomSize = (TABLESIZE * tableSplit) / ADC_TOP;
        for (unsigned int i = 0; i < bottomSize; i++) {
          wavetable[i] = -0.99;
        }
        for (unsigned int i = bottomSize; i < TABLESIZE; i++) {
          wavetable[i] = 0.99;
        }
        break;
      }

    case SAWTOOTH_WAVE:
      {
        Serial.println("Init sawtooth wavetable");
        for (unsigned int i = 0; i < TABLESIZE; i++) {
          wavetable[i] = 2 * i / tablesize - 1;
        }
        break;
      }

    case TRIANGLE_WAVE:
      {
        Serial.println("Init triangle wavetable");
        for (unsigned int i = 0; i < TABLESIZE / 2; i++) {
          wavetable[i] =  2 * i / (tablesize / 2) - 1;
        }
        for (unsigned int i = TABLESIZE / 2; i < TABLESIZE; i++) {
          wavetable[i] = 1 - 2 * i / (tablesize / 2);
        }
        break;
      }

    case MAX_WAVE:
      {
        Serial.println("Init max wavetable");
        for (unsigned int i = 0; i < TABLESIZE - 1; i = i + 2) {
          wavetable[i] =  -0.99;
          wavetable[i + 1] =  0.99;
        }

        break;
      }
  }
}
/* END INITIALIZE WAVETABLE */

char controlSource = INPUT_LOCAL;

/* INITIALISE INPUTS */
void initInputs() {

  // init ADC inputs (pots)
  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_11);

  potChannel[0] = 36; // GPIO 36
  potChannel[1] = 39; // GPIO 39
  potChannel[2] = 32; // GPIO 32
  potChannel[3] = 33; // GPIO 33
  potChannel[4] = 34; // GPIO 34
  potChannel[5] = 35; // GPIO 35

  for (char i = 0; i < POTS; i++) {
    adcAttachPin(potChannel[i]);
    potValue[i] = 0;
    inputScale[i] = 1.0f;
  }

  inputOffset[POT_PITCH] =  F_MIN;
  inputScale[POT_PITCH] = (float)(F_MAX - F_MIN) / (float)ADC_TOP;
  inputScale[POT_LARYNX] =  tablesize * (float)(LARYNX_MAX - LARYNX_MIN) / 100.0f;
  inputOffset[POT_LARYNX] = tablesize * (float)LARYNX_MIN / 100.0f;

  inputOffset[POT_F1] = F1_LOW;
  inputScale[POT_F1] = (float)(F1_HIGH - F1_LOW) / (float)ADC_TOP;
  inputOffset[POT_F2] = F2_LOW;
  inputScale[POT_F2] = (float)(F2_HIGH - F2_LOW) / (float)ADC_TOP;
  inputOffset[POT_F3_F] =  F3_LOW;
  inputScale[POT_F3_F] = (float)(F3_HIGH - F3_LOW) / (float)ADC_TOP;
  inputOffset[POT_F3_Q] =  F3_Q_MIN;
  inputScale[POT_F3_Q] = (float)(F3_Q_MAX - F3_Q_MIN) / (float)ADC_TOP;


  // init switch inputs
  switchChannel[0] = 22; //  GPIO 22
  switchChannel[1] = 23; //  GPIO 23
  switchChannel[2] = 12; // GPIO 12
  switchChannel[3] = 13; // GPIO 13
  switchChannel[4] = 14; // GPIO 14

  for (char i = 0; i < SWITCHES; i++) {
    switchGain[i] = 0;
    pinMode (switchChannel[i], INPUT);
    pinMode(switchChannel[i], INPUT_PULLDOWN);
  }
}
/* END INITIALISE INPUTS */

/*
   see https://mathr.co.uk/blog/2017-09-06_approximating_hyperbolic_tangent.html
*/
float softClip(float x) { // tanh approx
  float x2 = x * x;
  float x4 = x2 * x2;
  return x * (10395 + 1260 * x2 + 21 * x4) / (10395 + 4725 * x2 + 210 * x4 + x2 * x4);
}

// *** Initialise filters ***
// Variable formant filters
SvfLinearTrapOptimised2 formant1;
SvfLinearTrapOptimised2 formant2;
SvfLinearTrapOptimised2 formant3;

SvfLinearTrapOptimised2::FLT_TYPE formant1_type = SvfLinearTrapOptimised2::BAND_PASS_FILTER;
SvfLinearTrapOptimised2::FLT_TYPE formant2_type = SvfLinearTrapOptimised2::BAND_PASS_FILTER;
SvfLinearTrapOptimised2::FLT_TYPE formant3_type = SvfLinearTrapOptimised2::LOW_PASS_FILTER;

// fixed sibilant/fricative filters
SvfLinearTrapOptimised2 sf1;
SvfLinearTrapOptimised2 sf2;
SvfLinearTrapOptimised2 sf3;

SvfLinearTrapOptimised2::FLT_TYPE sf1_type = SvfLinearTrapOptimised2::HIGH_PASS_FILTER;
SvfLinearTrapOptimised2::FLT_TYPE sf2_type = SvfLinearTrapOptimised2::HIGH_PASS_FILTER;
SvfLinearTrapOptimised2::FLT_TYPE sf3_type = SvfLinearTrapOptimised2::HIGH_PASS_FILTER;

float pitch;
float formant1freq;
float formant2freq;
float f3freq;
float f3Q;

//* INPUT THREAD */
void ControlInput(void *pvParameter)
{
  initInputs();

  float filterGaindB = 0; // Gain to boost or cut the cutoff
  //float linearLevel = pow(10.0, leveldB / 20.0);
  float fc = 100;
  float Q = 5; // Q

  while (1) {
    // vTaskDelay(1000 / portTICK_RATE_MS); // was 1000
    vTaskDelay(2);

    // take mean of ADC readings, they are prone to noise

    for (char pot = 0; pot < POTS; pot++) {
      int sum = 0;
      for (int j = 0; j < ADC_SAMPLES; j++) {
        sum +=  analogRead(potChannel[pot]); // adc1_get_raw(ADC1_CHANNEL_0);
        // vTaskDelay(1);
      }
      potValue[pot] = sum / ADC_SAMPLES;
    }

    // PITCH CONTROL
    pitch = ((float)inputOffset[POT_PITCH] + inputScale[POT_PITCH] *  (float)potValue[POT_PITCH]);

    if (controlSource == INPUT_LOCAL)
      tableStep = pitch * tablesize / samplerate; // tableStep aka delta

    // LARYNX WAVEFORM CONTROL
    if (abs(tableSplit - potValue[POT_LARYNX]) > 8) {

      float potU = (float)potValue[POT_LARYNX] / (float)ADC_TOP;

      if (controlSource == INPUT_LOCAL)
        tableSplit = inputOffset[POT_LARYNX] + potU * inputScale[POT_LARYNX];

      initWavetable(wavetableL, currentWave);
    }

    if (controlSource == INPUT_LOCAL) {
      formant1freq = inputOffset[POT_F1] + (float)potValue[POT_F1] * inputScale[POT_F1];
      formant2freq = inputOffset[POT_F2] + (float)potValue[POT_F2] * inputScale[POT_F2];
      f3freq = inputOffset[POT_F3_F] + (float)potValue[POT_F3_F] * inputScale[POT_F3_F];
      f3Q = inputOffset[POT_F3_Q] + (float)potValue[POT_F3_Q] * inputScale[POT_F3_Q];
    }

    formant1.updateCoefficients(formant1freq, F1_Q, formant1_type, samplerate);
    formant2.updateCoefficients(formant2freq, F2_Q, formant2_type, samplerate);
    formant3.updateCoefficients(f3freq, f3Q, formant3_type, samplerate);
    // updateCoefficients(double cutoff, double q = 0.5, FLT_TYPE type = LOW_, double sampleRate = 44100)


    /* using biquad
      fc = potValue[POT_F2];
      formant2.updateCoefficients(fc, Q, formant2_type, samplerate);
      }
    */

    for (char i = 0; i < nswitches; i++) {
      if (digitalRead(switchChannel[i]) == 1) {
        switchGain[i] += attackStep;
        if (switchGain[i] > 1) switchGain[i] = 1;
      } else {
        switchGain[i] -= decayStep;
        if (switchGain[i] < 1) switchGain[i] = 0;
      }
    }
  }
}
/* END INPUT THREAD */



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
  sf1.updateCoefficients(SF1_F, SF1_Q, sf1_type, samplerate);
  sf2.setGain(filterGaindB);
  sf2.updateCoefficients(SF2_F, SF2_Q, sf2_type, samplerate);
  sf3.setGain(filterGaindB);
  sf3.updateCoefficients(SF3_F, SF3_Q, sf3_type, samplerate);

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

    float voice = wavetableL[lower] * err + wavetableL[upper] * (1 - err);

    float noise = random(-32768, 32767) / 32768.0f;
    noise = noise / 4.0f;

    // *** System block connections ***
    voice = switchGain[SWITCH_VOICED] * voice;
    float aspiration = switchGain[SWITCH_ASPIRATED] * noise;

    float f1_in = (voice + aspiration) / 2.0f;

    float s1 = sf1.tick(noise) * switchGain[SWITCH_SF1] / 8.0f;
    float s2 = sf2.tick(noise) * switchGain[SWITCH_SF2] / 4.0f;
    float s3 = sf3.tick(noise) * switchGain[SWITCH_SF3] / 2.0f;

    float sibilants = softClip(s1 + s2 + s3);

    float f3_in = softClip(formant1.tick(f1_in) + sibilants);
    float f2_in = softClip(formant3.tick(f3_in));

    float  valL = softClip(formant2.tick(f2_in));
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
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("STA Failed to configure");
  }

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  Serial.print("Connected to : ");
  Serial.println(WiFi.localIP());

  server.begin();

  server.on("/index.html", HTTP_GET, [](AsyncWebServerRequest * request) {
    // request->send(200, "text/plain", "Hello from Chatterbox!");
    request->send(SPIFFS, "/index.html", String(), false, pageProcessor);
  });
    server.on("/chatterbox.html", HTTP_GET, [](AsyncWebServerRequest * request) {
    // request->send(200, "text/plain", "Hello from Chatterbox!");
    request->send(SPIFFS, "/chatterbox.html", String(), false, pageProcessor);
  });
}
/* END WEB SERVER */

String pageProcessor(const String& var) {
  char snum[5];

  if (var == "PITCH") {
     itoa((int)pitch, snum, 10);
      return snum;
  }
    if (var == "WAVEFORM") {
     itoa(tableSplit, snum, 10);
      return snum;
  }
    if (var == "F1") {
     itoa(formant1freq, snum, 10);
      return snum;
  }
      if (var == "F2") {
     itoa(formant2freq, snum, 10);
      return snum;
  }
      if (var == "F3_F") {
     itoa(f3freq, snum, 10);
      return snum;
  }
      if (var == "F3_Q") {
     itoa(f3Q, snum, 10);
      return snum;
  }  
/*
  for (char i = 0; i < POTS; i++) {
    if (var.compareTo(potID[i]) == 0) {
      itoa(potValue[i], snum, 10);
      return snum;
    }
  }
  */

  for (char i = 0; i < SWITCHES; i++) {
    if (var.compareTo(switchID[i]) == 0) {
      if (switchGain[i] < 0.5f) {
        return "off";
      } else {
        return "on";
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