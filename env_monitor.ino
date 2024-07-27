#include <WaziDev.h>
#include <xlpp.h>
#include <DHT.h>

// Pin definitions
#define DHT_PIN 5           // DHT digital pin
#define LIGHT_SENSOR_PIN A0 // Light sensor analog pin
#define BUZZER_PIN 4        // Buzzer pin
#define LED_PIN 10            // LED pin
#define RELAY_PIN 7         // Relay pin
#define XLPP_TYPE_TEMPERATURE 0x67
#define XLPP_TYPE_HUMIDITY 0x68
#define XLPP_TYPE_ANALOG_INPUT 0x71

// LoRaWAN keys and addresses
const uint8_t devAddr[4] = {0xBD, 0x80, 0xBB, 0x5E};
const uint8_t nwkSkey[16] = {0xBD, 0x80, 0xBB, 0x5E, 0x86, 0x0E, 0xCC, 0x44, 0xE2, 0x82, 0xF3, 0x5F, 0x45, 0x5A, 0x18, 0x18}; //device's NwkSKey
const uint8_t appSkey[16] = {0xBD, 0x80, 0xBB, 0x5E, 0x86, 0x0E, 0xCC, 0x44, 0xE2, 0x82, 0xF3, 0x5F, 0x45, 0x5A, 0x18, 0x18}; // device's AppKey

WaziDev wazidev;
DHT dht(DHT_PIN, DHT11);

// Calibration values for the light sensor
int LIGHT_THRESHOLD = 500; // Arbitrary value

// Calibration values for the MQ-2 sensor
int MQ2_THRESHOLD = 400; // Arbitrary value
float TEMP_THRESHOLD = 34;
float HUMIDITY_THRESHOLD = 40;

float readLightSensorValue()
{
    int sensorValue = analogRead(LIGHT_SENSOR_PIN);
    return sensorValue;
}

float readMQ2Value()
{
    int sensorValue = analogRead(A0);
    return sensorValue;
}

void setup()
{
    Serial.begin(9600);
    wazidev.setupLoRaWAN(devAddr, appSkey, nwkSkey);

    dht.begin(); // start DHT sensor
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(LED_PIN, OUTPUT);
    pinMode(RELAY_PIN, OUTPUT);
    delay(2000);
}

XLPP xlpp(120);

uint8_t uplink()
{
    uint8_t e;

    // 1. Read sensor values.
    float humidity = dht.readHumidity();           // %
    float temperature = dht.readTemperature();     // °C
    int lightSensorValue = readLightSensorValue(); // Light sensor value
    int mq2SensorValue = readMQ2Value();         // MQ-2 sensor value

    // 2. Create xlpp payload for uplink.
    xlpp.reset();

    // Add sensor payload
    xlpp.addTemperature(0, temperature);
    xlpp.addRelativeHumidity(1, humidity);
    xlpp.addAnalogInput(2, lightSensorValue); // Add light sensor value
    xlpp.addAnalogInput(3, mq2SensorValue);   // Add MQ-2 sensor value

    Serial.print("Temp: ");
    Serial.print(temperature);
    Serial.print("C, Humid: ");
    Serial.print(humidity);
    Serial.print("%, Light: ");
    Serial.print(lightSensorValue);
    Serial.print(", MQ-2: ");
    Serial.print(mq2SensorValue);
    Serial.println(".");

    // 3. Send payload uplink with LoRaWAN.
    serialPrintf("LoRaWAN send ... ");
    e = wazidev.sendLoRaWAN(xlpp.buf, xlpp.len);
    if (e != 0)
    {
        serialPrintf("Err %d\n", e);
        return e;
    }
    serialPrintf("OK\n");

    // 4. Check and handle sensor data
    handleSensorData(temperature, humidity, lightSensorValue, mq2SensorValue);

    return 0;
}

void handleSensorData(float temperature, float humidity, int lightSensorValue, int mq2SensorValue)
{
    // Temperature and humidity monitoring
    if (temperature > TEMP_THRESHOLD || humidity > HUMIDITY_THRESHOLD)
    {
        // Temperature or humidity threshold exceeded, trigger alert
        triggerAlert();
    }

    // Lighting control
    if (lightSensorValue < LIGHT_THRESHOLD)
    {
        // Low light level, turn on LED and relay
        digitalWrite(LED_PIN, HIGH);
        digitalWrite(RELAY_PIN, HIGH); // Turn on relay to control lamp
    }
    else
    {
        // Sufficient light level, turn off LED and relay
        digitalWrite(LED_PIN, LOW);
        digitalWrite(RELAY_PIN, LOW); // Turn off relay to control lamp
    }

    // MQ-2 gas detection
    if (mq2SensorValue > MQ2_THRESHOLD)
    {
        // Gas concentration threshold exceeded, turn on LED and buzzer
        digitalWrite(LED_PIN, HIGH);
        tone(BUZZER_PIN, 1000); // Turn on buzzer with 1kHz tone
        Serial.println("LED and buzzer are ON");
    }
    else
    {
        digitalWrite(LED_PIN, LOW);
        noTone(BUZZER_PIN); // Turn off buzzer
        Serial.println("LED and buzzer are OFF");
    }
}

void triggerAlert()
{
    // Trigger buzzer to alert the user
    tone(BUZZER_PIN, 1000); // Turn on buzzer with 1kHz tone
    delay(5000);            // Alert for 5 seconds
    noTone(BUZZER_PIN);     // Turn off buzzer
}

uint8_t downlink(uint16_t timeout)
{
    uint8_t e;

    // 1. Receive LoRaWAN downlink message.
    serialPrintf("LoRa receive ... ");
    uint8_t offs = 0;
    long startSend = millis();
    e = wazidev.receiveLoRaWAN(xlpp.buf, &xlpp.offset, &xlpp.len, timeout);
    long endSend = millis();
    if (e)
    {
        if (e == ERR_LORA_TIMEOUT)
            serialPrintf("nothing received\n");
        else
            serialPrintf("Err %d\n", e);
        return e;
    }
    serialPrintf("OK\n");

    serialPrintf("Time On Air: %d ms\n", endSend - startSend);
    serialPrintf("LoRa SNR: %d\n", wazidev.loRaSNR);
    serialPrintf("LoRa RSSI: %d\n", wazidev.loRaRSSI);
    serialPrintf("LoRaWAN frame size: %d\n", xlpp.offset + xlpp.len);
    serialPrintf("LoRaWAN payload len: %d\n", xlpp.len);
    serialPrintf("Payload: ");
    if (xlpp.len == 0)
    {
        serialPrintf("(no payload received)\n");
        return 1;
    }
    printBase64(xlpp.getBuffer(), xlpp.len);
    serialPrintf("\n");

    // 2. Read xlpp payload from downlink message.
    int end = xlpp.len + xlpp.offset;
    while (xlpp.offset < end)
    {
        // [1] Always read the channel first ...
        uint8_t chan = xlpp.getChannel();
        serialPrintf("Chan %2d: ", chan);

        // [2] ... then the type ...
        uint8_t type = xlpp.getType();
        switch (type)
        {
        case XLPP_TYPE_TEMPERATURE:
            float tempThreshold = xlpp.getTemperature();
            if (tempThreshold != 0.0)
            {
                TEMP_THRESHOLD = tempThreshold;
                serialPrintf("Temperature threshold updated to %.2f°C\n", tempThreshold);
            }
            break;
        case XLPP_TYPE_HUMIDITY:
            float humidityThreshold = xlpp.getRelativeHumidity();
            if (humidityThreshold != 0.0)
            {
                HUMIDITY_THRESHOLD = humidityThreshold;
                serialPrintf("Humidity threshold updated to %.2f%%\n", humidityThreshold);
            }
            break;
        case XLPP_TYPE_ANALOG_INPUT:
            int lightThreshold = xlpp.getAnalogInput();
            if (lightThreshold != 0)
            {
                LIGHT_THRESHOLD = lightThreshold;
                serialPrintf("Light threshold updated to %d\n", lightThreshold);
            }
            break;
        default:
            serialPrintf("Unknown type %d\n", type);
            break;
        }
    }

    return 0;
}

void loop(void)
{
    // error indicator
    uint8_t e;

    // 1. LoRaWAN Uplink
    e = uplink();
    // if no error...
    if (!e)
    {
        // 2. LoRaWAN Downlink
        // waiting for 6 seconds only!
        downlink(6000);
    }

    serialPrintf("Waiting 5 seconds ...\n");
    delay(5000); // Wait for 5 seconds before the next cycle
}
