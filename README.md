# queima-osso

Projeto de sous vide.

### Arduino IDE

Usando Arduino IDE v1.8.19

Compatível com Arduino IDE v2.0.3 (apenas gravação SPIFFS não está disponível nessa versão).

### Configurações

A maioria das configurações do ESP8266/Arduino são padrões, exceto o tamanho da flash. 

O tamanho da flash deve considerar o tamanho da SPIFFS necessário (isso depende também do tamanho dos arquivos que são colocados na SPIFFS).
Para esse projeto o tamanho da flash está configurado como `4MB (FS:1MB OTA:~1019KB)`.

### Gravação de dados SPIFFS

Os arquivos da página web são armazenados na SPIFFS.
Para gravação desses dados é necessário utilizar a IDE de programação Arduino na versão 1.8.x, e instalar o plugin 
[ESP8266FS](https://github.com/esp8266/arduino-esp8266fs-plugin/releases).

Tutorial de instalação e gravação disponível [aqui](https://randomnerdtutorials.com/install-esp8266-filesystem-uploader-arduino-ide/).

1. Baixar e extrair a pasta ESP8266FS para o diretório do arduino (ex. `C:\Program Files (x86)\Arduino\tools`);
2. Reiniciar o Arduino IDE (v1.8.x);
3. Criar uma pasta `data` no sketch;
4. Exercutar o gravador em Tools->ESP8266 Sketch Data Upload (todos os arquivos na pasta `data` serão copiados para a SPIFFS).

### Dependências

- Biblioteca ESP8266 (by ESP8266 Community) v3.0.2 [github](https://github.com/esp8266/Arduino)
- Biblioteca ESPAsyncTCP (by dvarrel) v1.2.2 [github](https://github.com/me-no-dev/ESPAsyncTCP)
- Biblioteca ESPAsyncWebSrv ou ESPAsyncWebServer (by dvarrel) v1.2.3 [github](https://github.com/me-no-dev/ESPAsyncWebServer)
- Biblioteca Adafruit MCP9808 (by Adafruit) v2.0.0 [github](https://github.com/adafruit/Adafruit_MCP9808_Library)
- Biblioteca PID (by Brett Beauregard) v1.2.0 [playground](https://playground.arduino.cc/Code/PIDLibrary/) [github](https://github.com/br3ttb/Arduino-PID-Library)
- Biblioteca ArduinoJson (by Benoit Blanchon) v6.19.3 [ArduinoJson](https://arduinojson.org/?utm_source=meta&utm_medium=library.properties)
