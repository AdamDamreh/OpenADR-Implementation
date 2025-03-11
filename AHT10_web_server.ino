#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_AHTX0.h>
#include <WebServer.h>

// 🛜 WiFi Credentials
const char *ssid = "WIFI Username";
const char *password = "WIFI PASS";

// 🌐 Web Server (Port 80)
WebServer server(80);

// 🌡 AHT10 Sensor
Adafruit_AHTX0 aht;

void scanI2C() {
  Serial.println("🔍 Scanning I2C devices...");
  
  byte error, address;
  int nDevices = 0;
  for (address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0) {
      Serial.print("✅ I2C device found at 0x");
      Serial.println(address, HEX);
      nDevices++;
    }
  }
  
  if (nDevices == 0) {
    Serial.println("❌ No I2C devices found! Check wiring.");
  } else {
    Serial.println("✅ I2C devices detected.");
  }
}

void handleRoot() {
  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp);

  char msg[1500];
  snprintf(msg, 1500,
    "<html>\
    <head>\
      <meta http-equiv='refresh' content='5'/>\
      <meta name='viewport' content='width=device-width, initial-scale=1'>\
      <title>ESP32 AHT10 Server</title>\
      <style>\
      html { font-family: Arial; text-align: center; margin: 0 auto;}\
      h2 { font-size: 2.5rem; }\
      p { font-size: 2rem; }\
      </style>\
    </head>\
    <body>\
      <h2>ESP32 AHT10 Server</h2>\
      <p>🌡 Temperature: %.2f °C</p>\
      <p>💧 Humidity: %.2f %%</p>\
    </body>\
    </html>", 
    temp.temperature, humidity.relative_humidity
  );

  server.send(200, "text/html", msg);
}

void setup() {
  Serial.begin(115200);
  delay(1000); 
  Serial.println("\n\n🚀 Booting ESP32...");

  // 🛜 Connect to WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");

  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < 20) {
    delay(500);
    Serial.print(".");
    retries++;
  }
  
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\n❌ WiFi Connection Failed! Check SSID/PASSWORD.");
    return;
  }
  
  Serial.println("\n✅ Connected to WiFi!");
  Serial.print("📡 IP Address: ");
  Serial.println(WiFi.localIP());

  // 🔍 Scan I2C
  Wire.begin(27, 33);  // Ensure correct SDA/SCL pins
  scanI2C();

  // 🌡 Initialize AHT10
  if (!aht.begin()) {
    Serial.println("❌ AHT10 not detected! Check connections.");
    while (1) delay(10);
  }
  Serial.println("✅ AHT10 Sensor Initialized!");

  // 🌐 Start HTTP Server
  server.on("/", handleRoot);
  server.begin();
  Serial.println("✅ HTTP Server Started!");
}

void loop() {
  server.handleClient();

  // 🌡 Read Sensor Data
  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp);

  Serial.print("🌡 Temperature: ");
  Serial.print(temp.temperature);
  Serial.print(" °C, 💧 Humidity: ");
  Serial.print(humidity.relative_humidity);
  Serial.println(" %");

  delay(5000);  // Slow down readings
}
