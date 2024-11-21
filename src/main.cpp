
/*
Code for DHT sensor adapted from: https://wiki.dfrobot.com/DHT22_Temperature_and_humidity_module_SKU_SEN0137 19-11-2024
Code for Async Webserver adapted from: https://randomnerdtutorials.com/esp32-async-web-server-espasyncwebserver-library/ 19-11-2024
Code for OTA adapted from : https://randomnerdtutorials.com/esp32-ota-over-the-air-arduino/ 19-11-2024
Code for MQTT adapted from a ChatGPT query for the MQTT excercises: ChatGPT; GPT-4. OpenAI, 2023. https://openai.com/chatgpt 19-11-2024
Code correction and Webserver html by ChatGPT:  ChatGPT. GPT-4. OpenAI, 2023. https://openai.com/chatgpt.  19-11-2024
Code correction remote button: Claude Ai; Anthropic AI https://claude.ai 21-11-2024
*/

// Libraries
#include <Arduino.h>
#include <DHT.h>
#include <Adafruit_Sensor.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ElegantOTA.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <secrets.h>

// LED's
#define ledRedLum 25
#define ledRedTemp 26
#define ledRedHum 14
#define ledGreenLum 13
#define ledGreenTemp 21
#define ledGreenHum 22

// Lightsensor
#define ldrSensor 36
int lum = 0;
unsigned long previousMillisLdr = 0;
const long intervalLdr = 5000;

// DHT 22 sensor
#define DHTPIN 16
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
float hum;
float temp;
unsigned long previousMillisDht = 0;
const long intervalDht = 5000;

// LED test button
#define button 19
unsigned long previousMillisTestActive = 0;
const long testActive = 2000;
bool ledTestActive = false;

// WebServer
AsyncWebServer server(80);

// MQTT
WiFiClient espClient;
PubSubClient client(espClient);
const char *mqtt_server = "10.6.121.204"; //  IP address of the MQTT broker
const int mqtt_port = 1883;               // broker port (default is 1883)
// MQTT Topics
const char *temperatureTopic = "home/sensor/temperature";
const char *humidityTopic = "home/sensor/humidity";
const char *luminosityTopic = "home/sensor/luminosity";

// Connect to Wi-Fi
void connectWifi()
{
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.println("Connecting to WiFi..");
    }
    Serial.println("Connected to WiFi");
    Serial.print("ESP32 IP Address: ");
    Serial.println(WiFi.localIP()); // Print the IP address
}

// Connect to the MQTT broker
void connectToMQTT()
{
    while (client.connected() == false)
    {
        Serial.println("Connecting to MQTT...");
        if (client.connect("ESP32Client"))
        {
            Serial.println("Connected to MQTT");
            client.subscribe("home/testLEDs");
        }
        else
        {
            Serial.print("Failed, rc=");
            Serial.print(client.state());
            delay(5000);
        }
    }
}

// Tests all status LED's, lighting them up for 2 seconds
void testLEDs()
{
    bool buttonState = digitalRead(button); //  Read the button state

    if (buttonState == HIGH && ledTestActive == false) //  If the button is pressed down (HIGH) and LED's are not already turned on
    {
        digitalWrite(ledRedLum, HIGH); //  Turn on all LED's
        digitalWrite(ledRedTemp, HIGH);
        digitalWrite(ledRedHum, HIGH);
        digitalWrite(ledGreenLum, HIGH);
        digitalWrite(ledGreenTemp, HIGH);
        digitalWrite(ledGreenHum, HIGH);

        previousMillisTestActive = millis(); // Store the current time to track the 2-second duration
        ledTestActive = true;                // Set the flag to indicate that LED's are on
    }

    if (ledTestActive && millis() - previousMillisTestActive >= testActive) //  If the LED's have been on for 2 seconds or more, turn them off
    {

        digitalWrite(ledRedLum, LOW); // Turn off all LED's
        digitalWrite(ledRedTemp, LOW);
        digitalWrite(ledRedHum, LOW);
        digitalWrite(ledGreenLum, LOW);
        digitalWrite(ledGreenTemp, LOW);
        digitalWrite(ledGreenHum, LOW);

        // Reset the flag to indicate that LEDs are off
        ledTestActive = false;
    }
}


void testLEDsRemotely()
{
    if (ledTestActive == false) // If the LEDs are not already on
    {
        // Turn on all LEDs
        digitalWrite(ledRedLum, HIGH);
        digitalWrite(ledRedTemp, HIGH);
        digitalWrite(ledRedHum, HIGH);
        digitalWrite(ledGreenLum, HIGH);
        digitalWrite(ledGreenTemp, HIGH);
        digitalWrite(ledGreenHum, HIGH);

        previousMillisTestActive = millis();  // Start the 2-second timer
        ledTestActive = true;          // Set the flag to indicate LEDs are on
    }

    // Check if 2 seconds have passed
    if (ledTestActive && millis() - previousMillisTestActive >= testActive)
    {
        // Turn off all LEDs after 2 seconds
        digitalWrite(ledRedLum, LOW);
        digitalWrite(ledRedTemp, LOW);
        digitalWrite(ledRedHum, LOW);
        digitalWrite(ledGreenLum, LOW);
        digitalWrite(ledGreenTemp, LOW);
        digitalWrite(ledGreenHum, LOW);

        ledTestActive = false;  // Reset the flag
    }
}

// Callback function to handle incoming MQTT messages
void callback(char *topic, byte *payload, unsigned int length)
{
    String message = "";
    for (int i = 0; i < length; i++)
    {
        message += (char)payload[i];
    }

    Serial.print("Message arrived on topic: ");
    Serial.print(topic);
    Serial.print(". Message: ");
    Serial.println(message);

    // Check for the test command from Node-RED
    if (String(topic) == "home/testLEDs" && message == "test")
    {
        // Trigger LED test via MQTT
        testLEDsRemotely();
    }
}

// Read the lightsensor with 5 second intervals
void readLdr()
{
    unsigned long currentMillisLdr = millis();

    if (currentMillisLdr - previousMillisLdr >= intervalLdr)
    {
        lum = analogRead(ldrSensor);
        Serial.print("LDR waarde: ");
        Serial.println(lum);
        previousMillisLdr = currentMillisLdr;
    }
}

//  Reads the humidity and temeprature with 5 second intervals
void readDht()
{
    unsigned long currentMillisDht = millis();

    if (currentMillisDht - previousMillisDht >= intervalDht)
    {
        hum = dht.readHumidity();
        temp = dht.readTemperature();

        if (isnan(hum) || isnan(temp))
        {
            Serial.println("Failed to read from DHT sensor!"); // Error message
            return;
        }

        Serial.print("Humidity: ");
        Serial.print(hum);
        Serial.print(" %, Temperature: ");
        Serial.println(temp);
        previousMillisDht = currentMillisDht;
    }
}

// Runs the status LED's
void indicateStatus()
{
    // Humidity indicator
    if (hum >= 85) // Optimal humidity is above 85 percent
    {
        digitalWrite(ledGreenHum, HIGH);
        digitalWrite(ledRedHum, LOW); // Turn off the red LED when green is ON
    }
    else
    {
        digitalWrite(ledGreenHum, LOW); // Turn off the green LED when red is ON
        digitalWrite(ledRedHum, HIGH);
    }

    // Temperature indicator
    if (temp < 19 || temp > 23) // Optimal temperature for mycelium growth is 21°C
    // if (temp < 12 && temp > 16) // Optimal temperature for mushroom growth is 13 to 15°C
    {
        digitalWrite(ledRedTemp, HIGH);
        digitalWrite(ledGreenTemp, LOW);
    }
    else
    {
        digitalWrite(ledGreenTemp, HIGH);
        digitalWrite(ledRedTemp, LOW);
    }

    // Light intensity indicator
    if (lum <= 1500)
    {
        digitalWrite(ledGreenLum, HIGH);
        digitalWrite(ledRedLum, LOW);
    }
    else
    {
        digitalWrite(ledGreenLum, LOW);
        digitalWrite(ledRedLum, HIGH);
    }
}

// Publish sensor values to MQTT broker
void publishSensorData()
{
    // Publish temperature, humidity, and luminosity to MQTT topics
    if (client.connected())
    {
        client.publish(temperatureTopic, String(temp).c_str());
        client.publish(humidityTopic, String(hum).c_str());
        client.publish(luminosityTopic, String(lum).c_str());
        // Serial.println("Sensor data published to MQTT broker."); // for debugging
    }
}

void setup()
{
    Serial.begin(115200);
    Serial.println("Serial monitor started");
    dht.begin(); // start the temperature and humidity sensor

    // Initialize input/output pins
    pinMode(ledRedLum, OUTPUT);
    pinMode(ledRedTemp, OUTPUT);
    pinMode(ledRedHum, OUTPUT);
    pinMode(ledGreenLum, OUTPUT);
    pinMode(ledGreenTemp, OUTPUT);
    pinMode(ledGreenHum, OUTPUT);

    // Connect to wifi
    connectWifi();

    // Connect to the MQTT Broker
    client.setServer(mqtt_server, 1883); // Set the broker's IP and port
    client.setCallback(callback);
    connectToMQTT();                     // Establish connection to MQTT broker

    // Serve the HTML page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        String html = "<!DOCTYPE html><html lang='en'><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1.0'><title>Sensor Data</title><style>body{font-family:Arial,sans-serif;text-align:center;padding-top:50px;}.sensor-box{display:inline-block;margin:20px;padding:20px;border:1px solid #ccc;border-radius:8px;box-shadow:0 0 10px rgba(0,0,0,0.1);}h2{font-size:1.5em;margin-bottom:10px;}p{font-size:1.2em;}</style></head><body><h1>Sensor Data Display</h1><div class='sensor-box'><h2>Temperature</h2><p id='temperature'>Loading...</p></div><div class='sensor-box'><h2>Humidity</h2><p id='humidity'>Loading...</p></div><div class='sensor-box'><h2>Luminosity</h2><p id='luminosity'>Loading...</p></div><script>function fetchSensorData(){fetch('/sensor').then(response=>response.json()).then(data=>{document.getElementById('temperature').textContent=data.temperature+' °C';document.getElementById('humidity').textContent=data.humidity+' %';document.getElementById('luminosity').textContent=data.luminosity;}).catch(error=>console.error('Error fetching sensor data:', error));}setInterval(fetchSensorData, 5000);fetchSensorData();</script></body></html>";
        request->send(200, "text/html", html); });

    // Serve the sensor data as JSON
    server.on("/sensor", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        String jsonData = "{\"temperature\": " + String(temp) + ", \"humidity\": " + String(hum) + ", \"luminosity\": " + String(lum) + "}";
        request->send(200, "application/json", jsonData); });
    
    // Start server for over the air updates 
    ElegantOTA.begin(&server);
    server.begin();
}

void loop()
{
    readDht();
    readLdr();
    client.loop(); // Ensure MQTT client processes messages
    publishSensorData();
    testLEDs();
    if (ledTestActive == false)
    { // This suspends this function while the LED's are being tested.(Or rather, this function only runs while the LED's are not being tested)
        indicateStatus();
    }
}
