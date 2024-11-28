#include "DHTesp.h"
#include <WiFi.h>
#include <HTTPClient.h>

const int DHT_PIN = 15;

// Configuração do Wi-Fi
const char* ssid = "Girassol";
const char* password = "beijaflor";

// URL do servidor
String serverURL = "https://ingestion.edgeimpulse.com/api/training/data";

// Instância do sensor
DHTesp dhtSensor;

void setup() {
  Serial.begin(115200);
  dhtSensor.setup(DHT_PIN, DHTesp::DHT22);
  Serial.println("Sensor conectado com sucesso.");

  // Conectar ao Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Conectando ao WiFi...");
    delay(2000);
  }
  Serial.print("[WIFI] Connected: ");
  Serial.println(WiFi.localIP());

}

void loop() {
  
  // Leitura do sensor
  TempAndHumidity  data = dhtSensor.getTempAndHumidity();
  if (isnan(data.temperature) || isnan(data.humidity)) {
    Serial.println("Erro na leitura do sensor");
    delay(2000);
    return;
  }

  Serial.println("Temp: " + String(data.temperature, 2) + "°C");
  Serial.println("Humidity: " + String(data.humidity, 1) + "%");

  //Verifica conexão Wi-Fi antes de enviar dados
  if (WiFi.status() == WL_CONNECTED) {
    enviarDados(data.temperature, data.humidity);
  } else {
    Serial.println("WiFi desconectado.");
  }
  delay(10000); // Wait for a new reading from the sensor (DHT22 has ~0.5Hz sample rate)


}

void enviarDados(float temperature, float humidity) {
  HTTPClient http;

  http.begin(serverURL);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("x-api-key", "ei_f061925c245eb86432a21b746bcb676c39bde6976adc5318ee5886d42d2b2f31");
  http.addHeader("x-file-name", "esp-data.json");

  // Monta o JSON
  String jsonData = String("{") +
      "\"payload\":{" +
          "\"device_name\":\"WOKWI-SAMPLE\"," +
          "\"device_type\":\"ESP32\"," +
          "\"interval_ms\":20.0," +
          "\"sensors\":[" +
              "{" +
                  "\"name\":\"temperature\"," +
                  "\"units\":\"celsius\"" +
              "}," +
              "{" +
                  "\"name\":\"humidity\"," +
                  "\"units\":\"percent\"" +
              "}" +
          "]," +
          "\"values\":[[" + String(temperature, 2) + "," + String(humidity, 1) + "]]" +
      "}," +
      "\"protected\":{" +
          "\"alg\":\"none\"," +
          "\"ver\":\"v1\"" +
      "}," +
      "\"signature\":\"00\"" +
  "}";

  // Envia dados
  int httpResponseCode = http.POST(jsonData);

  // Processa resposta
  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.println("Resposta do servidor:");
    Serial.println(httpResponseCode);
    Serial.println(response);
  } else {
    Serial.println("Erro ao enviar dados:");
    Serial.println(httpResponseCode);
  }
  http.end();
}
