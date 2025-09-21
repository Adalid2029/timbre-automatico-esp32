#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include <LittleFS.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <RTClib.h>

RTC_DS3231 rtc;
// --- Configuración OLED ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SDA 21
#define SCL 22
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Configuración del servidor web
WebServer server(80);

// Configuración del relé
const int RELAY_PIN = 2;

// Configuración WiFi AP
const char *ssid = "Timbre 23 de Marzo";
const char *password = "mariaelena13376222";

// Almacenamiento persistente
Preferences preferences;

// Variables globales
bool sistemaActivo = true;
bool estadoDias[7] = {true, true, true, true, true, true, true}; // L-D
String diasNombres[7] = {"Lunes", "Martes", "Miércoles", "Jueves", "Viernes", "Sábado", "Domingo"};
bool timbreSonando = false;
unsigned long inicioTimbre = 0;

struct TipoHorario
{
  String nombre;
  String horarios[50]; // Máximo 50 horarios por tipo
  int cantidadHorarios;
  bool activo;
};

TipoHorario tiposHorarios[10]; // Máximo 10 tipos de horarios
int cantidadTipos = 0;

// Variables para mostrar logs en pantalla
String logActual = "";
unsigned long ultimoLog = 0;
int pantallaActual = 0; // 0=estado, 1=logs, 2=info
unsigned long ultimoCambio = 0;
bool oledDisponible = false;

// --- Función para mostrar en OLED (con verificación) ---
void mostrarPantalla(String linea1, String linea2 = "", String linea3 = "", String linea4 = "")
{
  if (!oledDisponible)
    return; // No intentar usar OLED si no está disponible

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.println(linea1);

  if (linea2 != "")
  {
    display.setCursor(0, 16);
    display.println(linea2);
  }

  if (linea3 != "")
  {
    display.setCursor(0, 32);
    display.println(linea3);
  }

  if (linea4 != "")
  {
    display.setCursor(0, 48);
    display.println(linea4);
  }

  display.display();
}

void actualizarPantalla()
{
  if (!oledDisponible)
    return;

  unsigned long ahora = millis();

  // Cambiar pantalla cada 5 segundos
  if (ahora - ultimoCambio > 5000)
  {
    ultimoCambio = ahora;
    pantallaActual = (pantallaActual + 1) % 3;
  }

  // USAR DS3231 en lugar del sistema millis()
  DateTime now = rtc.now();
  String horaStr = (now.hour() < 10 ? "0" : "") + String(now.hour()) + ":" +
                   (now.minute() < 10 ? "0" : "") + String(now.minute());

  switch (pantallaActual)
  {
  case 0: // Estado del sistema
    mostrarPantalla(
        "TIMBRE 23 MARZO",
        "Estado: " + String(sistemaActivo ? "ACTIVO" : "INACTIVO"),
        "Hora: " + horaStr,
        "Tipos: " + String(cantidadTipos));
    break;

  case 1: // Logs
    mostrarPantalla(
        "LOGS DEL SISTEMA",
        logActual.substring(0, 20),
        "Tipos: " + String(cantidadTipos),
        "IP: " + WiFi.softAPIP().toString());
    break;

  case 2: // Información WiFi
    mostrarPantalla(
        "RED WIFI",
        "SSID: " + String(ssid),
        "IP: " + WiFi.softAPIP().toString(),
        "Clientes: " + String(WiFi.softAPgetStationNum()));
    break;
  }
}

void log(String mensaje)
{
  Serial.println(mensaje);
  logActual = mensaje;
  ultimoLog = millis();
}

void setup()
{
  Serial.begin(115200);
  delay(1000);
  log("Iniciando sistema...");

  // Intentar iniciar pantalla OLED (sin bloquear si falla)
  Wire.begin(SDA, SCL);
  if (display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  {
    oledDisponible = true;
    log("OLED inicializado OK");
    mostrarPantalla("TIMBRE 23 MARZO", "Iniciando...");
  }
  else
  {
    log("OLED no disponible - continuando sin pantalla");
    oledDisponible = false;
  }

  delay(1000);
  if (!rtc.begin())
  {
    log("Error: DS3231 no encontrado");
  }
  else
  {
    log("DS3231 inicializado OK");
  }
  // Configurar relé
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);
  log("Relé configurado");

  // Inicializar LittleFS
  if (oledDisponible)
    mostrarPantalla("TIMBRE 23 MARZO", "Iniciando LittleFS...");

  if (!LittleFS.begin(true))
  {
    log("Error: LittleFS falló");
    if (oledDisponible)
      mostrarPantalla("ERROR", "LittleFS falló");
    delay(2000);
  }
  else
  {
    log("LittleFS iniciado OK");

    // === CÓDIGO DE DIAGNÓSTICO OBLIGATORIO ===
    log("=== DIAGNÓSTICO LITTLEFS ===");

    // Información del sistema de archivos
    size_t totalBytes = LittleFS.totalBytes();
    size_t usedBytes = LittleFS.usedBytes();
    log("LittleFS - Total: " + String(totalBytes) + " bytes, Usado: " + String(usedBytes) + " bytes");

    // Listar todos los archivos
    File root = LittleFS.open("/");
    if (!root)
    {
      log("Error: No se puede abrir directorio raíz");
    }
    else
    {
      log("Directorio raíz abierto OK");

      File file = root.openNextFile();
      int contador = 0;
      while (file)
      {
        String fileName = file.name();
        size_t fileSize = file.size();
        log("Archivo " + String(contador++) + ": " + fileName + " (" + String(fileSize) + " bytes)");
        file = root.openNextFile();
      }
      root.close();

      if (contador == 0)
      {
        log("¡NO HAY ARCHIVOS EN LITTLEFS!");
      }
    }

    // Verificar archivos específicos
    String archivos[] = {"/index.html", "/style.css", "/app.js"};
    for (int i = 0; i < 3; i++)
    {
      File test = LittleFS.open(archivos[i], "r");
      if (test)
      {
        log("✅ " + String(archivos[i]) + " existe (" + String(test.size()) + " bytes)");
        test.close();
      }
      else
      {
        log("❌ " + String(archivos[i]) + " NO EXISTE");
      }
    }

    log("=== FIN DIAGNÓSTICO ===");
  }

  // Inicializar almacenamiento
  preferences.begin("timbre", false);
  cargarConfiguracion();
  log("Configuración cargada");

  // Configurar punto de acceso WiFi
  if (oledDisponible)
    mostrarPantalla("TIMBRE 23 MARZO", "Iniciando WiFi...");
  WiFi.mode(WIFI_AP);
  delay(100);

  bool apResult = WiFi.softAP(ssid, password, 1, 0, 4);

  if (apResult)
  {
    log("WiFi AP creado OK");
    IPAddress IP = WiFi.softAPIP();
    log("IP: " + IP.toString());
    if (oledDisponible)
      mostrarPantalla("WiFi OK", "IP: " + IP.toString(), "SSID: " + String(ssid));
    delay(2000);
  }
  else
  {
    log("Error: WiFi AP falló");
    if (oledDisponible)
      mostrarPantalla("ERROR", "WiFi AP falló");
    delay(2000);
  }

  // Configurar rutas del servidor web
  server.on("/", HTTP_GET, manejarPaginaPrincipal);
  server.on("/style.css", HTTP_GET, manejarCSS);
  server.on("/app.js", HTTP_GET, manejarJS);
  server.on("/api/hora", HTTP_GET, manejarObtenerHora);
  server.on("/api/configurar", HTTP_POST, manejarConfigurar);
  server.on("/api/sonar", HTTP_POST, manejarSonarManual);
  server.on("/api/toggle", HTTP_POST, manejarToggleSistema);
  server.on("/api/estado", HTTP_GET, manejarObtenerEstado);
  server.on("/api/dias", HTTP_POST, manejarConfigurarDias);
  server.on("/api/logs", HTTP_GET, manejarObtenerLogs);
  server.on("/api/configurar-hora", HTTP_POST, manejarConfigurarHora);
  server.on("/api/detener", HTTP_POST, manejarDetenerTimbre);

  server.begin();
  log("Servidor web iniciado");

  // Mensaje de bienvenida
  log("Sistema listo!");
  if (oledDisponible)
    mostrarPantalla("SISTEMA LISTO", "Timbre 23 Marzo", "WiFi: " + String(ssid));
  delay(2000);
}

void manejarConfigurarHora()
{
  if (!server.hasArg("plain"))
  {
    server.send(400, "application/json", "{\"error\":\"No data received\"}");
    return;
  }

  String body = server.arg("plain");
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, body);

  if (error)
  {
    server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
    return;
  }

  int año = doc["año"];
  int mes = doc["mes"];
  int dia = doc["dia"];
  int hora = doc["hora"];
  int minuto = doc["minuto"];
  int segundo = doc["segundo"];

  rtc.adjust(DateTime(año, mes, dia, hora, minuto, segundo));

  log("Hora configurada: " + String(dia) + "/" + String(mes) + "/" + String(año) + " " + String(hora) + ":" + String(minuto));

  server.send(200, "application/json", "{\"status\":\"ok\"}");
}

void loop()
{
  server.handleClient();

  if (timbreSonando && (millis() - inicioTimbre >= 10000))
  {
    digitalWrite(RELAY_PIN, HIGH);
    timbreSonando = false;
    log("Timbre terminado");
    if (oledDisponible)
      mostrarPantalla("TIMBRE", "Completado", "Sistema activo");
  }

  if (sistemaActivo)
  {
    verificarHorarios();
  }

  // Actualizar pantalla OLED (solo si está disponible)
  if (oledDisponible)
  {
    actualizarPantalla();
  }

  // Verificar estado del WiFi periódicamente
  static unsigned long ultimaVerificacionWiFi = 0;
  if (millis() - ultimaVerificacionWiFi > 30000)
  {
    ultimaVerificacionWiFi = millis();
    if (WiFi.getMode() != WIFI_AP && WiFi.getMode() != WIFI_AP_STA)
    {
      log("WiFi desconectado, reintentando...");
      WiFi.mode(WIFI_AP);
      WiFi.softAP(ssid, password);
    }
  }

  delay(100);
}

void manejarPaginaPrincipal()
{
  File file = LittleFS.open("/index.html", "r");
  if (!file)
  {
    server.send(404, "text/plain", "Archivo no encontrado - Verifica diagnóstico en Serial Monitor");
    log("Error: index.html no encontrado");
    return;
  }

  server.streamFile(file, "text/html");
  file.close();
  log("Página principal servida");
}

void manejarCSS()
{
  File file = LittleFS.open("/style.css", "r");
  if (!file)
  {
    server.send(404, "text/plain", "CSS no encontrado");
    return;
  }

  server.streamFile(file, "text/css");
  file.close();
}

void manejarJS()
{
  File file = LittleFS.open("/app.js", "r");
  if (!file)
  {
    server.send(404, "text/plain", "JS no encontrado");
    return;
  }

  server.streamFile(file, "application/javascript");
  file.close();
}

void manejarObtenerHora()
{
  DateTime now = rtc.now();

  String horaActual = (now.hour() < 10 ? "0" : "") + String(now.hour()) + ":" + (now.minute() < 10 ? "0" : "") + String(now.minute()) + ":" + (now.second() < 10 ? "0" : "") + String(now.second());

  String fechaActual = (now.day() < 10 ? "0" : "") + String(now.day()) + "/" + (now.month() < 10 ? "0" : "") + String(now.month()) + "/" + String(now.year());

  String response = "{\"hora\":\"" + horaActual + "\",\"fecha\":\"" + fechaActual + "\"}";
  server.send(200, "application/json", response);
}

void manejarObtenerLogs()
{
  String logs = "{\"ultimoLog\":\"" + logActual + "\",\"timestamp\":" + String(ultimoLog) + "}";
  server.send(200, "application/json", logs);
}

void manejarConfigurar()
{
  if (!server.hasArg("plain"))
  {
    server.send(400, "application/json", "{\"error\":\"No data received\"}");
    return;
  }

  String body = server.arg("plain");
  DynamicJsonDocument doc(4096);
  DeserializationError error = deserializeJson(doc, body);

  if (error)
  {
    server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
    return;
  }

  String accion = doc["accion"];

  if (accion == "agregar_tipo")
  {
    String nombre = doc["nombre"];
    agregarTipoHorario(nombre);
    log("Tipo agregado: " + nombre);
  }
  else if (accion == "agregar_horario")
  {
    int tipoIndex = doc["tipoIndex"];
    String horario = doc["horario"];
    agregarHorarioATipo(tipoIndex, horario);
    log("Horario agregado: " + horario);
  }
  else if (accion == "eliminar_horario")
  {
    int tipoIndex = doc["tipoIndex"];
    int horarioIndex = doc["horarioIndex"];
    eliminarHorario(tipoIndex, horarioIndex);
    log("Horario eliminado");
  }
  else if (accion == "eliminar_tipo")
  {
    int tipoIndex = doc["tipoIndex"];
    eliminarTipoHorario(tipoIndex);
    log("Tipo eliminado: " + String(tipoIndex));
  }
  else if (accion == "toggle_tipo")
  {
    int tipoIndex = doc["tipoIndex"];
    toggleTipoHorario(tipoIndex);
    log("Tipo toggled");
  }

  guardarConfiguracion();
  server.send(200, "application/json", "{\"status\":\"ok\"}");
}

void manejarSonarManual()
{
  sonarTimbre();
  log("Timbre manual activado");
  server.send(200, "application/json", "{\"status\":\"timbre_sonado\"}");
}

void manejarToggleSistema()
{
  sistemaActivo = !sistemaActivo;
  preferences.putBool("sistemaActivo", sistemaActivo);

  log("Sistema " + String(sistemaActivo ? "activado" : "desactivado"));

  String response = "{\"sistemaActivo\":" + String(sistemaActivo ? "true" : "false") + "}";
  server.send(200, "application/json", response);
}

void manejarObtenerEstado()
{
  String response = "{\"sistemaActivo\":" + String(sistemaActivo ? "true" : "false") + ",\"tipos\":" + generarJSONTipos() + ",\"dias\":" + generarJSONDias() + "}";
  server.send(200, "application/json", response);
}

void manejarConfigurarDias()
{
  if (!server.hasArg("plain"))
  {
    server.send(400, "application/json", "{\"error\":\"No data received\"}");
    return;
  }

  String body = server.arg("plain");
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, body);

  if (error)
  {
    server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
    return;
  }

  for (int i = 0; i < 7; i++)
  {
    estadoDias[i] = doc["dias"][i];
  }

  guardarConfiguracion();
  log("Días configurados");
  server.send(200, "application/json", "{\"status\":\"ok\"}");
}

void verificarHorarios()
{
  DateTime now = rtc.now();
  String horaActual = (now.hour() < 10 ? "0" : "") + String(now.hour()) + ":" + (now.minute() < 10 ? "0" : "") + String(now.minute());

  int diaIndex = now.dayOfTheWeek(); // 0=Domingo, 1=Lunes... 6=Sábado
  if (diaIndex == 0)
    diaIndex = 6; // Convertir Domingo a posición 6
  else
    diaIndex--; // Convertir 1-6 a 0-5

  // Verificar si hoy está habilitado
  if (!estadoDias[diaIndex])
  {
    return;
  }

  // Verificar cada tipo de horario
  for (int i = 0; i < cantidadTipos; i++)
  {
    if (!tiposHorarios[i].activo)
      continue;

    // Verificar cada horario del tipo
    for (int j = 0; j < tiposHorarios[i].cantidadHorarios; j++)
    {
      if (tiposHorarios[i].horarios[j] == horaActual && now.second() == 0)
      {
        log("TIMBRE! " + tiposHorarios[i].nombre + " " + horaActual);
        sonarTimbre();
        return;
      }
    }
  }
}

void sonarTimbre()
{
  log("TIMBRE SONANDO...");
  if (oledDisponible)
    mostrarPantalla("TIMBRE SONANDO", "*** ACTIVO ***", "Duración: 10 seg");

  digitalWrite(RELAY_PIN, LOW);
  timbreSonando = true;
  inicioTimbre = millis();
  // NO usar delay() aquí - se maneja en loop()
}

// Resto de funciones...
void agregarTipoHorario(String nombre)
{
  if (cantidadTipos < 10)
  {
    tiposHorarios[cantidadTipos].nombre = nombre;
    tiposHorarios[cantidadTipos].cantidadHorarios = 0;
    tiposHorarios[cantidadTipos].activo = true;
    cantidadTipos++;
  }
}

void agregarHorarioATipo(int tipoIndex, String horario)
{
  if (tipoIndex < cantidadTipos && tiposHorarios[tipoIndex].cantidadHorarios < 50)
  {
    tiposHorarios[tipoIndex].horarios[tiposHorarios[tipoIndex].cantidadHorarios] = horario;
    tiposHorarios[tipoIndex].cantidadHorarios++;
  }
}

void eliminarHorario(int tipoIndex, int horarioIndex)
{
  if (tipoIndex < cantidadTipos && horarioIndex < tiposHorarios[tipoIndex].cantidadHorarios)
  {
    for (int i = horarioIndex; i < tiposHorarios[tipoIndex].cantidadHorarios - 1; i++)
    {
      tiposHorarios[tipoIndex].horarios[i] = tiposHorarios[tipoIndex].horarios[i + 1];
    }
    tiposHorarios[tipoIndex].cantidadHorarios--;
  }
}

void toggleTipoHorario(int tipoIndex)
{
  if (tipoIndex < cantidadTipos)
  {
    tiposHorarios[tipoIndex].activo = !tiposHorarios[tipoIndex].activo;
  }
}

void eliminarTipoHorario(int tipoIndex)
{
  if (tipoIndex >= cantidadTipos)
    return;

  // Mover todos los tipos hacia atrás
  for (int i = tipoIndex; i < cantidadTipos - 1; i++)
  {
    tiposHorarios[i] = tiposHorarios[i + 1];
  }

  cantidadTipos--;

  // Limpiar el último tipo
  tiposHorarios[cantidadTipos].nombre = "";
  tiposHorarios[cantidadTipos].cantidadHorarios = 0;
  tiposHorarios[cantidadTipos].activo = false;
}

String generarJSONTipos()
{
  String json = "[";
  for (int i = 0; i < cantidadTipos; i++)
  {
    if (i > 0)
      json += ",";
    json += "{\"nombre\":\"" + tiposHorarios[i].nombre + "\",";
    json += "\"activo\":" + String(tiposHorarios[i].activo ? "true" : "false") + ",";
    json += "\"horarios\":[";

    for (int j = 0; j < tiposHorarios[i].cantidadHorarios; j++)
    {
      if (j > 0)
        json += ",";
      json += "\"" + tiposHorarios[i].horarios[j] + "\"";
    }
    json += "]}";
  }
  json += "]";
  return json;
}

String generarJSONDias()
{
  String json = "[";
  for (int i = 0; i < 7; i++)
  {
    if (i > 0)
      json += ",";
    json += String(estadoDias[i] ? "true" : "false");
  }
  json += "]";
  return json;
}

void cargarConfiguracion()
{
  sistemaActivo = preferences.getBool("sistemaActivo", true);
  cantidadTipos = preferences.getInt("cantidadTipos", 0);

  for (int i = 0; i < 7; i++)
  {
    estadoDias[i] = preferences.getBool(("dia" + String(i)).c_str(), true);
  }

  for (int i = 0; i < cantidadTipos; i++)
  {
    String keyNombre = "tipo" + String(i) + "_nombre";
    String keyActivo = "tipo" + String(i) + "_activo";
    String keyCantidad = "tipo" + String(i) + "_cantidad";

    tiposHorarios[i].nombre = preferences.getString(keyNombre.c_str(), "");
    tiposHorarios[i].activo = preferences.getBool(keyActivo.c_str(), true);
    tiposHorarios[i].cantidadHorarios = preferences.getInt(keyCantidad.c_str(), 0);

    for (int j = 0; j < tiposHorarios[i].cantidadHorarios; j++)
    {
      String keyHorario = "tipo" + String(i) + "_h" + String(j);
      tiposHorarios[i].horarios[j] = preferences.getString(keyHorario.c_str(), "");
    }
  }
}

void guardarConfiguracion()
{
  preferences.putBool("sistemaActivo", sistemaActivo);
  preferences.putInt("cantidadTipos", cantidadTipos);

  for (int i = 0; i < 7; i++)
  {
    preferences.putBool(("dia" + String(i)).c_str(), estadoDias[i]);
  }

  for (int i = 0; i < cantidadTipos; i++)
  {
    String keyNombre = "tipo" + String(i) + "_nombre";
    String keyActivo = "tipo" + String(i) + "_activo";
    String keyCantidad = "tipo" + String(i) + "_cantidad";

    preferences.putString(keyNombre.c_str(), tiposHorarios[i].nombre);
    preferences.putBool(keyActivo.c_str(), tiposHorarios[i].activo);
    preferences.putInt(keyCantidad.c_str(), tiposHorarios[i].cantidadHorarios);

    for (int j = 0; j < tiposHorarios[i].cantidadHorarios; j++)
    {
      String keyHorario = "tipo" + String(i) + "_h" + String(j);
      preferences.putString(keyHorario.c_str(), tiposHorarios[i].horarios[j]);
    }
  }
}
void manejarDetenerTimbre()
{
  if (timbreSonando)
  {
    digitalWrite(RELAY_PIN, HIGH); // Desactivar relé
    timbreSonando = false;
    log("TIMBRE DETENIDO POR EMERGENCIA");

    if (oledDisponible)
    {
      mostrarPantalla("EMERGENCIA", "Timbre detenido", "por usuario");
    }
  }

  server.send(200, "application/json", "{\"status\":\"timbre_detenido\"}");
}