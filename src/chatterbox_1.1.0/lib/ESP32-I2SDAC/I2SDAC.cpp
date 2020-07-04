#include <Arduino.h>
#include <driver/i2s.h>
#include <I2SDAC.h>

I2sDAC::I2sDAC() {}

I2sDAC::~I2sDAC() {}

bool I2sDAC::begin(int sampleRate, int dataPort, int bclk, int wsel, int dout)
{
	Serial.println("I2sDAC::begin");
	port = dataPort;

	i2s_config_t i2s_config = {
		.mode					= (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
		.sample_rate			= sampleRate,
		.bits_per_sample		= I2S_BITS_PER_SAMPLE_16BIT,
		.channel_format			= I2S_CHANNEL_FMT_RIGHT_LEFT,
		.communication_format	= (i2s_comm_format_t)(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
		.intr_alloc_flags		= ESP_INTR_FLAG_LEVEL1,
		.dma_buf_count			= 8,
		.dma_buf_len			= 64,
		.use_apll				= 0
	};

	// Install i2s driver, return false if it fails
	if (i2s_driver_install((i2s_port_t)port, &i2s_config, 0, NULL) != ESP_OK)
		return false;

	// Set the pin layout
	i2s_pin_config_t pinLayout = {
		.bck_io_num = bclk,
		.ws_io_num = wsel,
		.data_out_num = dout,
		.data_in_num = I2S_PIN_NO_CHANGE
	};

	i2s_set_pin((i2s_port_t)port, &pinLayout);

	// Fill the DMA buffer with zeros
	i2s_zero_dma_buffer((i2s_port_t)port);

	return true;
}

void I2sDAC::writeSample(int16_t left, int16_t right)
{
	size_t written = write(left, right);

	// Retry the sample sample until it is accepted
	
	while (written != sizeof(uint32_t))
		written = write(left, right);
		
}

void I2sDAC::writeSample(float left, float right)
{
	writeSample(static_cast<int16_t>(left * 32767), static_cast<int16_t>(right * 32767));
}

size_t I2sDAC::write(int16_t left, int16_t right)
{
	// Merge samples
	uint32_t sample = (left<<16) | (right & 0xffff);
	size_t written;

	// Write
	i2s_write((i2s_port_t)port, (const char*)&sample, sizeof(uint32_t), &written, 0);

	// Return number of written bytes (for checking if the sample is actually written)
	return written;
}


