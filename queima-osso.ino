#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Adafruit_MCP9808.h>
#include <PID_v1.h>
#include <ArduinoJson.h>

#include "fw_config.h"

#define MY_VERSION  1

// Servidor HTTP
ESP8266WebServer server(80);

// Sensor de temperatura
// Adafruit_MCP9808 já inicializa os pinos I2C na porta padrão
Adafruit_MCP9808 temp_sensor;
double temp_value = 0.0;

// Controlador PID
double pid_setpoint = FW_CONFIG_PID_SETPOINT;
double pid_input = 0.0, pid_output = 0.0;
double pid_kp = FW_CONFIG_PID_KP, pid_ki = FW_CONFIG_PID_KI, pid_kd = FW_CONFIG_PID_KD;
PID pid(&pid_input, &pid_output, &pid_setpoint, pid_kp, pid_ki, pid_kd, P_ON_M, DIRECT);

// Uso geral
unsigned long last_millis;

static double get_temp(void);

static void server_handler_root(void);
static void server_handler_get_temp(void);
static void server_handler_getvalues(void);
static void server_handler_setvalues(void);

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.print("queima-osso v");
  Serial.println(MY_VERSION);

#if !FW_CONFIG_TEST_XPTEC
#if FW_CONFIG_AP_PASS_EN
  WiFi.softAP(FW_CONFIG_AP_SSID, FW_CONFIG_AP_PASS);
  Serial.println("wifi ap iniciada");
#else
  WiFi.softAP(FW_CONFIG_AP_SSID);
  Serial.println("wifi ap iniciada sem senha");
#endif
  IPAddress my_ip = WiFi.softAPIP();
  Serial.print("endereço ip: ");
  Serial.println(my_ip);
#else
  WiFi.mode(WIFI_STA);
  WiFi.begin("Xptec", "@ADM1993@");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
#endif

  temp_sensor.begin(0x18);
  Serial.println("sensor de temperatura iniciado");

  pid.SetOutputLimits(FW_CONFIG_PID_OUT_LIMIT_MIN, FW_CONFIG_PID_OUT_LIMIT_MAX);
  pid.SetMode(AUTOMATIC);
  Serial.println("controlador iniciado");

  server.on("/", HTTP_GET, server_handler_root);
  server.on("/get_temp", HTTP_GET, server_handler_get_temp);
  server.on("/get_pid", HTTP_GET, server_handler_get_pid);
  server.on("/set_pid", HTTP_POST, server_handler_set_pid);

  server.begin();
  Serial.println("servidor iniciado");
}

void loop() {
  server.handleClient();

  temp_value = get_temp();
  
  pid_input = temp_value;
  pid.Compute();

#if FW_CONFIG_PERIODIC_DEBUG_MS > 0
  if (millis() - last_millis > FW_CONFIG_PERIODIC_DEBUG_MS) {
    Serial.printf("%.2f,%.2f\n", pid_input, pid_output / FW_CONFIG_PID_OUT_LIMIT_MAX);
    last_millis = millis();
  }
#endif
}

static double get_temp(void) {
  double t = temp_sensor.readTempC();
  if (isnan(t) || isinf(t)) {
    t = temp_value;
  }
  return t;
}

static void server_handler_root(void) {
  String response = "Bem vindo - queima-osso v" + String(MY_VERSION);
  server.send(200, "text/html", response);
}

static void server_handler_get_temp(void) {
  String response = "{\"temp\":" + String(temp_value, 2) + "}";
  server.send(200, "text/json", response);
}

static void server_handler_get_pid(void) {
  String response = "{";
  response += "\"setpoint\":" + String(pid_setpoint, 2) + ",";
  response += "\"kp\":" + String(pid_kp, 2) + ",";
  response += "\"ki\":" + String(pid_ki, 2) + ",";
  response += "\"kd\":" + String(pid_kd, 2) + ",";
  response += "}";
  server.send(200, "text/json", response);
}

static void server_handler_set_pid(void) {
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, server.arg("plain"));

  if (error) {
    server.send(200, "text/json", FW_CONFIG_HTTP_RESP_ERROR);
    return;
  }

  #define JSON_GET(dst, key) if (doc.containsKey(key)) dst = doc[key]

  JSON_GET(pid_setpoint, "setpoint");
  JSON_GET(pid_kp, "kp");
  JSON_GET(pid_ki, "ki");
  JSON_GET(pid_kd, "kd");

  pid.SetTunings(pid_kp, pid_ki, pid_kd);
  
  server.send(200, "text/json", FW_CONFIG_HTTP_RESP_OK);
}
