#ifndef __FW_CONFIG_H__
#define __FW_CONFIG_H__

#define FW_CONFIG_TEST_XPTEC              1

// Configurações padrão da rede wifi AP
#define FW_CONFIG_AP_SSID                 "queima-osso"
#define FW_CONFIG_AP_PASS                 "@queima123osso!"

// Permite habilitar/desabilitar o uso de senha na rede wifi AP
#define FW_CONFIG_AP_PASS_EN              0

// Define o período de envio das mensagens de debug
// Para desativar essa funcionalidade basta definir como 0
#define FW_CONFIG_PERIODIC_DEBUG_MS       1000

// Configuração padrão para o PID
#define FW_CONFIG_PID_KP                  40.0
#define FW_CONFIG_PID_KI                  1.0
#define FW_CONFIG_PID_KD                  3.0
#define FW_CONFIG_PID_SETPOINT            60.0

// Válores máximos de saída do PID
// Esses valores foram definidos empiricamente usando o osciloscópio para ver onde
// o controle da onda é estável
#define FW_CONFIG_PID_OUT_LIMIT_MIN       10
#define FW_CONFIG_PID_OUT_LIMIT_MAX       240

// Respostas padrão para o servidor HTTP
#define FW_CONFIG_HTTP_RESP_OK            "{\"success\":1}"
#define FW_CONFIG_HTTP_RESP_ERROR         "{\"success\":0}"

#endif
