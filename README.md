# MQTT-Arango-Munoz-Rendon

## Comandos:

### Broker:

gcc mqttBroker.c encoder.c connect.c publish.c -o broker -lm

./broker

### CLient:

gcc mqttClient.c encoder.c connect.c publish.c -o client -lm

./client