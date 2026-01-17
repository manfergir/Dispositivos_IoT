# Práctica 3 – Redes en Contiki-NG

En la carpeta actual se encuentran los **códigos fuente, archivos de configuración (`project-conf.h`), Makefiles y carpetas `build`** correspondientes a los **ejercicios 1, 2 y 3 de la Práctica 3: Redes en Contiki-NG**.

La práctica está organizada por ejercicios, siguiendo la estructura indicada a continuación.

---

## Estructura de la práctica

### Ejercicio 1 – Nodos reales

Contiene el código y la compilación del despliegue de una red formada por **un nodo servidor y un nodo cliente reales**, utilizando comunicación **UDP sobre IPv6 y RPL**.

En este ejercicio se han configurado explícitamente el **PAN ID** y el **canal IEEE 802.15.4**, tal y como se solicita en el enunciado de la práctica.

---

### Ejercicio 2 – Simulación en COOJA (una red)

Contiene el código y la compilación para el despliegue de **una red simulada en COOJA**, formada por **un nodo servidor de tipo SKY y tres nodos cliente de tipo Z1**.

La red se ha configurado con valores propios de **PAN ID y canal**, distintos a los utilizados en el ejercicio 1.

---

### Ejercicio 3 – Simulación en COOJA (dos redes)

Contiene dos subcarpetas, `redA` y `redB`, cada una correspondiente a **una red independiente simulada en COOJA**. 
Cada red está formada por **un nodo servidor SKY y tres nodos cliente Z1**.

Cada subcarpeta incluye su propio archivo de configuración (`project-conf.h`) y su correspondiente carpeta `build`, utilizando **PAN IDs distintos** para garantizar la separación entre ambas redes.

---

## Compilación

Cada ejercicio (y cada red, en el caso del ejercicio 3) puede compilarse de forma independiente desde su propia carpeta, ya que los **Makefiles están configurados correctamente con la ruta a `Makefile.include`**, permitiendo la ejecución directa de los comandos de compilación sin dependencias externas.

---

## Autores

Trabajo realizado por:

- **Manuel Fernández Giráldez**
- **Nuria Sorrentino Rebull**

