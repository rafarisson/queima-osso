#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebSrv.h>
#include <FS.h>
#include <Adafruit_MCP9808.h>
#include <PID_v1.h>
#include <ArduinoJson.h>

extern "C" {
#include <user_interface.h>
}

#include "fw_config.h"

#define MY_VERSION  1

// Servidor HTTP
AsyncWebServer server(80);

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
static String server_handler_get_fw(void);
static String server_handler_get_temp(void);
static String server_handler_set_pid(AsyncWebServerRequest *request);

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.print("queima-osso v");
  Serial.println(MY_VERSION);

  rst_info *resetInfo;
  resetInfo = ESP.getResetInfoPtr();
  Serial.print("reset: ");
  Serial.println(resetInfo->reason);

  if (!SPIFFS.begin())
    Serial.println("erro ao iniciar spiffs");
  else
    Serial.println("spiffs iniciada");

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

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.min.html", "text/html");
  });
  server.on("/jquery.min.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/jquery.min.js", "text/javascript");
  });
  server.on("/app.min.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/app.min.js", "text/javascript");
  });
  server.on("/style.min.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/style.min.css", "text/css");
  });
  server.on("/get_fw.json", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "application/json", server_handler_get_fw().c_str());
  });
  server.on("/get_temp.json", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "application/json", server_handler_get_temp().c_str());
  });
  server.on("/set_pid.json", HTTP_POST, [](AsyncWebServerRequest *request){
    request->send_P(200, "application/json", server_handler_set_pid(request).c_str());
  });

  server.begin();
  Serial.println("servidor iniciado");
}

void loop() {
  temp_value = get_temp();
  
  pid_input = temp_value;
  pid.Compute();

#if FW_CONFIG_PERIODIC_DEBUG_MS > 0
  if (millis() - last_millis > FW_CONFIG_PERIODIC_DEBUG_MS) {
    Serial.printf("%.2f,%.2f\n", pid_input, FW_CONFIG_PID_OUT_PERCENT(pid_output));
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

static String server_handler_get_fw(void) {
  String response;
  StaticJsonDocument<200> doc;

  doc["temp_min"] = FW_CONFIG_TEMP_MIN;
  doc["temp_max"] = FW_CONFIG_TEMP_MAX;
  doc["setpoint"] = pid_setpoint;
  doc["kp"] = pid_kp;
  doc["ki"] = pid_ki;
  doc["kd"] = pid_kd;

  serializeJson(doc, response);
  return response;
}

static String server_handler_get_temp(void) {
  String response;
  StaticJsonDocument<200> doc;

  doc["temp"] = temp_value;
  doc["output"] = FW_CONFIG_PID_OUT_PERCENT(pid_output);

  serializeJson(doc, response);
  return response;
}

static String server_handler_set_pid(AsyncWebServerRequest *request) {
  if (!request->params())
    return FW_CONFIG_HTTP_RESP_ERROR;
  
  //for (int i = 0; i < request->params(); i++) {
  //  AsyncWebParameter *p = request->getParam(i);
  //  Serial.printf("%s: %s\n", p->name().c_str(), p->value().c_str());
  //}

  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, request->getParam(0)->value().c_str());
  
  #define JSON_GET(dst, key) if (doc.containsKey(key)) dst = doc[key]

  if (error) {
    return FW_CONFIG_HTTP_RESP_ERROR;
  }

  double n_setpoint = pid_setpoint;
  
  JSON_GET(n_setpoint, "setpoint");
  JSON_GET(pid_kp, "kp");
  JSON_GET(pid_ki, "ki");
  JSON_GET(pid_kd, "kd");

  if (n_setpoint != pid_setpoint && (n_setpoint >= FW_CONFIG_TEMP_MIN && n_setpoint <= FW_CONFIG_TEMP_MAX))
    pid_setpoint = n_setpoint;
  pid.SetTunings(pid_kp, pid_ki, pid_kd);

  return FW_CONFIG_HTTP_RESP_OK;
}
