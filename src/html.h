#ifndef HTML_H
#define HTML_H

const char html_content[] = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Sensor Data</title>
    <style>
        /* Base Styling */
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background-color: #f9f9f9;
            margin: 0;
            padding: 0;
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            min-height: 100vh;
        }
        h1 {
            font-size: 2.5em;
            color: #333;
            margin-bottom: 20px;
        }

        /* Sensor Box Styling */
        .sensor-box {
            display: inline-block;
            margin: 20px;
            padding: 20px 30px;
            background-color: #fff;
            border: 1px solid #e0e0e0;
            border-radius: 10px;
            box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1);
            transition: transform 0.2s, box-shadow 0.2s;
        }
        .sensor-box:hover {
            transform: translateY(-5px);
            box-shadow: 0 6px 12px rgba(0, 0, 0, 0.15);
        }
        h2 {
            font-size: 1.8em;
            color: #444;
            margin-bottom: 10px;
        }
        p {
            font-size: 1.4em;
            color: #666;
            margin: 0;
        }

        /* Responsive Design */
        @media (max-width: 768px) {
            .sensor-box {
                margin: 15px;
                padding: 15px 20px;
            }
            h2 {
                font-size: 1.6em;
            }
            p {
                font-size: 1.2em;
            }
        }
    </style>
</head>
<body>
    <h1>Sensor Data Display</h1>
    <div class="sensor-box">
        <h2>Temperature</h2>
        <p id="temperature">Loading...</p>
    </div>
    <div class="sensor-box">
        <h2>Humidity</h2>
        <p id="humidity">Loading...</p>
    </div>
    <div class="sensor-box">
        <h2>Luminosity</h2>
        <p id="luminosity">Loading...</p>
    </div>
    <script>
        function fetchSensorData() {
            fetch('/sensor')
                .then(response => response.json())
                .then(data => {
                    document.getElementById('temperature').textContent = data.temperature + ' °C';
                    document.getElementById('humidity').textContent = data.humidity + ' %';
                    document.getElementById('luminosity').textContent = data.luminosity;
                })
                .catch(error => console.error('Error fetching sensor data:', error));
        }
        setInterval(fetchSensorData, 5000);
        fetchSensorData();
    </script>
</body>
</html>
)rawliteral";

#endif // HTML_H
