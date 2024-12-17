import tensorflow as tf
import numpy as np
import paho.mqtt.client as mqtt

MODEL_PATH = "temperature-preview.lite"

BROKER = "broker.hivemq.com"
PORT = 1883
TOPIC_SUBSCRIBE = "esp32/dht22/dados"

interpreter = tf.lite.Interpreter(model_path=MODEL_PATH)
interpreter.allocate_tensors()

input_details = interpreter.get_input_details()
output_details = interpreter.get_output_details()

def on_connect(client, userdata, flags, rc):
    print(f"Conectado ao broker MQTT com código {rc}")
    client.subscribe(TOPIC_SUBSCRIBE)

def on_message(client, userdata, msg):
    try:
        payload = msg.payload.decode('utf-8')
        print(f"Mensagem recebida: {payload}")

        temperatura, umidade = map(float, payload.split(","))
        print(f"Temperatura: {temperatura:.2f}°C, Umidade: {umidade:.2f}%")

        input_data = np.array([[temperatura, umidade]], dtype=np.float32)
        interpreter.set_tensor(input_details[0]['index'], input_data)
        
        interpreter.invoke()
        
        output_data = interpreter.get_tensor(output_details[0]['index'])
        previsao = output_data[0]

        resultado = "Chuva" if previsao[0] > 0.5 else "Sem chuva"
        print(f"Previsão: {resultado}")

    except Exception as e:
        print(f"Erro durante o processamento: {e}")

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

try:
    print("Conectando ao broker MQTT...")
    client.connect(BROKER, PORT, 60)
    client.loop_forever()
except KeyboardInterrupt:
    print("Desconectando...")
    client.disconnect()
except Exception as e:
    print(f"Erro ao conectar ao broker MQTT: {e}")
