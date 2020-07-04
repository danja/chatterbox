#pragma once

class I2sDAC
{
public:
	I2sDAC();
	~I2sDAC();

//	bool begin(int sampleRate = 44100, int dataPort = 0, int bclk = 26, int wsel = 25, int dout = 33);
bool begin(int sampleRate = 22050, int dataPort = 0, int bclk = 26, int wsel = 25, int dout = 33);
	// Tries to write a sample into the DMA buffer
	// Tries again until the sample has been accepted
	void writeSample(int16_t left, int16_t right);
	void writeSample(float left, float right);

private:
	int8_t port;

	// When the DMA buffer is full, written samples will be discarded
	// You should check if the returned value equals sizeof(uint32_t)
	// This way you know if a sample has been written or discarded and can try again if needed
	size_t write(int16_t left, int16_t right);
};

