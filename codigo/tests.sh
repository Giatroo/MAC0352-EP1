#!/usr/bin/bash

mosquitto_pub -V mqttv5 -t 'topic1' -p 8001 -m 'Mensagem' &
mosquitto_pub -V mqttv5 -t 'topic1' -p 8001 -m 'Mensagem1' &
mosquitto_pub -V mqttv5 -t 'topic1' -p 8001 -m 'Mensagem2' &
mosquitto_pub -V mqttv5 -t 'topic1' -p 8001 -m 'Mensagem3' &
mosquitto_pub -V mqttv5 -t 'topic1' -p 8001 -m 'Mensagem4' &

