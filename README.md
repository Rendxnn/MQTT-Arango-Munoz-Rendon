# MQTT-Arango-Munoz-Rendon 


## Comandos: 


### Broker: 


gcc mqttBroker.c encoder.c connect.c publish.c disconnect.c -o broker -lm 


./broker 


### CLient: 


gcc mqttClient.c encoder.c connect.c publish.c disconnect.c -o client -lm 


./client 


<h1>Introducción</h1> 

El propósito de este proyecto es la implementación del protocolo MQTT en el lenguaje de programación C, con el fin de lograr la comunicación entre dos maquinas mediante este protocolo. 

El protocolo MQTT(Message Queuing Telemetry Transport) es un protocolo de mensajería que ofrece un tipo de conexión Cliente-Servidor con un modelo publicación/suscripción. Gracias a su sistema de formato binario y encabezado de pocos bytes, hace este protocolo particularmente bueno en entornos de bajo ancho de banda. Fue diseñado por Andy Standford-Clark y Arlen Nipper en 1999 para conectar sistemas de telemetría en oleoductos por satélite.  

La versión implementación del protocolo MQTT es: versión 3.1.1(MQTT3.1.1). La documentación utilizada en esta implementación es la siguiente: http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.pdf; Con esta conseguimos entender los fundamentos clave del protocolo, una vez entendidos se paso a la implementación en C. 

C es un lenguaje de programación, originalmente desarrollado por Dennis Ritchie en 1972, se trata de un lenguaje de tipos de datos estáticos, débilmente tipado. C es un lenguaje de bajo nivel lo que permite manipular Bytes y proporciona control directo sobre la memoria, haciendo de C un lenguaje eficiente para la implementación de este protocolo.  

Es importante destacar que, la implementación exacta del protocolo no fue posible, algunas cuestiones que se discutirán mas adelante se trabajaron de manera diferente o no se logro su implementación. De igual forma se hablara de lo logrado y su estado actual. 


<h1>Desarrollo</h1> 

El presente proyecto se desarrolló sobre la especificación del protocolo MQTT3.1.1 debido a que la versión mas reciente (MQTT5) implica la implementación de una serie de propiedades y atributos  

que se consideraron innecesarios para el correcto entendimiento e implementación del protocolo. 


En la implementación del broker hicimos uso de un API SOCKET en este caso elegimos SOCK_STREAM. Este tipo de socket es orientado a la conexión y proporciona un flujo bidireccional de datos, que está secuenciado, es fiable y no está duplicado. Este tipo de socket se implementa en TCP. Estas características lo hacen fundamental para la implementación del protocolo MQTT que se ejecuta sobre TCP. En este usamos la librería arpa/inet.h que proporciona funciones y estructuras de datos que se utilizan para trabajar con direcciones de red en el contexto de sockets de red en sistemas operativos tipo Unix, como Linux. Estas funciones son parte de la API de sockets de Berkeley, que es una interfaz estándar para la programación de red en sistemas Unix. 


Se hizo uso de la estructura de datos struct propia de C para el fixheader, subscribe y la concurrencia. El uso de struct en C permite crear tipos de datos personalizados que pueden contener diferentes tipos de datos, lo que facilita la organización y manipulación de datos en la implementación. 


En el desarrollo del proyecto fue clave el uso de las variables tipo char que en C al declararse separan 1byte en memoria, esto es realmente eficiente a la hora de implementar el protocolo ya que se necesita manejar bytes en la creación del paquete. 

Adicional se debe modificar ciertos bits dentro de estos bytes para temas de QoS y flags propios del protocolo. C permite desplazar bits y modificarlos mediante el uso de compuertas lógicas, haciendo esta tarea de modificación de bits posible y relativamente fácil.  


La concurrencia que proporciona la conexión de múltiples clientes se trabajó con la biblioteca pthread que proporciona soporte para la creación y el manejo de hilos de ejecución en un programa. El nombre pthread proviene de "POSIX Threads", que son una implementación de la norma POSIX (Portable Operating System Interface) para programación de hilos. Gracias a estos hilos se implemento la concurrencia. 


El manejo de recepción y re-envío de mensajes se implementó usando un arreglo de estructuras, las cuales poseen la información del tema (topic) de subscripción y el socket del cliente suscrito a dicho tema. De esta forma, en el momento en el que el broker recibe un mensaje publish proveniente de un cliente, recorre la lista existente en busca de estructura cuyo tema asociado se igual al recién publicado, y envía al socket asociado a la estructura el mensaje que acaba de recibir. 



### Logrados y No Logrados 


#### Logrados 


- Conexión de cliente con servidor 


- implementación completa de lectura y escritura de mensajes CONNECT, CONNACK, PUBLISH, PUBACK, DISCONNECT y lectura de mensajes SUBSCRIBE. 


- Implementación concurrente del broker MQTT. 


- Implementación concurrente de cliente MQTT. 


- Escritura del Log 


- Despliegue en maquina AWS 


### No Logrados 


- Envío de mensajes a clientes subscritos a un tópico determinado. 


- Construcción de mensaje subscribe 


- Generación automática de clientId y packetIdentifier.



<h1>Conclusiones</h1> 

Durante el desarrollo del proyecto se evidencio gran avance en las competencias de diseño y desarrollo de aplicaciones concurrentes en red. Se empleo la API socket,lo que permitió la comunicación entre el cliente y el servidor. 

Aunque se lograron avances significativos, no se alcanzaron completamente todos los objetivos. Esto debido a varios factores tales como la complejidad del diseño, los desafíos asociados a la escritura del protocolo y complejidad de la herramienta utilizada para la implementación (C). 


De acuerdo con la hipótesis, C demostró ser particularmente útil en la implementación de este protocolo. Gracias a su capacidad para modificar espacios de memoria y bytes directamente, siendo así, bastante eficiente a la hora de estructurar y leer los mensajes. Aun así uno de los grandes desafíos fue entender y apropiarse de la herramienta de manera correcta, su gran capacidad viene acompañada de un grado de complejidad considerable que entorpeció nuestro desarrollo debido a una falta de habilidad técnica. En segundo lugar, la gestión del tiempo, este fue un gran desafío, a pesar de que en la implementación se logró recrear los paquetes de control PUBLISH, SUBSCRIBE, CONNECT Y DISCONECT (Con sus respectivos ACK), debido a la poca gestión del tiempo no se logró la implementación de más paquetes de control y características propias del protocolo. 


En conclusión, a pesar de los desafíos iniciales, se presenta una implementación fiable, aunque incompleta del protocolo MQTT donde se podría explorar la implementación de características adicionales del protocolo. Se espera para próximos proyectos tener un mayor grado de habilidad con la herramienta y una administración del tiempo más avanzada. 


<h1>Referencias</h1> 

https://mqtt.org/mqtt-specification/ 