#include "DHTesp.h"
#include <WiFi.h>
#include <PubSubClient.h>

// Configurações do Wi-Fi
const char* ssid = "Girassol";
const char* password = "beijaflor";

// Configurações do MQTT
const char* mqtt_broker = "broker.hivemq.com"; // Broker público gratuito
const int mqtt_port = 1883;                    // Porta padrão MQTT
const char* mqtt_topic = "esp32/dht22/dados";  // Tópico onde os dados serão publicados

WiFiClient espClient;
PubSubClient mqttClient(espClient);

const int DHT_PIN = 15;       // Pino do DHT22
DHTesp dhtSensor;             // Instância do sensor DHT22

unsigned long previousMillis = 0;
const long interval = 10000;  // Intervalo para envio de dados (10 segundos)

void setup() {
  Serial.begin(115200);
  dhtSensor.setup(DHT_PIN, DHTesp::DHT22);

  // Conectar ao Wi-Fi
  Serial.println("Conectando ao WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("\n[WIFI] Conectado com IP: " + WiFi.localIP().toString());

  // Configurar conexão MQTT
  mqttClient.setServer(mqtt_broker, mqtt_port);
  conectarMQTT();
}

void loop() {
  // Reconectar MQTT se necessário
  if (!mqttClient.connected()) {
    conectarMQTT();
  }
  mqttClient.loop();

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    // Leitura do sensor
    TempAndHumidity data = dhtSensor.getTempAndHumidity();
    if (isnan(data.temperature) || isnan(data.humidity)) {
      Serial.println("Erro na leitura do sensor.");
      return;
    }

    // Exibir dados no monitor serial
    Serial.println("Temp: " + String(data.temperature, 2) + "°C");
    Serial.println("Humidity: " + String(data.humidity, 1) + "%");

    // Publicar dados no tópico MQTT
    publicarDadosMQTT(data.temperature, data.humidity);
  }
}

void conectarMQTT() {
  Serial.println("Conectando ao broker MQTT...");
  while (!mqttClient.connected()) {
    if (mqttClient.connect("ESP32_Client_DHT22")) {
      Serial.println("Conectado ao broker MQTT!");
    } else {
      Serial.print("Falha na conexão. Código: ");
      Serial.println(mqttClient.state());
      delay(2000);
    }
  }
}

void publicarDadosMQTT(float temperature, float humidity) {
  // Montar a mensagem como uma string no formato "temperatura,umidade"
  String mensagem = String(temperature, 2) + "," + String(humidity, 1);
  
  // Publicar mensagem no tópico MQTT
  if (mqttClient.publish(mqtt_topic, mensagem.c_str())) {
    Serial.println("Dados publicados com sucesso: " + mensagem);
  } else {
    Serial.println("Falha ao publicar mensagem.");
  }
}
