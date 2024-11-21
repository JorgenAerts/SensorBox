
/*
Code for DHT sensor adapted from: https://wiki.dfrobot.com/DHT22_Temperature_and_humidity_module_SKU_SEN0137 
Code for Async Webserver adapted from: https://randomnerdtutorials.com/esp32-async-web-server-espasyncwebserver-library/
Code correction and Webserver html by ChatGPT:  ChatGPT. GPT-4. OpenAI, 2023. https://openai.com/chatgpt.

*/




#include <Arduino.h>
#include <DHT.h>
#include <Adafruit_Sensor.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ElegantOTA.h>
#include <WiFi.h>
#include <secrets.h>

// LEDs
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

// Button
#define buttonRead 19
unsigned long previousMillisButton = 0;
const long intervalButton = 300;

// WebServer
AsyncWebServer server(80);

void setup()
{
    Serial.begin(115200);
    Serial.println("Serial monitor started");
    dht.begin();

    // Initialize input/output pins
    pinMode(ledRedLum, OUTPUT);
    pinMode(ledRedTemp, OUTPUT);
    pinMode(ledRedHum, OUTPUT);
    pinMode(ledGreenLum, OUTPUT);
    pinMode(ledGreenTemp, OUTPUT);
    pinMode(ledGreenHum, OUTPUT);

    // Connect to Wi-Fi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.println("Connecting to WiFi..");
    }
    Serial.println("Connected to WiFi");
    Serial.print("ESP32 IP Address: ");
    Serial.println(WiFi.localIP()); // Print the IP address

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

    ElegantOTA.begin(&server);
    server.begin();
}

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

void readDht()
{
    unsigned long currentMillisDht = millis();

    if (currentMillisDht - previousMillisDht >= intervalDht)
    {
        hum = dht.readHumidity();
        temp = dht.readTemperature();

        if (isnan(hum) || isnan(temp))
        {
            Serial.println("Failed to read from DHT sensor!");
            return;
        }

        Serial.print("Humidity: ");
        Serial.print(hum);
        Serial.print(" %, Temperature: ");
        Serial.println(temp);
        previousMillisDht = currentMillisDht;
    }
}

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

void readButton()
{
    unsigned long currentMillisButton = millis();
    if (currentMillisButton - previousMillisButton >= intervalButton)
    {
        if (digitalRead(buttonRead) == HIGH)
        {
            Serial.print("Humidity: ");
            Serial.print(hum);
            Serial.print(" %, Temp: ");
            Serial.print(temp);
            Serial.println(" Celsius");

            previousMillisButton = currentMillisButton;
        }
    }
}

void loop()
{
    readDht();
    readLdr();
    readButton();
    indicateStatus();
}
