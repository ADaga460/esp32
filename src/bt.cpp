#include "bt.h"
#include "BluetoothSerial.h"

static BluetoothSerial SerialBT;

// RX parsing state
static enum { RX_WAIT_LEN,
              RX_READ_PAYLOAD } rxState = RX_WAIT_LEN;
static uint16_t rxExpected = 0;
static std::vector<uint8_t> rxBuffer;

void bt_init(const char *deviceName)
{
    if (!SerialBT.begin(deviceName))
    {
        Serial.println("SerialBT.begin failed");
    }
    else
    {
        Serial.printf("SerialBT started as '%s'\n", deviceName);
    }
    rxBuffer.reserve(512);
}

// helper: write 2-byte little-endian length
static void writeLengthPrefix(uint16_t len)
{
    uint8_t b[2];
    b[0] = len & 0xff;
    b[1] = (len >> 8) & 0xff;
    SerialBT.write(b, 2);
}

bool bt_send_frame(const uint8_t *data, uint16_t len)
{
    if (!SerialBT.hasClient())
    {
        // no connected client
        return false;
    }

    // Send length prefix then data
    writeLengthPrefix(len);
    size_t w = SerialBT.write(data, len);
    SerialBT.flush();
    return (w == len);
}

// internal: read available bytes into rxBuffer and parse messages
static void bt_poll_internal()
{
    while (SerialBT.available())
    {
        int c = SerialBT.read();
        if (c < 0)
            break;
        rxBuffer.push_back((uint8_t)c);

        // parse state machine
        while (true)
        {
            if (rxState == RX_WAIT_LEN)
            {
                if (rxBuffer.size() >= 2)
                {
                    rxExpected = (uint16_t)rxBuffer[0] | ((uint16_t)rxBuffer[1] << 8);
                    // consume the two length bytes
                    rxBuffer.erase(rxBuffer.begin(), rxBuffer.begin() + 2);
                    rxState = RX_READ_PAYLOAD;
                }
                else
                {
                    break;
                }
            }
            if (rxState == RX_READ_PAYLOAD)
            {
                if (rxBuffer.size() >= rxExpected)
                {
                    // full payload available -> convert to String and store in queue (we will return immediately on poll)
                    // For simplicity we keep a single last message; if multiple complete messages exist, last one wins.
                    // Create a null-terminated string
                    std::string s((char *)rxBuffer.data(), rxExpected);
                    // store into a dedicated place by overwriting rxBuffer to remaining bytes after payload
                    std::vector<uint8_t> remainder;
                    if (rxBuffer.size() > rxExpected)
                    {
                        remainder.assign(rxBuffer.begin() + rxExpected, rxBuffer.end());
                    }
                    rxBuffer.swap(remainder);
                    // we want to expose this payload externally. Use a static holder by pushing into a global queue.
                    // Simple approach: write lastReceivedText to a static String variable.
                    static String lastReceivedText;
                    lastReceivedText = String(s.c_str());
                    // Expose by copying into a globally accessible variable via function
                    // We'll use a small trick: store message into Serial's software buffer by printing with special marker.
                    // Instead, keep lastReceivedText in a static and provide a getter below.
                    // To keep threads simple, store in SerialBT's "properties": use a static variable here.
                    // We'll set a static flag and string that bt_poll_receive_text reads.
                    extern void bt_internal_store_message(const String &);
                    bt_internal_store_message(lastReceivedText);
                    rxState = RX_WAIT_LEN;
                }
                else
                {
                    break;
                }
            }
        } // inner while
    } // outer while
}

// storage for last message
static bool has_stored_msg = false;
static String stored_msg = "";

void bt_internal_store_message(const String &s)
{
    stored_msg = s;
    has_stored_msg = true;
}

bool bt_poll_receive_text(String &outText)
{
    // poll to fetch bytes and parse
    bt_poll_internal();

    if (has_stored_msg)
    {
        outText = stored_msg;
        has_stored_msg = false;
        stored_msg = "";
        return true;
    }
    return false;
}
