#include "contiki.h"
#include "net/routing/routing.h"
#include "random.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"
#include <stdint.h>
#include <inttypes.h>


#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO


#include "contiki.h"
#include "../arch/platform/nrf52840/common/temperature-sensor.h"
#include "lib/sensors.h"
#include "dev/button-hal.h"
#include <stdio.h>

/* Procesos */
PROCESS(temp_reader_process, "Temperature reader");
PROCESS(temp_timer_process,  "Temperature timer");
AUTOSTART_PROCESSES(&temp_reader_process, &temp_timer_process);

/* Estado del “interruptor” */
static uint8_t switch_state = 0;

PROCESS_THREAD(temp_reader_process, ev, data)
{
  static int32_t t_c;     /* raw del sensor (resolución 0.25ºC) */
  static int32_t entero;  /* parte entera en ºC */
  static int32_t decimal; /* parte decimal en ºC (00,25,50,75) */

  /* Para Fahrenheit en centésimas (para imprimir con 2 decimales sin float) */
  static int32_t c_x100;  /* ºC * 100 */
  static int32_t f_x100;  /* ºF * 100 */
  static int32_t f_entero;
  static int32_t f_decimal;

  PROCESS_BEGIN();

  /* Activar el sensor interno de temperatura */
  SENSORS_ACTIVATE(temperature_sensor);

  while(1) {
    /* Espera evento del temporizador O del botón */
    PROCESS_YIELD();

    /* --- Evento botón: toggle --- */
    if(ev == button_hal_press_event) {
      switch_state = !switch_state;
      printf("switch=%u\n", switch_state);
    }

    /* --- Evento temporizador: medir y enviar temps --- */
    if(ev == PROCESS_EVENT_CONTINUE) {

      /* Leer (en ºC raw) */
      t_c = temperature_sensor.value(0);

      /* Tu forma: parte entera y decimal */
      entero  = t_c >> 2;
      decimal = (t_c & 0b11) * 25;

      /* Publicar temp_c con tu formato */
      printf("temp_c=%ld.%02ld\n", (long)entero, (long)decimal);

      /* Calcular Fahrenheit sin floats:
         C*100 = entero*100 + decimal
         F*100 = C*100 * 9/5 + 3200
      */
      c_x100 = entero * 100 + decimal;
      f_x100 = (c_x100 * 9) / 5 + 3200;

      f_entero  = f_x100 / 100;
      f_decimal = f_x100 % 100;

      printf("temp_F=%ld.%02ld\n", (long)f_entero, (long)f_decimal);
    }
  }

  /* (No se alcanzará en este ejemplo) */
  SENSORS_DEACTIVATE(temperature_sensor);
  PROCESS_END();
}

PROCESS_THREAD(temp_timer_process, ev, data)
{
  static struct etimer timer2s;

  PROCESS_BEGIN();

  while(1) {
    /* Temporizador de 2 segundos (si quieres 3 como antes, cambia el 2 por 3) */
    etimer_set(&timer2s, 2 * CLOCK_SECOND);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer2s));

    /* Enviar evento al lector */
    process_post(&temp_reader_process, PROCESS_EVENT_CONTINUE, NULL);
  }

  PROCESS_END();
}