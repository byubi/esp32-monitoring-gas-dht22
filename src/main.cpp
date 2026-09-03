#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include "index.h"

// ================= STANDAR LOGIKA ACTIVE-HIGH =================
#define LED_ON  HIGH  
#define LED_OFF LOW   

// ================= KONFIGURASI WI-FI & TELEGRAM =================
const char* ssid     = "Raspberry R7";      
const char* password = "wlanebc417";  

#define BOT_TOKEN    "8655285800:AAGUqsqs2eO-aJusp84Grln9ie-wDp0T6a0" 
#define CHANNEL_ID "-1003701371588" 

// ================= PIN MAPPING ARDUTECH ESP32 V4 =================
#define DHTPIN        15    
#define DHTTYPE       DHT11
#define MQ2_ANALOG_PIN 34   

#define LED1_PIN      19    
#define LED2_PIN      2     
#define LED3_PIN      5     
#define LED4_PIN      4     

#define BUZZER_PIN    13    
#define BUTTON_S1_PIN 12    
#define BUTTON_S2_PIN 14    

// ================= AMBANG BATAS RENTANG (THRESHOLD) =================
const float TEMP_MIN_THRESHOLD = 10.0; 
const float TEMP_MAX_THRESHOLD = 33.0; 
const float HUM_MIN_THRESHOLD  = 30.0; 
const float HUM_MAX_THRESHOLD  = 90.0; 
const int   GAS_THRESHOLD      = 1000; 

const unsigned long REPORT_INTERVAL = 60000;       // Broadcast Laporan 1 menit
const unsigned long TELEGRAM_CHECK_INTERVAL = 3000; // Cek Telegram Tiap 3 Detik

DHT dht(DHTPIN, DHTTYPE);
WebServer server(80);

WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);

volatile float humidity = 50.0;
volatile float temperature = 25.0;
volatile int gasValue = 0;

volatile bool isGasDanger  = false;
volatile bool isTempDanger = false;
volatile bool isHumDanger  = false;

volatile bool alarmMuted = false;
volatile bool testMode  = false;

bool tempAlertSent = false;
bool humAlertSent  = false;
bool gasAlertSent  = false;

unsigned long systemStartTime = 0;
unsigned long lastTelegramCheck = 0;
unsigned long lastReportTime = 0;

// ================= INTERRUPT TOMBOL =================
void IRAM_ATTR ISR_Button_S1() {
  alarmMuted = true;
  digitalWrite(BUZZER_PIN, LOW); 
}

void IRAM_ATTR ISR_Button_S2() {
  if (digitalRead(BUTTON_S2_PIN) == LOW) {
    testMode = true;
    digitalWrite(LED3_PIN, LED_ON);
    digitalWrite(BUZZER_PIN, HIGH);
  } else {
    testMode = false;
    digitalWrite(LED3_PIN, LED_OFF);
    digitalWrite(BUZZER_PIN, LOW);
  }
}

// ================= FORMAT LAPORAN (REAL-TIME STATUS) =================
String getStatusReport() {
  String msg = "📊 LAPORAN MONITORING REAL-TIME\n";
  msg += "━━━━━━━━━━━━━━━━━━━━━━\n";
  
  msg += "🌡️ Suhu: " + String((float)temperature, 1) + " °C ";
  if (temperature < TEMP_MIN_THRESHOLD) msg += "❌ (EXTREME DINGIN)\n";
  else if (temperature > TEMP_MAX_THRESHOLD) msg += "❌ (EXTREME PANAS)\n";
  else msg += "✅ (Aman)\n";

  msg += "💧 Kelembapan: " + String((float)humidity, 1) + " % ";
  if (humidity < HUM_MIN_THRESHOLD) msg += "⚠️ (TERLALU KERING)\n";
  else if (humidity > HUM_MAX_THRESHOLD) msg += "❌ (TERLALU LEMBAP)\n";
  else msg += "✅ (Aman)\n";

  msg += "🔥 Kadar Gas: " + String((int)gasValue) + " ADC ";
  msg += (isGasDanger ? "❌ (BOCOR)\n\n" : "✅ (Aman)\n\n");

  msg += "🌐 Web Dashboard: http://" + WiFi.localIP().toString() + "\n";
  msg += "━━━━━━━━━━━━━━━━━━━━━━\n";

  if (isTempDanger || isHumDanger || isGasDanger) {
    msg += "⚠️ Status: Ada indikasi bahaya!";
  } else {
    msg += "🟢 Status: Semua kondisi lingkungan aman!";
  }

  return msg;
}

// ================= PESAN INTERAKTIF GRUP TELEGRAM =================
void handleNewMessages(int numNewMessages) {
  Serial.print("\n[Telegram] Ada ");
  Serial.print(numNewMessages);
  Serial.println(" pesan baru.");

  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    String text = bot.messages[i].text;
    
    Serial.print("[Telegram] Chat ID: ");
    Serial.print(chat_id);
    Serial.print(" | Pesan: ");
    Serial.println(text);

    text.toLowerCase();

    if (text.startsWith("/status") || text == "status" || text == "/start") {
      Serial.println("[Telegram] Mengirim balasan laporan...");
      bool sent = bot.sendMessage(chat_id, getStatusReport(), "");
      if (sent) {
        Serial.println("[Telegram] Berhasil terkirim!");
      } else {
        Serial.println("[Telegram] Gagal terkirim.");
      }
    }
    else if (text.startsWith("/suhu") || text == "suhu" || text == "lembab") {
      String msg = "🌡️ SUHU & KELEMBAPAN REAL-TIME\n\n";
      msg += "🌡️ Suhu: " + String((float)temperature, 1) + " °C\n";
      msg += "💧 Kelembapan: " + String((float)humidity, 1) + " %\n\n";
      msg += (isTempDanger || isHumDanger ? "⚠️ Kondisi lingkungan tidak normal!\n" : "✅ Kondisi normal & aman.\n");
      msg += "🌐 Dashboard Web: http://" + WiFi.localIP().toString();
      bot.sendMessage(chat_id, msg, "");
    }
    else if (text.startsWith("/gas") || text == "gas") {
      String msg = "🔥 KADAR GAS REAL-TIME\n\n";
      msg += "🔥 Nilai Gas: " + String((int)gasValue) + " ADC\n\n";
      msg += (isGasDanger ? "🚨 PERINGATAN KEBOCORAN GAS!" : "✅ Kadar gas aman.");
      msg += "\n🌐 Dashboard Web: http://" + WiFi.localIP().toString();
      bot.sendMessage(chat_id, msg, "");
    }
  }
}

void handleRoot() { server.send(200, "text/html", HTML_CONTENT); }
void handleData() {
  String dangerMsg = "🟢 Semua kondisi lingkungan aman!";
  if (isGasDanger) {
    dangerMsg = "🚨 BAHAYA: Kadar Gas Melebihi Batas Threshold!";
  } else if (isTempDanger) {
    if (temperature < TEMP_MIN_THRESHOLD) {
      dangerMsg = "⚠️ BAHAYA: Suhu Terlalu Dingin (Ekstrem)!";
    } else {
      dangerMsg = "🔥 BAHAYA: Suhu Melebihi Batas Threshold!";
    }
  } else if (isHumDanger) {
    if (humidity < HUM_MIN_THRESHOLD) {
      dangerMsg = "💧 BAHAYA: Kelembapan Terlalu Kering!";
    } else {
      dangerMsg = "💦 BAHAYA: Kelembapan Terlalu Tinggi!";
    }
  }

  String json = "{";
  json += "\"temp\":" + String((float)temperature, 1) + ",";
  json += "\"hum\":" + String((float)humidity, 1) + ",";
  json += "\"gas\":" + String((int)gasValue) + ",";
  json += "\"temp_danger\":" + String(isTempDanger ? "true" : "false") + ",";
  json += "\"hum_danger\":" + String(isHumDanger ? "true" : "false") + ",";
  json += "\"gas_danger\":" + String(isGasDanger ? "true" : "false") + ",";
  json += "\"message\":\"" + dangerMsg + "\"";
  json += "}";
  server.send(200, "application/json", json);
}

// ================= SETUP =================
void setup() {
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW); 

  Serial.begin(115200);
  systemStartTime = millis();

  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(LED3_PIN, OUTPUT);
  pinMode(LED4_PIN, OUTPUT);

  pinMode(BUTTON_S1_PIN, INPUT_PULLUP);
  pinMode(BUTTON_S2_PIN, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(BUTTON_S1_PIN), ISR_Button_S1, FALLING);
  attachInterrupt(digitalPinToInterrupt(BUTTON_S2_PIN), ISR_Button_S2, CHANGE);

  digitalWrite(LED1_PIN, LED_OFF);
  digitalWrite(LED2_PIN, LED_OFF);
  digitalWrite(LED3_PIN, LED_OFF);
  digitalWrite(LED4_PIN, LED_OFF);

  dht.begin();

  Serial.println("\n--- Memulai Sistem ESP32 ---");
  WiFi.begin(ssid, password);
  
  int wifiTimeout = 0;
  while (WiFi.status() != WL_CONNECTED && wifiTimeout < 20) {
    delay(500);
    Serial.print(".");
    wifiTimeout++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n[OK] Wi-Fi Terhubung!");
    Serial.print("IP Address Web Server: http://");
    Serial.println(WiFi.localIP());
    digitalWrite(LED2_PIN, LED_ON);

    configTime(7 * 3600, 0, "pool.ntp.org", "time.nist.gov");
    time_t now = time(nullptr);
    int timeTimeout = 0;
    while (now < 24 * 3600 && timeTimeout < 20) {
      delay(500);
      Serial.print(".");
      now = time(nullptr);
      timeTimeout++;
    }

    secured_client.setInsecure();
    secured_client.setHandshakeTimeout(10000); 

    bot.getUpdates(bot.last_message_received + 1);
    Serial.println("[OK] Telegram Bot Siap!");

  } else {
    Serial.println("\n[ERROR] Wi-Fi Gagal Terhubung!");
    digitalWrite(LED2_PIN, LED_OFF);
  }

  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.begin();
}

// ================= LOOP UTAMA =================
void loop() {
  // 1. Web Server Client
  server.handleClient();
  unsigned long currentMillis = millis();

  // 2. Baca Sensor Gas
  gasValue = analogRead(MQ2_ANALOG_PIN);

  // 3. Baca Sensor DHT11 Tiap 1 Detik
  static unsigned long lastDHTRead = 0;
  if (currentMillis - lastDHTRead >= 1000) {
    lastDHTRead = currentMillis;
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    if (!isnan(h)) humidity = h;
    if (!isnan(t)) temperature = t;

    isTempDanger = (temperature < TEMP_MIN_THRESHOLD || temperature > TEMP_MAX_THRESHOLD);
    isHumDanger  = (humidity < HUM_MIN_THRESHOLD || humidity > HUM_MAX_THRESHOLD);
    isGasDanger  = (gasValue > GAS_THRESHOLD);
  }

  // 4. Proses Telegram (Polling per 3 detik)
  if (WiFi.status() == WL_CONNECTED) {
    if (currentMillis - lastTelegramCheck >= TELEGRAM_CHECK_INTERVAL) {
      lastTelegramCheck = currentMillis;
      int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
      if (numNewMessages > 0) {
        handleNewMessages(numNewMessages);
      }
    }

    // Alert Otomatis
    if (isTempDanger && !tempAlertSent) {
      String alertText = (temperature < TEMP_MIN_THRESHOLD) ? 
        "🚨 ALERT SUHU DINGIN EXTREME! (" + String((float)temperature, 1) + " °C)\n🌐 http://" + WiFi.localIP().toString() : 
        "⚠️ ALERT SUHU PANAS EXTREME! (" + String((float)temperature, 1) + " °C)\n🌐 http://" + WiFi.localIP().toString();
      if (bot.sendMessage(CHANNEL_ID, alertText, "")) tempAlertSent = true;
    } else if (!isTempDanger && tempAlertSent) {
      if (bot.sendMessage(CHANNEL_ID, "✅ Suhu Kembali Normal: " + String((float)temperature, 1) + " °C", "")) tempAlertSent = false;
    }

    if (isGasDanger && !gasAlertSent) {
      if (bot.sendMessage(CHANNEL_ID, "🚨 ALERT KEBOCORAN GAS! (" + String((int)gasValue) + " ADC)\n🌐 http://" + WiFi.localIP().toString(), "")) gasAlertSent = true;
    } else if (!isGasDanger && gasAlertSent) {
      if (bot.sendMessage(CHANNEL_ID, "✅ Kadar Gas Kembali Normal: " + String((int)gasValue) + " ADC", "")) gasAlertSent = false;
    }

    // Broadcast Rutin Tiap 1 menit
    if (currentMillis - lastReportTime >= REPORT_INTERVAL) {
      lastReportTime = currentMillis;
      bot.sendMessage(CHANNEL_ID, getStatusReport(), "");
    }
  }

  // 5. Reset Alarm
  if (!isTempDanger && !isGasDanger && !isHumDanger) {
    alarmMuted = false;
  }

  // 6. Indikator LED Gas
  static unsigned long lastBlink = 0;
  if (isGasDanger) {
    if (currentMillis - lastBlink >= 200) {
      lastBlink = currentMillis;
      digitalWrite(LED4_PIN, !digitalRead(LED4_PIN));
    }
  } else {
    digitalWrite(LED4_PIN, LED_OFF);
  }

  // 7. Buzzer
  if (!testMode) {
    digitalWrite(LED3_PIN, isTempDanger ? LED_ON : LED_OFF);
    bool isSystemReady = (currentMillis - systemStartTime > 6000);

    if (isSystemReady && (isGasDanger || isTempDanger || isHumDanger) && !alarmMuted) {
      digitalWrite(BUZZER_PIN, (millis() % 400 < 200) ? HIGH : LOW);
    } else {
      digitalWrite(BUZZER_PIN, LOW);
    }
  }

  // 8. Heartbeat LED1
  static unsigned long lastHeartbeat = 0;
  if (currentMillis - lastHeartbeat >= 1000) {
    lastHeartbeat = currentMillis;
    digitalWrite(LED1_PIN, !digitalRead(LED1_PIN));
  }
}