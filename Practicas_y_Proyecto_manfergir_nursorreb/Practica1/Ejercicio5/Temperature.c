#include "contiki.h"
#include "../arch/platform/nrf52840/common/temperature-sensor.h"
#include <stdio.h>

PROCESS(temp_reader_process, "Temperature reader");
PROCESS(temp_timer_process,  "Temperature timer");
AUTOSTART_PROCESSES(&temp_reader_process, &temp_timer_process);


PROCESS_THREAD(temp_reader_process, ev, data)
{
  static int32_t t_c;
  static int32_t entero;
  static int32_t decimal;

  PROCESS_BEGIN();

  // Activación del sensor interno de temp
  SENSORS_ACTIVATE(temperature_sensor);

  while(1) {
    // Esperamos rx evento
    PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_CONTINUE);

    // Se lee la temperatura
    t_c = temperature_sensor.value(0);

    // Almacenamos la parte entera
    entero = t_c >> 2;

    // ALmacenamos la parte decimal
    decimal = (t_c & 0b11) * 25;


    printf("%ld.%02ld\n", (long)entero, (long)decimal);
  }
  SENSORS_DEACTIVATE(temperature_sensor);
  PROCESS_END();
}

PROCESS_THREAD(temp_timer_process, ev, data)
{
  static struct etimer timer3s;

  PROCESS_BEGIN();

  while(1) {
    // TImer 3 secs
    etimer_set(&timer3s, 3 * CLOCK_SECOND);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer3s));

    // Envío asíncrono
    process_post(&temp_reader_process, PROCESS_EVENT_CONTINUE, NULL);
  }

  PROCESS_END();
}
