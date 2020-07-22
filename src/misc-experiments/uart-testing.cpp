#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "string.h"
#include "driver/gpio.h"

/////////////////////////
void init(void) {
    const uart_config_t uart_config = {
        .baud_rate = 31250,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
        // ,         .source_clk = 0x0, // UART_SCLK_APB
    };
    // We won't use a buffer for sending data.
    uart_driver_install(UART_NUM_1, RX_BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(UART_NUM_1, &uart_config);
    uart_set_pin(UART_NUM_1, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}

int sendData(const char* logName, const char* data)
{
    const int len = strlen(data);
    const int txBytes = uart_write_bytes(UART_NUM_1, data, len);
    // ESP_LOGI(logName, "Wrote %d bytes", txBytes);
   // Serial.print("Writing bytes: ");
   // Serial.println(txBytes, DEC);
    return txBytes;
}

static void tx_task(void *arg)
{
  Serial.print("tx_task");
    static const char *TX_TASK_TAG = "TX_TASK";
    // esp_log_level_set(TX_TASK_TAG, ESP_LOG_INFO);
    while (1) {
        sendData(TX_TASK_TAG, "01234567"); // Hello world
       // vTaskDelay(2000 / portTICK_PERIOD_MS);
        vTaskDelay(10);
    }
}

int count = 0;

static void rx_task(void *arg)
{
  Serial.print("rx_task");
    static const char *RX_TASK_TAG = "RX_TASK";
    // esp_log_level_set(RX_TASK_TAG, ESP_LOG_INFO);
    uint8_t* data = (uint8_t*) malloc(RX_BUF_SIZE+1);
    while (1) {
      if (count++ % 64 == 0)
    {
      Serial.println("+");
      //  plotter.Plot();
    }
      
     // vTaskDelay(10);
        const int rxBytes = uart_read_bytes(UART_NUM_1, data, RX_BUF_SIZE, 10); // 1000 / portTICK_RATE_MS
        if (rxBytes > 0) {
            data[rxBytes] = 0;
            // ESP_LOGI(RX_TASK_TAG, "Read %d bytes: '%s'", rxBytes, data);
            // ESP_LOG_BUFFER_HEXDUMP(RX_TASK_TAG, data, rxBytes, ESP_LOG_INFO);
            char str[rxBytes+1];
memcpy(str, data, rxBytes);
           // for(int i=0;i<rxBytes;i++){
// Serial.print((char)data[i]);
// Serial.print("*");
            // }
           Serial.print(str);
Serial.print("*"); 
        }
    }
    free(data);
}
////////
