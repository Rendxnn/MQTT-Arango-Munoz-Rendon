# MQTT-Arango-Munoz-Rendon

## Comandos:

### Broker:

gcc mqttBroker.c encoder.c connect.c publish.c subscribe.c disconnect.c -o broker -lm

./broker

### CLient:

gcc mqttClient.c encoder.c connect.c publish.c subscribe.c disconnect.c -o client -lm

./client