
#include "Processors.h"

// Processors::Processors{}()

void OutputDAC(void *pvParameter)
{
  unsigned int frameCount = 0;

  // Serial.print("Audio thread started at core: ");
  // Serial.println(xPortGetCoreID());

  fricative1.initParameters(SF1F, SF1Q, "highPass", samplerate); // TODO make SF1F etc variable?
  fricative2.initParameters(SF2F, SF2Q, "highPass", samplerate);
  fricative3.initParameters(SF3F, SF3Q, "highPass", samplerate);

  nasalLP.initParameters(NASAL_LPF, NASAL_LPQ, "lowPass", samplerate);
  nasalFixedBP.initParameters(NASAL_FIXEDBPF, NASAL_FIXEDBPQ, "bandPass", samplerate);
  nasalFixedNotch.initParameters(NASAL_FIXEDNOTCHF, NASAL_FIXEDNOTCHQ, "notch", samplerate);

 sing1.initParameters(SING1F, SING1Q, "bandPass", samplerate);
  sing2.initParameters(SING2F, SING2Q, "bandPass", samplerate);

  int pointer = 0;

  NoiseMaker creakNoise = NoiseMaker();
  NoiseMaker shoutNoise = NoiseMaker();
  NoiseMaker sf1Noise = NoiseMaker();
  NoiseMaker sf3Noise = NoiseMaker();
  // Shapers shaper = Shapers();

  ProcessorCreator processorCreator;
  Processor softClip = processorCreator.create(ProcessorCreator::SOFTCLIP);

  while (1)
  {
    // *** Read wavetable voice ***
    if (switches[TOGGLE_CREAK].on())
    {
      pointer = pointer + tableStep * (1.0f - creakNoise.stretchedNoise() * growl);
      if (pointer < 0)
        pointer = 0;
    }
    else
    {
      pointer = pointer + tableStep;
    }

    if (pointer >= tablesize)
      pointer = pointer - tablesize;

    float err = 0;

    // interpolate between neighbouring values
    err = pointer - floor(pointer);

    int lower = (int)floor(pointer);
    int upper = ((int)ceil(pointer)) % TABLESIZE;

    float larynxPart = larynxWavetable[lower] * err + larynxWavetable[upper] * (1 - err);
    float sinePart = sineWavetable[lower] * err + sineWavetable[upper] * (1 - err);
    float sawtoothPart = sawtoothWavetable[lower] * err + sawtoothWavetable[upper] * (1 - err);
    float voice = sineRatio * sinePart + sawtoothRatio * sawtoothPart + larynxRatio * larynxPart;

    // CHECK THE MIN/MAX OF A VALUE
    float monitorValue = larynxPart;
    if (monitorValue < minValue)
    {
      minValue = monitorValue;
    }
    if (monitorValue > maxValue)
    {
      maxValue = monitorValue;
    }
    if (switches[SWITCH_STRESS].on() && switches[SWITCH_DESTRESS].on())
    {
      Serial.print("minValue = ");
      Serial.println(minValue, DEC);
      Serial.print("maxValue = ");
      Serial.println(maxValue, DEC);
    }

    float noise = random(-32768, 32767) / 32768.0f;

    // ************************************************
    // ****************** THE WIRING ******************
    // ************************************************

    for (int i = 0; i < N_PUSH_SWITCHES; i++)
    {
      if (switches[i].on())
      {
        switches[i].gain(switches[i].gain() + attackStep); // TODO refactor
      }
      else
      {
        switches[i].gain(switches[i].gain() - decayStep); // TODO refactor
      }
    }

    voice = (switches[SWITCH_VOICED].gain() + switches[SWITCH_NASAL].gain()) * voice / 2.0f;

    float aspiration = switches[SWITCH_ASPIRATED].gain() * noise;

    float current = (emphasisGain * (voice + aspiration)) * SIGNAL_GAIN;

    if (switches[TOGGLE_SING].on())
    {
      float sing1Val = SING1_GAIN * sing1.process(current);
      float sing2Val = SING2_GAIN * sing2.process(current);
    
      current = softClip.process((current + sing1Val + sing2Val) / 3.0f);
    }

    if (switches[TOGGLE_SHOUT].on())
    {
      current = softClip.process(current * (1.0f - growl * shoutNoise.stretchedNoise())); // amplitude mod
    }

    float s1 = fricative1.process(sf1Noise.pink(noise)) * SF1_GAIN * switches[SWITCH_SF1].gain();
    float s2 = fricative2.process(noise) * SF2_GAIN * switches[SWITCH_SF2].gain();
    float s3 = 3 * fricative3.process(noise - sf3Noise.pink(noise)) * SF3_GAIN * switches[SWITCH_SF3].gain();

    float sibilants = softClip.process((s1 + s2 + s3) / 3.0f);

    float mix1 = current + sibilants;

    // pharynx/mouth is serial
    float mix2 = softClip.process(F1_GAIN * svf1.process(mix1));
    float mix3 = softClip.process(F2_GAIN * svf2.process(mix2));
    float mix4 = mix3;

    if (switches[SWITCH_NASAL].on())
    {
      mix4 = NASAL_LP_GAIN * nasalLP.process(mix3) + NASAL_FIXEDBP_GAIN * nasalFixedBP.process(mix3) + NASAL_FIXEDNOTCH_GAIN * nasalFixedNotch.process(mix3);

      mix4 = softClip.process(mix4 / 3.0f);
    }
    float mix5 = F3_GAIN * svf3.process(mix4);

    float valL = softClip.process(current);
    float valR = creakNoise.stretchedNoise();

    dac.writeSample(sinePart, mix5); //mix5

    // ****************** END WIRING ******************

    // Pause thread after delivering 64 samples so that other threads can do stuff
    if (frameCount++ % 64 == 0)
      vTaskDelay(1); // was 64, 1
  }
}