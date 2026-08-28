#ifndef INDEX_H
#define INDEX_H

#include <Arduino.h>

const char HTML_CONTENT[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="id">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 Safety Dashboard</title>
    <style>
        * { box-sizing: border-box; margin: 0; padding: 0; }
        body { 
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; 
            background-color: #121212; 
            color: #ffffff; 
            text-align: center; 
            padding: 20px; 
        }
        h1 { color: #00adb5; margin-bottom: 5px; }
        p.subtitle { color: #888; font-size: 0.9rem; margin-bottom: 20px; }
        
        .container {
            display: flex;
            flex-wrap: wrap;
            justify-content: center;
            gap: 15px;
            max-width: 600px;
            margin: 0 auto;
        }
        
        .card { 
            background: #1e1e1e; 
            border-radius: 12px; 
            padding: 20px; 
            flex: 1 1 200px;
            box-shadow: 0 4px 10px rgba(0,0,0,0.5); 
            border: 1px solid #333;
        }
        
        .card h3 { color: #aaa; font-size: 1rem; margin-bottom: 10px; }
        .value { font-size: 2.2rem; font-weight: bold; color: #00adb5; margin: 10px 0; }
        
        .status-box { 
            padding: 12px; 
            border-radius: 8px; 
            font-weight: bold; 
            font-size: 1.1rem;
            margin-top: 10px; 
            transition: background-color 0.3s;
        }
        .safe { background-color: #2e7d32; color: #fff; }
        .danger { background-color: #c62828; color: #fff; animation: pulse 1s infinite; }

        @keyframes pulse {
            0% { opacity: 1; }
            50% { opacity: 0.5; }
            100% { opacity: 1; }
        }
    </style>
    <script>
        setInterval(function() {
            fetch('/data')
                .then(response => response.json())
                .then(data => {
                    document.getElementById("temp").innerText = data.temp + " °C";
                    document.getElementById("hum").innerText = data.hum + " %";
                    document.getElementById("gas").innerText = data.gas;
                    
                    let statusElem = document.getElementById("status");
                    // Update: Menambahkan data.hum_danger agar kelembapan ekstrem juga memicu alert merah
                    if (data.gas_danger || data.temp_danger || data.hum_danger) {
                        statusElem.innerText = "⚠️ BAHAYA TERDETEKSI!";
                        statusElem.className = "status-box danger";
                    } else {
                        statusElem.innerText = "✅ KONDISI AMAN";
                        statusElem.className = "status-box safe";
                    }
                })
                .catch(err => console.error("Gagal mengambil data:", err));
        }, 2000);
    </script>
</head>
<body>
    <h1>Safety Dashboard</h1>
    <p class="subtitle">ESP32 Integrated Safety System</p>
    
    <div class="container">
        <div class="card">
            <h3>Suhu & Kelembapan</h3>
            <div class="value" id="temp">-- °C</div>
            <div style="color:#aaa;">Kelembapan: <span id="hum" style="color:#fff;">-- %</span></div>
        </div>
        
        <div class="card">
            <h3>Sensor Gas (MQ-2)</h3>
            <div class="value" id="gas">--</div>
            <div style="color:#aaa;">Indeks Kerapatan Gas</div>
        </div>
    </div>
    
    <div class="container" style="margin-top: 15px;">
        <div class="card" style="flex: 1 1 100%;">
            <h3>Status Lingkungan</h3>
            <div id="status" class="status-box safe">Membaca Data...</div>
        </div>
    </div>
</body>
</html>
)rawliteral";

#endif