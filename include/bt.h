#pragma once
#include <Arduino.h>

// Initialize SPP server with name shown to phone
void bt_init(const char *deviceName);

// Send a binary audio frame. This prefixes length internally and writes to SPP.
bool bt_send_frame(const uint8_t *data, uint16_t len);

// Non-blocking poll for text received. If available, fills out String and returns true.
bool bt_poll_receive_text(String &outText);
