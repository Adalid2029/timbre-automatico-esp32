#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

// --- Configuración WiFi ---
const char* ssid = "XiaomiDESKT";
const char* password = "38514579";

// --- Configuración OLED ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SDA 21
#define SCL 22
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// --- Variables para BTC ---
double btcPrice = 0.0;
double previousPrice = 0.0;
String priceDirection = "";
unsigned long lastUpdate = 0;
const unsigned long updateInterval = 5000;  // Actualizar cada 10 segundos

void setup() {
  Serial.begin(115200);

  // Inicializar I2C
  Wire.begin(SDA, SCL);

  // Inicializar pantalla OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("Error al inicializar pantalla OLED"));
    for (;;)
      ;
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("Iniciando...");
  display.display();
  delay(2000);

  // Conectar a WiFi
  connectToWiFi();

  // Primera consulta de precio
  getBTCPrice();
  displayBTCData();
  // displayDetailedData();
}

void loop() {
  // Verificar conexión WiFi
  if (WiFi.status() != WL_CONNECTED) {
    connectToWiFi();
  }

  // Actualizar precio cada intervalo definido
  if (millis() - lastUpdate >= updateInterval) {
    getBTCPrice();
    displayBTCData();
    // displayDetailedData();
    lastUpdate = millis();
  }

  delay(1000);
}

void connectToWiFi() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Conectando WiFi...");
  display.display();

  WiFi.begin(ssid, password);
  Serial.print("Conectando a WiFi");

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.println("WiFi conectado!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());

    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("WiFi Conectado!");
    display.print("IP: ");
    display.println(WiFi.localIP());
    display.display();
    delay(2000);
  } else {
    Serial.println("Error de conexión WiFi");
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Error WiFi!");
    display.display();
    delay(3000);
  }
}

void getBTCPrice() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin("https://api.binance.com/api/v3/ticker/price?symbol=BTCUSDT");

    int httpResponseCode = http.GET();

    if (httpResponseCode == 200) {
      String payload = http.getString();

      // Parsear JSON
      DynamicJsonDocument doc(1024);
      deserializeJson(doc, payload);

      previousPrice = btcPrice;
      btcPrice = doc["price"].as<double>();

      // Determinar dirección del precio
      if (btcPrice > previousPrice && previousPrice != 0.0) {
        priceDirection = "UP";
      } else if (btcPrice < previousPrice && previousPrice != 0.0) {
        priceDirection = "DOWN";
      } else {
        priceDirection = "STABLE";
      }

      Serial.print("Precio BTC: $");
      Serial.println(btcPrice, 2);
      Serial.print("Dirección: ");
      Serial.println(priceDirection);

    } else {
      Serial.print("Error HTTP: ");
      Serial.println(httpResponseCode);
      btcPrice = -1;
    }

    http.end();
  }
}

void displayBTCData() {
  display.clearDisplay();

  // Título
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("=== BTC TRADING ===");

  // Precio principal
  display.setTextSize(2);    // Tamaño de texto grande (2x) para destacar
  display.setCursor(0, 12);  // Posición X=0, Y=12 (debajo del título)

  if (btcPrice > 0) {    // Si el precio es válido
    display.print("$");  // Imprime símbolo de dólar

    if (btcPrice >= 100000) {             // Si precio >= 100,000
      display.print(btcPrice / 1000, 3);  // ✅ CORRECTO: Divide entre 1000 y muestra 3 decimales
      display.print("K");                 // Añade "K" (ej: 117.456K)
    } else {
      display.print(btcPrice, 3);  // ✅ CORRECTO: Precio normal con 3 decimales
    }
  } else {
    display.print("ERROR");  // Si hay error en obtener precio
  }

  // Indicador de dirección
  display.setTextSize(1);
  display.setCursor(0, 32);
  if (priceDirection == "UP") {
    display.print("^ SUBIENDO");
  } else if (priceDirection == "DOWN") {
    display.print("v BAJANDO");
  } else if (priceDirection == "STABLE") {
    display.print("- ESTABLE");
  }

  // Precio anterior (si existe) - también corregido para mostrar decimales
  if (previousPrice > 0 && previousPrice != btcPrice) {
    display.setCursor(0, 42);
    display.print("Ant: $");
    if (previousPrice >= 100000) {
      display.print(previousPrice / 1000, 3);  // ✅ CORRECTO: Con decimales
      display.print("K");
    } else {
      display.print(previousPrice, 3);  // ✅ CORRECTO: Con decimales
    }
  }

  // Estado de conexión y tiempo
  display.setCursor(0, 54);
  if (WiFi.status() == WL_CONNECTED) {
    display.print("Online ");
  } else {
    display.print("Offline ");
  }

  // Mostrar tiempo transcurrido desde última actualización
  unsigned long secondsSinceUpdate = (millis() - lastUpdate) / 1000;
  display.print(secondsSinceUpdate);
  display.print("s");

  display.display();
}

// Función adicional para mostrar datos detallados (opcional)
void displayDetailedData() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("BTC DETALLES");
  display.println("---------------");

  display.print("Precio: $");
  display.println(btcPrice, 2);

  if (previousPrice > 0) {
    double change = btcPrice - previousPrice;
    double changePercent = (change / previousPrice) * 100;

    display.print("Cambio: $");
    display.println(change, 2);

    display.print("% Cambio: ");
    display.print(changePercent, 2);
    display.println("%");
  }

  display.print("Estado: ");
  display.println(priceDirection);

  display.display();
}