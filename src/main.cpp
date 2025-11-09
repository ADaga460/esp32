#include <Arduino.h>
#include "mic.h"
#include "bt.h"
#include "display.h"

// FRAME definition: 20 ms at 16kHz, 16-bit mono -> 320 samples -> 640 bytes
#define SAMPLE_RATE 16000
#define FRAME_MS 20
#define SAMPLES_PER_FRAME ((SAMPLE_RATE * FRAME_MS) / 1000)
#define FRAME_BYTES (SAMPLES_PER_FRAME * 2) // 16-bit samples

static uint8_t pcmFrame[FRAME_BYTES];

void setup()
{
    Serial.begin(115200);
    delay(1000);
    Serial.println("ESP32 Assistive Glasses - firmware boot");

    mic_init(SAMPLE_RATE);
    bt_init("ESP32_SPP_ASSIST");
    display_init();

    Serial.println("Initialization complete.");
}

void loop()
{
    // 1) Capture one PCM frame (blocking until full frame)
    size_t got = mic_get_frame(pcmFrame, FRAME_BYTES);
    if (got == FRAME_BYTES)
    {
        // 2) Send frame over SPP (prefix length 2 bytes little-endian)
        //bt_send_frame(pcmFrame, FRAME_BYTES);
        Serial.printf("mic frame sent: %u bytes\n", (unsigned)got);
    }
    else
    {
        // partial read - drop or handle; we drop to keep deterministic timing
        Serial.printf("mic frame short: %u\n", (unsigned)got);
    }

    // 3) Check for incoming text from phone
    String incoming;
    if (bt_poll_receive_text(incoming))
    {
        Serial.print("RX text: ");
        Serial.println(incoming);
        //display_render_text(incoming.c_str());
    }

    // DEBUG TEST MODE
    static uint32_t lastTest = 0;
    if (millis() - lastTest > 2000)
    {
        lastTest = millis();
        display_render_text("TEST MODE\nESP32 OK");
        Serial.println("TEST: OLED render OK");
    }

    // Small yield to allow BT stack processing
    delay(2);
}
