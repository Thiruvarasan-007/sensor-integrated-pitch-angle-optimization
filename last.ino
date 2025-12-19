#include <WiFi.h>
#include <HTTPClient.h>
#include <TinyGPSPlus.h>

// ----------------------
// USER CONFIG
// ----------------------
const char* ssid     = "*************";
const char* password = "*********";
const char* OWM_API_KEY = "your api key";
const char* SERVER_URL = "http://10.237.128.62:5000/predict_from_device";

// ----------------------
// GPS SETUP
// ----------------------
TinyGPSPlus gps;
HardwareSerial GPS_Serial(1);   // UART1

// ----------------------
// GLOBAL VARIABLES
// ----------------------
float latitude = 0.0;
float longitude = 0.0;
float altitude = 500.0;      // fixed site altitude (m)

float wind_speed = 0.0;
float wind_deg   = 0.0;
float temperature = 25.0;

// ----------------------
// WIFI CONNECT
// ----------------------
void connectWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Connecting WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

// ----------------------
// READ GPS + DISPLAY NMEA
// ----------------------
void readGPS() {
  static String nmeaLine = "";

  while (GPS_Serial.available()) {
    char c = GPS_Serial.read();

    // ---- SHOW RAW NMEA DATA ----
    Serial.write(c);

    // ---- PARSE FOR TinyGPS++ ----
    gps.encode(c);

    // optional: collect full NMEA line
    if (c == '\n') {
      nmeaLine = "";
    } else {
      nmeaLine += c;
    }
  }

  // ---- SHOW PARSED LOCATION ----
  if (gps.location.isValid()) {
    latitude  = gps.location.lat();
    longitude = gps.location.lng();

    Serial.printf("\nGPS FIX → Lat: %.6f  Lon: %.6f\n",
                  latitude, longitude);
  } else {
    Serial.println("\nNo GPS fix yet...");
  }
}

// ----------------------
// FETCH WEATHER DATA
// ----------------------
bool fetchWeather() {

  if (WiFi.status() != WL_CONNECTED) return false;
  if (!gps.location.isValid()) return false;

  HTTPClient http;

  String url = "http://api.openweathermap.org/data/2.5/weather?lat=" +
               String(latitude, 6) +
               "&lon=" + String(longitude, 6) +
               "&appid=" + OWM_API_KEY +
               "&units=metric";

  Serial.println("\nFetching Weather...");
  http.begin(url);
  int code = http.GET();

  if (code == 200) {
    String payload = http.getString();

    wind_speed  = payload.substring(payload.indexOf("\"speed\":") + 8).toFloat();
    wind_deg    = payload.substring(payload.indexOf("\"deg\":") + 6).toFloat();
    temperature = payload.substring(payload.indexOf("\"temp\":") + 7).toFloat();

    Serial.println("Weather fetched");
    http.end();
    return true;
  }

  Serial.printf("Weather error: %d\n", code);
  http.end();
  return false;
}

// ----------------------
// SEND DATA → ML SERVER
// ----------------------
void sendToMLServer() {

  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  http.begin(SERVER_URL);
  http.addHeader("Content-Type", "application/json");

  String json = "{";
  json += "\"wind_speed\":" + String(wind_speed, 2) + ",";
  json += "\"wind_deg\":" + String(wind_deg, 2) + ",";
  json += "\"temperature\":" + String(temperature, 2) + ",";
  json += "\"altitude\":" + String(altitude, 2) + ",";
  json += "\"latitude\":" + String(latitude, 6) + ",";
  json += "\"longitude\":" + String(longitude, 6);
  json += "}";

  Serial.println("\nSending to ML Server:");
  Serial.println(json);

  int code = http.POST(json);

  if (code == 200) {
    String response = http.getString();
    Serial.println("ML Response:");
    Serial.println(response);

    int idx = response.indexOf("\"pitch\":");
    if (idx != -1) {
      float pitch = response.substring(idx + 8).toFloat();
      Serial.printf("✅ Predicted Pitch Angle: %.2f°\n", pitch);
    }
  } else {
    Serial.printf("POST Error: %d\n", code);
  }

  http.end();
}

// ----------------------
// SETUP
// ----------------------
void setup() {
  Serial.begin(115200);

  // GPS RX=16 TX=17
  GPS_Serial.begin(9600, SERIAL_8N1, 16, 17);

  Serial.println("ESP32 GPS + NMEA + Weather + ML");
  connectWiFi();
}

// ----------------------
// LOOP
// ----------------------
void loop() {

  readGPS();

  if (gps.location.isValid()) {
    if (fetchWeather()) {
      sendToMLServer();
    }
  }

  delay(30000);   // 30 seconds
}
