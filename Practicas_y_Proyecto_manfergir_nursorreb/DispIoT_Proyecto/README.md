# Sistema de Monitorización IoT con Contiki-NG y Docker

En este proyecto hemos diseñado e implementado un sistema completo de monitorización IoT, integrando nodos sensores basados en **Contiki-NG** (nRF52840) con una infraestructura de visualización que hemos desplegado mediante **Docker** (Mosquitto, Prometheus, Grafana).

## 1. Obtención del Código
Hemos alojado el código fuente en nuestro repositorio. Puedes clonarlo o descargar la última versión de la rama `main` desde el siguiente enlace:

* **Repositorio:** [https://github.com/manfergir/Dispositivos_IoT](https://github.com/manfergir/Dispositivos_IoT)

### Estructura de nuestro proyecto
* `Proyecto/`: Hemos ubicado el código fuente en esta carpeta, en dos rutas: 
    * `Proyecto/udp-client`: contiene la carpeta `build` con los binarios, el código fuente del cliente, el fichero de configuración del proyecto y un Makefile
    * `Proyecto/udp-server`: contiene la carpeta `build` con los binarios, el código fuente del servidor, el fichero de configuración del proyecto y un Makefile
* `docker/`: Aquí hemos incluido los ficheros de despliegue (`docker-compose.yml`) y la configuración de los servicios.

## 2. Programación de los Nodos (Firmware)
En las respectivas carpetas `build` dentro de la carpeta de cada nodo se encuentran los binarios listos para ser cargados en cada uno de ellos. No onbstante, si existiera algún problema:

1. **Compilación y Carga:**
   Copia la carpeta `Proyecto/udp-server`y `Proyecto/upd-client` dentro de la misma carpeta (Puedes llamarla como quieras, `Proyecto` por ejemplo) dentro de `contiki-ng/examples`. Esto es primordial ya que los ficheros Makefile de cada uno de los nodos se han configurado para que accedan al fichero Makefile.include desde exáctamente esos niveles de profundidad (../../..). 
   
   El resultado final debería ser `contiki-ng/examples/CarpetaCreada/udp-server` y `contiki-ng/examples/CarpetaCreada/udp-client`. Desde aquí, es cuestión de hacer `make TARGET=...` en cada una de las subcarpetas y listo.

3. **Verificación:**
   Conecta los nodos y verifica mediante terminal serie que nuestro servidor inicia correctamente la red RPL y que el cliente comienza a transmitir datos periódicamente.

## 3. Despliegue de Servicios (Docker)
Hemos automatizado del sistema de visualización mediante contenedores.

1. Accede a nuestro directorio de infraestructura:
   ```bash
   cd docker/

   docker-compose up --build -d

2. AUtomáticamente, al levantar Grafana y Prometheus, dirígite a la sección de Dashboards porque tendrás uno creado y listo para la visualización.




# Trabajo realizado por Manuel Fernández Giráldez y Nuria Sorrentino Rebull