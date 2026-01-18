#include "contiki.h"
#include "../arch/platform/nrf52840/common/temperature-sensor.h"
#include <stdio.h>


/* Declaración de procesos */
PROCESS(temp_reader_process, "Temperature reader");
PROCESS(temp_timer_process,  "Temperature timer");
AUTOSTART_PROCESSES(&temp_reader_process, &temp_timer_process);

PROCESS_THREAD(temp_reader_process, ev, data)
{
  static int32_t raw_value;
  static int32_t f_centimas; // Valor en Farenheit multiplicado por 100
  static int32_t entero_f;
  static int32_t decimal_f;

  PROCESS_BEGIN();

  /* Activar el sensor interno de temperatura */
  SENSORS_ACTIVATE(temperature_sensor);

  while(1) {
    /* Espera el evento del temporizador */
    PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_CONTINUE);

    raw_value = temperature_sensor.value(0);
    
    f_centimas = (raw_value * 45) + 3200;

    entero_f = f_centimas / 100;
    decimal_f = f_centimas % 100;

    printf("%ld.%02ld\n", (long)entero_f, (long)decimal_f);
  }

  SENSORS_DEACTIVATE(temperature_sensor);
  PROCESS_END();
}


PROCESS_THREAD(temp_timer_process, ev, data)
{
 static struct etimer timer3s;


 PROCESS_BEGIN();


 while(1) {
   /* Temporizador de 3 segundos */
   etimer_set(&timer3s, 3 * CLOCK_SECOND);
   PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer3s));


   /* Enviar evento asíncrono al lector */
   process_post(&temp_reader_process, PROCESS_EVENT_CONTINUE, NULL);
 }


 PROCESS_END();
}


