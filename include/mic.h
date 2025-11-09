#pragma once
#include <Arduino.h>

void mic_init(uint32_t sampleRate);

size_t mic_get_frame(uint8_t *outBuf, size_t bufSize);
