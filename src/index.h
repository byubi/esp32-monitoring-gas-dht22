#ifndef INDEX_H
#define INDEX_H

const char HTML_CONTENT[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="id">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 Safety Dashboard</title>
    <style>
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background-color: #121212;
            color: #e0e0e0;
            margin: 0;
            padding: 20px;
            display: flex;
            flex-direction: column;
            align-items: center;
        }
        h2 {
            color: #00bcd4;
            margin-bottom: 5px;
        }
        p.subtitle {
            color: #888;
            margin-top: 0;
            margin-bottom: 30px;
        }
        .container {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(280px, 1fr));
            gap: 20px;
            width: 100%;
            max-width: 800px;
        }
        .card {
            background-color: #1e1e1e;
            border-radius: 12px;
            padding: 20px;
            box-shadow: 0 4px 10px rgba(0,0,0,0.5);
            text-align: center;
            border: 1px solid #333;
        }
        .card h3 {
            margin-top: 0;
            color: #b0bec5;
        }
        .value {
            font-size: 2rem;
            font-weight: bold;
            margin: 15px 0;
            color: #ffffff;
        }
        .status-card {
            grid-column: 1 / -1;
            background-color: #1e1e1e;
            border-radius: 12px;
            padding: 20px;
            box-shadow: 0 4px 10px rgba(0,0,0,0.5);
            text-align: center;
            border: 1px solid #333;
        }
        #statusBox {
            padding: 15px;
            border-radius: 8px;
            font-weight: bold;
            font-size: 1.1rem;
            color: #fff;
            background-color: #449d44;
            transition: background-color 0.3s ease;
        }
    </style>
</head>
<body>

    <h2>Safety Dashboard</h2>
    <p class="subtitle">ESP32 Integrated Safety System</p>

    <div class="container">
        <div class="card">
            <h3>Suhu & Kelembapan</h3>
            <div class="value" id="tempVal">-- °C</div>
            <div style="font-size: 1.1rem; color: #90caf9;" id="humVal">Kelembapan: -- %</div>
        </div>

        <div class="card">
            <h3>Sensor Gas (MQ-2)</h3>
            <div class="value" id="gasVal">--</div>
            <div style="font-size: 1.0rem; color: #b0bec5;">Indeks Kerapatan Gas</div>
        </div>

        <div class="status-card">
            <h3>Status Lingkungan</h3>
            <div id="statusBox">Memuat data sensor...</div>
        </div>
    </div>

    <script>
        setInterval(function() {
            fetch('/data')
                .then(response => response.json())
                .then(data => {
                    document.getElementById('tempVal').innerText = data.temp.toFixed(1) + " °C";
                    document.getElementById('humVal').innerText = "Kelembapan: " + data.hum.toFixed(1) + " %";
                    document.getElementById('gasVal').innerText = data.gas;

                    const statusBox = document.getElementById('statusBox');
                    statusBox.innerText = data.message;

                    if (data.temp_danger || data.hum_danger || data.gas_danger) {
                        statusBox.style.backgroundColor = '#d9534f'; // Merah (Bahaya)
                    } else {
                        statusBox.style.backgroundColor = '#449d44'; // Hijau (Aman)
                    }
                })
                .catch(error => console.log('Error fetching data:', error));
        }, 2000);
    </script>

</body>
</html>
)rawliteral";

#endif