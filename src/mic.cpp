#include "mic.h"
#include "driver/i2s.h"
#include "driver/adc.h"

static const i2s_port_t I2S_PORT = (i2s_port_t)0;
static uint32_t g_sampleRate = 16000;

// ADC pin wiring: connect mic preamp output to GPIO34 (ADC1_CH6)
// If you have different wiring change this channel.
#define ADC_INPUT_UNIT ADC_UNIT_1
#define ADC_INPUT_CHANNEL ADC1_CHANNEL_6 // GPIO34

void mic_init(uint32_t sampleRate)
{
    g_sampleRate = sampleRate;

    // I2S config: use built-in ADC via I2S
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_ADC_BUILT_IN),
        .sample_rate = (int)g_sampleRate,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT, // mono
        .communication_format = (i2s_comm_format_t)I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 4,
        .dma_buf_len = 512,
        .use_apll = false};

    i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
    i2s_set_adc_mode(ADC_INPUT_UNIT, ADC_INPUT_CHANNEL);
    i2s_adc_enable(I2S_PORT);

    // small warm-up delay
    delay(50);
}

// Read exactly bufSize bytes (blocks until read). Returns bytes read.
size_t mic_get_frame(uint8_t *outBuf, size_t bufSize)
{
    size_t bytesRead = 0;
    size_t toRead = bufSize;
    // i2s_read accepts pointer and returns bytes read
    esp_err_t ret = i2s_read(I2S_PORT, (void *)outBuf, toRead, &bytesRead, portMAX_DELAY);
    if (ret != ESP_OK)
    {
        Serial.printf("i2s_read err %d\n", ret);
        return 0;
    }
    // i2s ADC might return 16-bit samples left aligned into 16 bits. We return raw bytes as-is.
    return bytesRead;
}
