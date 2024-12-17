#include <WiFi.h>
#include <Preferences.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Adafruit_NeoPixel.h>

#define NUM_LEDS 12
#define LED_PIN 25
#define AP_SSID "ESP32_Setup"
#define AP_PASSWORD "12345678"

bool ledsOn = true;
uint32_t selectedColor = 0xFF0000; 
int mode = 0;

Preferences preferences;
String storedSSID;
String storedPassword;
bool wifiConfigured = false;

Adafruit_NeoPixel ledRing(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);
AsyncWebServer server(80);

byte heat[NUM_LEDS];

uint32_t wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return ledRing.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if (WheelPos < 170) {
    WheelPos -= 85;
    return ledRing.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return ledRing.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

void resetWiFiConfig() {
  Serial.println("WLAN Daten werden gelöscht...");
  preferences.begin("wifi", false);
  preferences.clear();
  preferences.end();
  Serial.println("WLAN-Daten gelöscht. Neustart!");
  ESP.restart();
}

bool connectToWiFi(const char* ssid, const char* password) {
  Serial.println("Versuche, mit WLAN zu verbinden...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  unsigned long startAttemptTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 20000) {
    delay(100);
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("Verbunden mit: ");
    Serial.println(ssid);
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    return true;
  } else {
    Serial.println("Konnte nicht mit WLAN verbinden!");
    return false;
  }
}

String getWiFiOptions() {
  Serial.println("Scanne nach verfügbaren WLANs...");
  int n = WiFi.scanNetworks();
  String options = "";
  for (int i = 0; i < n; ++i) {
    options += "<option value=\"" + String(WiFi.SSID(i)) + "\">" + String(WiFi.SSID(i)) + "</option>";
  }
  return options;
}

String setupPage() {
  Serial.println("Erstelle Setup-Seite...");
  String page = "<!DOCTYPE html><html><head>"
                "<meta charset=\"UTF-8\">"
                "<meta name=\"viewport\" content=\"width=device-width,initial-scale=1.0\">"
                "<style>body{font-family:Arial;padding:20px;}h1{color:#333;}label{display:block;margin-top:10px;}button{margin-top:20px;padding:10px;}</style>"
                "</head><body><h1>WLAN Einrichten</h1>";
  page += "<form action=\"/saveWiFi\" method=\"POST\">";
  page += "<label for=\"ssid\">Wähle dein WLAN:</label>";
  page += "<select name=\"ssid\" id=\"ssid\">" + getWiFiOptions() + "</select>";
  page += "<label for=\"password\">Passwort:</label>";
  page += "<input type=\"password\" name=\"password\" id=\"password\"><br><br>";
  page += "<button type=\"submit\">Speichern</button></form>";
  page += "<form action=\"/resetWiFi\" method=\"POST\" id=\"resetWiFiForm\" style=\"margin-top:20px;\">"
          "<button type=\"submit\" style=\"background:#f00;color:#fff;\">WLAN Daten zurücksetzen</button></form>";
  // Skript für AJAX, damit Reset nicht navigiert
  page += "<script>"
          "document.getElementById('resetWiFiForm').addEventListener('submit', function(e){"
          "e.preventDefault();"
          "fetch('/resetWiFi', {method:'POST'}).then(r=>r.text()).then(t=>console.log(t));"
          "});"
          "</script>";
  page += "</body></html>";
  return page;
}

String controlPage() {
  Serial.println("Erstelle Control-Seite...");
  String page = "<!DOCTYPE html><html><head>"
                "<meta charset=\"UTF-8\">"
                "<meta name=\"viewport\" content=\"width=device-width,initial-scale=1.0\">"
                "<style>body{font-family:Arial;padding:20px;}h1{color:#333;}label,select,input{display:block;margin-top:10px;}button{margin-top:20px;padding:10px;} .colorInput {width:100%;height:50px;}</style>"
                "</head><body><h1>LED Steuerung</h1>"
                "<p>Verbunden mit: " + String(WiFi.SSID()) + "</p>"
                "<form id=\"colorForm\">"
                "<label for=\"color\">Farbe auswählen:</label>"
                "<input type=\"color\" name=\"color\" id=\"color\" value=\"#ff0000\" class=\"colorInput\">"
                "<button type=\"submit\">Farbe setzen</button>"
                "</form>"
                "<form id=\"effectForm\">"
                "<label for=\"effect\">Effekt auswählen:</label>"
                "<select name=\"effect\" id=\"effect\">"
                "<option value=\"0\">Farbe statisch</option>"
                "<option value=\"1\">Blinken</option>"
                "<option value=\"2\">Füllen</option>"
                "<option value=\"3\">Rainbow-Füllen</option>"
                "<option value=\"4\">Blitz (Lightning)</option>"
                "<option value=\"5\">Dementor Alarm</option>"
                "<option value=\"6\">Disco</option>"
                "<option value=\"7\">Feuer</option>"
                "<option value=\"8\">Meteor</option>"
                "<option value=\"9\">Regentropfen</option>"
                "</select>"
                "<button type=\"submit\">Effekt setzen</button>"
                "</form>"
                "<form id=\"toggleForm\">"
                "<button type=\"submit\">LEDs Ein/Aus</button>"
                "</form>"
                "<form id=\"resetWiFiForm\">"
                "<button type=\"submit\" style=\"background:#f00;color:#fff;\">WLAN Daten zurücksetzen</button>"
                "</form>"
                "<script>"
                "document.getElementById('colorForm').addEventListener('submit', function(e){"
                "e.preventDefault();"
                "const color = document.getElementById('color').value;"
                "fetch('/setColor',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:'color='+encodeURIComponent(color)})"
                ".then(r=>r.text()).then(t=>console.log(t));"
                "});"

                "document.getElementById('effectForm').addEventListener('submit', function(e){"
                "e.preventDefault();"
                "const effect = document.getElementById('effect').value;"
                "fetch('/setEffect',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:'effect='+encodeURIComponent(effect)})"
                ".then(r=>r.text()).then(t=>console.log(t));"
                "});"

                "document.getElementById('toggleForm').addEventListener('submit', function(e){"
                "e.preventDefault();"
                "fetch('/toggleLEDs',{method:'POST'})"
                ".then(r=>r.text()).then(t=>console.log(t));"
                "});"

                "document.getElementById('resetWiFiForm').addEventListener('submit', function(e){"
                "e.preventDefault();"
                "fetch('/resetWiFi',{method:'POST'})"
                ".then(r=>r.text()).then(t=>console.log(t));"
                "});"
                "</script>"
                "</body></html>";
  return page;
}

uint32_t htmlColorToRGB(const String &colorStr) {
  if (colorStr.length() == 7 && colorStr.charAt(0) == '#') {
    long number = strtol(colorStr.substring(1).c_str(), NULL, 16);
    uint8_t r = (number >> 16) & 0xFF;
    uint8_t g = (number >> 8) & 0xFF;
    uint8_t b = number & 0xFF;
    return ledRing.Color(r, g, b);
  }
  return ledRing.Color(255, 0, 0); 
}

// Effekte unverändert
void blinkingEffect() {
  static bool state = false;
  state = !state;
  for (int i = 0; i < NUM_LEDS; i++) {
    ledRing.setPixelColor(i, state ? selectedColor : 0);
  }
  ledRing.show();
  delay(500);
}

void fillingEffect() {
  static int index = 0;
  static bool filling = true;
  if (filling) {
    ledRing.setPixelColor(index, selectedColor);
    index++;
    if (index >= NUM_LEDS) filling = false;
  } else {
    index--;
    ledRing.setPixelColor(index, 0);
    if (index <= 0) filling = true;
  }
  ledRing.show();
  delay(100);
}

void rainbowFillingEffect() {
  static int index = 0;
  static bool filling = true;
  if (filling) {
    ledRing.setPixelColor(index, wheel((index * 256 / NUM_LEDS) % 256));
    index++;
    if (index >= NUM_LEDS) {
      filling = false; 
    }
  } else {
    index--;
    ledRing.setPixelColor(index, 0);
    if (index <= 0) {
      filling = true;
    }
  }
  ledRing.show();
  delay(150);
}

void lightningEffect() {
  ledRing.clear();
  int flashes = random(3, 6);
  for (int i = 0; i < flashes; i++) {
    int randomLED = random(0, NUM_LEDS);
    ledRing.setPixelColor(randomLED, ledRing.Color(255, 255, 255));
    ledRing.show();
    delay(random(50, 150));
    ledRing.setPixelColor(randomLED, 0);
    ledRing.show();
    delay(random(100, 300));
  }
  delay(random(500, 1500));
}

void dementorAlarmEffect() {
  static int startIndex = 0;
  startIndex = (startIndex + 1) % NUM_LEDS;
  for (int i = 0; i < NUM_LEDS; i++) {
    int position = (startIndex + i) % NUM_LEDS;
    uint32_t color;
    int randomEffect = random(0, 3);
    if (randomEffect == 0) {
      color = ledRing.Color(0, 0, random(50, 255));
    } else if (randomEffect == 1) {
      color = ledRing.Color(random(10, 50), random(10, 50), random(10, 50));
    } else {
      color = ledRing.Color(random(200, 255), random(200, 255), random(200, 255));
    }
    if (random(0, 10) > 7) {
      ledRing.setPixelColor(position, 0);
    } else {
      ledRing.setPixelColor(position, color);
    }
  }
  ledRing.show();
  delay(random(50, 200));
}

void discoEffect() {
  for (int i = 0; i < NUM_LEDS; i++) {
    int r = random(0, 256);
    int g = random(0, 256);
    int b = random(0, 256);
    ledRing.setPixelColor(i, ledRing.Color(r, g, b));
  }
  ledRing.show();
  delay(random(100, 200));
}

uint32_t heatToColor(byte temperature) {
  byte t = temperature;
  byte r = (t > 128) ? 255 : t * 2;
  byte g = (t < 128) ? t * 2 : 255;
  byte b = 0;
  return ledRing.Color(r, g, b);
}

void fireEffect() {
  for (int i = 0; i < NUM_LEDS; i++) {
    heat[i] = max(0, heat[i] - (int)random(0, 6));
  }
  for (int i = NUM_LEDS - 1; i >= 2; i--) {
    heat[i] = (heat[i - 1] + heat[i - 2] + heat[i - 2]) / 3;
  }
  if (random(0, 255) < 160) {
    int sparkIndex = random(0, 3);
    heat[sparkIndex] = min(255, heat[sparkIndex] + (int)random(160, 255));
  }
  for (int i = 0; i < NUM_LEDS; i++) {
    byte t = heat[i];
    uint32_t color = heatToColor(t);
    ledRing.setPixelColor(i, color);
  }
  ledRing.show();
  delay(30);
}

void meteorEffect() {
  static int position = 0;
  static bool direction = true;
  int meteorSize = 5;
  uint8_t fadeAmount = 64;
  for (int i = 0; i < NUM_LEDS; i++) {
    uint32_t currentColor = ledRing.getPixelColor(i);
    uint8_t r = (currentColor >> 16) & 0xFF;
    uint8_t g = (currentColor >> 8) & 0xFF;
    uint8_t b = (currentColor & 0xFF);
    r = (r > fadeAmount) ? r - fadeAmount : 0;
    g = (g > fadeAmount) ? g - fadeAmount : 0;
    b = (b > fadeAmount) ? b - fadeAmount : 0;
    ledRing.setPixelColor(i, ledRing.Color(r, g, b));
  }
  for (int i = 0; i < meteorSize; i++) {
    int pos = (position - i + NUM_LEDS) % NUM_LEDS;
    uint8_t brightness = 255 - (i * (255 / meteorSize));
    ledRing.setPixelColor(pos, ledRing.Color(brightness, brightness, brightness));
  }
  ledRing.show();
  position = direction ? position + 1 : position - 1;
  if (position >= NUM_LEDS) {
    direction = false;
    position = NUM_LEDS - 1;
  } else if (position < 0) {
    direction = true;
    position = 0;
  }
  delay(100);
}

void raindropEffect() {
  static uint32_t colors[NUM_LEDS];
  for (int i = 0; i < NUM_LEDS; i++) {
    uint8_t r = (colors[i] >> 16) & 0xFF;
    uint8_t g = (colors[i] >> 8) & 0xFF;
    uint8_t b = colors[i] & 0xFF;
    r = (r > 10) ? r - 10 : 0;
    g = (g > 10) ? g - 10 : 0;
    b = (b > 10) ? b - 10 : 0;
    colors[i] = ledRing.Color(r, g, b);
    ledRing.setPixelColor(i, colors[i]);
  }
  if (random(0, 100) < 20) {
    int dropPosition = random(0, NUM_LEDS);
    colors[dropPosition] = ledRing.Color(random(50, 255), random(50, 255), random(50, 255));
  }
  ledRing.show();
  delay(50);
}

void setupRoutes() {
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    Serial.println("GET / aufgerufen");
    if (!wifiConfigured) {
      Serial.println("wifiConfigured = false, sende Setup-Seite");
      request->send(200, "text/html; charset=UTF-8", setupPage());
    } else {
      Serial.println("wifiConfigured = true, sende Control-Seite");
      request->send(200, "text/html; charset=UTF-8", controlPage());
    }
  });

  server.on("/saveWiFi", HTTP_POST, [](AsyncWebServerRequest *request) {
    Serial.println("POST /saveWiFi aufgerufen");
    if (request->hasParam("ssid", true) && request->hasParam("password", true)) {
      String ssid = request->getParam("ssid", true)->value();
      String pass = request->getParam("password", true)->value();
      Serial.println("WLAN-Daten erhalten:");
      Serial.print("SSID: ");
      Serial.println(ssid);
      Serial.print("Pass: ");
      Serial.println(pass);

      preferences.begin("wifi", false);
      preferences.putString("ssid", ssid);
      preferences.putString("password", pass);
      preferences.end();

      request->send(200, "text/plain; charset=UTF-8", "WLAN-Daten gespeichert. Neustart...");
      delay(1000);
      ESP.restart();
    } else {
      request->send(400, "text/plain; charset=UTF-8", "Fehlende Parameter.");
    }
  });

  server.on("/setColor", HTTP_POST, [](AsyncWebServerRequest *request) {
    Serial.println("POST /setColor aufgerufen");
    if (request->hasParam("color", true)) {
      String colorValue = request->getParam("color", true)->value();
      selectedColor = htmlColorToRGB(colorValue);
      Serial.print("Farbe gesetzt auf: ");
      Serial.println(colorValue);
      request->send(200, "text/plain; charset=UTF-8", "Farbe gesetzt auf: " + colorValue);
    } else {
      request->send(400, "text/plain; charset=UTF-8", "Ungültige Parameter");
    }
  });

  server.on("/setEffect", HTTP_POST, [](AsyncWebServerRequest *request) {
    Serial.println("POST /setEffect aufgerufen");
    if (request->hasParam("effect", true)) {
      String effectValue = request->getParam("effect", true)->value();
      mode = effectValue.toInt();
      Serial.print("Effekt geändert auf: ");
      Serial.println(effectValue);
      request->send(200, "text/plain; charset=UTF-8", "Effekt geändert auf: " + effectValue);
    } else {
      request->send(400, "text/plain; charset=UTF-8", "Ungültige Parameter");
    }
  });

  server.on("/toggleLEDs", HTTP_POST, [](AsyncWebServerRequest *request) {
    Serial.println("POST /toggleLEDs aufgerufen");
    ledsOn = !ledsOn;
    Serial.print("LEDs Status: ");
    Serial.println(ledsOn ? "An" : "Aus");
    request->send(200, "text/plain; charset=UTF-8", ledsOn ? "LEDs eingeschaltet" : "LEDs ausgeschaltet");
  });

  server.on("/resetWiFi", HTTP_POST, [](AsyncWebServerRequest *request) {
    Serial.println("POST /resetWiFi aufgerufen");
    resetWiFiConfig();
    request->send(200, "text/plain; charset=UTF-8", "WLAN-Daten wurden gelöscht. Neustart...");
  });
}

void setup() {
  Serial.begin(115200);
  Serial.println("Starte Setup...");

  ledRing.begin();
  ledRing.setBrightness(50);
  ledRing.clear();
  ledRing.show();

  Serial.println("Lese gespeicherte WLAN-Daten...");
  preferences.begin("wifi", true);
  storedSSID = preferences.getString("ssid", "");
  storedPassword = preferences.getString("password", "");
  preferences.end();

  Serial.print("storedSSID: ");
  Serial.println(storedSSID);
  Serial.print("storedPassword: ");
  Serial.println(storedPassword);

  if (storedSSID != "" && storedPassword != "") {
    Serial.println("WLAN-Daten vorhanden, versuche zu verbinden...");
    wifiConfigured = connectToWiFi(storedSSID.c_str(), storedPassword.c_str());
    if (!wifiConfigured) {
      Serial.println("Konnte nicht verbinden, AP-Modus wird gestartet.");
    } else {
      Serial.println("WLAN konfiguriert und verbunden.");
    }
  } else {
    Serial.println("Keine WLAN-Daten vorhanden, AP-Modus wird gestartet.");
    wifiConfigured = false;
  }

  if (!wifiConfigured) {
    Serial.println("Starte AP-Modus...");
    WiFi.mode(WIFI_AP);
    if (WiFi.softAP(AP_SSID, AP_PASSWORD)) {
      Serial.println("AP gestartet!");
      Serial.print("AP SSID: ");
      Serial.println(AP_SSID);
      Serial.print("AP IP: ");
      Serial.println(WiFi.softAPIP());
    } else {
      Serial.println("AP konnte nicht gestartet werden!");
    }
  }

  setupRoutes();
  server.begin();
}

void loop() {
  if (wifiConfigured) {
    if (ledsOn) {
      switch (mode) {
        case 0:
          for (int i = 0; i < NUM_LEDS; i++) {
            ledRing.setPixelColor(i, selectedColor);
          }
          ledRing.show();
          delay(50);
          break;
        case 1:
          blinkingEffect();
          break;
        case 2:
          fillingEffect();
          break;
        case 3:
          rainbowFillingEffect();
          break;
        case 4:
          lightningEffect();
          break;
        case 5:
          dementorAlarmEffect();
          break;
        case 6:
          discoEffect();
          break;
        case 7:
          fireEffect();
          break;
        case 8:
          meteorEffect();
          break;
        case 9:
          raindropEffect();
          break;
        default:
          for (int i = 0; i < NUM_LEDS; i++) {
            ledRing.setPixelColor(i, 0xFF0000);
          }
          ledRing.show();
          delay(50);
          break;
      }
    } else {
      ledRing.clear();
      ledRing.show();
      delay(50);
    }
  } else {
    for (int i = 0; i < NUM_LEDS; i++) {
      ledRing.setPixelColor(i, 0x0000FF);
    }
    ledRing.show();
    delay(500);
    for (int i = 0; i < NUM_LEDS; i++) {
      ledRing.setPixelColor(i, 0);
    }
    ledRing.show();
    delay(500);
  }
}
