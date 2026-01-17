# Práctica 4 – Contiki-NG: Monitorización de la Información

En la carpeta actual se encuentran los archivos correspondientes a la **Práctica 4**, que incluye los **Ejercicios 1 y 2**.

La práctica está organizada en dos subcarpetas:

- **ej1/** 
  Contiene los archivos del **Ejercicio 1**, en el que se lee el sensor interno de temperatura de la tarjeta **Nordic NRF52840** y se envía la medida en **grados Fahrenheit** al sistema de monitorización mediante **MQTT**, utilizando el topic `temp_F`.

- **ej2/** 
  Contiene los archivos del **Ejercicio 2**, en el que se amplía la aplicación para enviar:
  - la temperatura en **grados centígrados** (`temp_c`),
  - la temperatura en **grados Fahrenheit** (`temp_F`),
  - y el estado del pulsador de la tarjeta (`switch`),
  representándose todas las medidas en **Grafana**.

Cada una de las carpetas (`ej1` y `ej2`) presenta la **misma estructura interna**, que incluye:
- una carpeta `docker/` con la configuración del sistema de monitorización (Mosquitto, MQTT exporter, Prometheus y Grafana),
- una carpeta `temp/` con el código del firmware para la NRF52840,
- y una carpeta `rpl-udp/` con código de prácticas anteriores utilizado como referencia.

---

Trabajo realizado por 
**Manuel Fernández Giráldez** y **Nuria Sorrentino Rebull**

