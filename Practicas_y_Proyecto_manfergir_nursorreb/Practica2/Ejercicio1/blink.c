/*
 * Copyright (c) 2006, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */

/**
 * \file
 *         A very simple Contiki application showing how Contiki programs look
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki.h"
#include "dev/leds.h"

#include <stdio.h> /* For printf() */

/*
LO distinto de esta práctica. Tenemos que declarar una variable global para identificar el 
evento que nnosotros vamos a usar para llamar a los parpadeos de los links (ya que no es un)
eventoque contiki conozca como los que hemos usado anteriormente
*/
static process_event_t evento_arranque;

/*---------------------------------------------------------------------------*/
PROCESS(parpadeo_1_process, "Parpadeo LED 1 cada 2 segundos");
PROCESS(parpadeo_2_process, "Parpadeo LED 2 cada 4 segundos");
PROCESS(timer_process, "Proceso de arranque (timer de 3 segundos)");
AUTOSTART_PROCESSES(&parpadeo_1_process, &parpadeo_2_process, &timer_process);

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(parpadeo_1_process, ev, data)
{
  // creamos una estructura tipo etimer para el contador de 2 segundos
  static struct etimer timer_led1; 

  PROCESS_BEGIN();

  // Esperamos el evento 
  PROCESS_WAIT_EVENT_UNTIL(ev==evento_arranque);
  printf("Tarea 1 iniciada, LED verde \n");

  while (1)
  {
    etimer_set(&timer_led1, 2*CLOCK_SECOND);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer_led1));
    leds_toggle(LEDS_GREEN);
    printf("Cambiando LED verde\n");

    etimer_reset(&timer_led1);
  }
  
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

PROCESS_THREAD(parpadeo_2_process, ev, data)
{
  // creamos una estructura tipo etimer para el contador de 4 segundos
  static struct etimer timer_led2; 

  PROCESS_BEGIN();

  // Esperamos el evento 
  PROCESS_WAIT_EVENT_UNTIL(ev==evento_arranque);
  printf("Tarea 2 iniciada, LED azul \n");


  while (1)
  {
    etimer_set(&timer_led2, 4*CLOCK_SECOND);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer_led2));
    leds_toggle(LEDS_YELLOW); // En lla placa se vee azul
    printf("Cambiando LED azul\n");

    etimer_reset(&timer_led2);
  }
  
  PROCESS_END();
}

PROCESS_THREAD(timer_process, ev, data)
{
  static struct etimer time_init;

  PROCESS_BEGIN();

  // Aquí es donde se solicita que se aloje el evento que nosotros mismos hemos creado
  evento_arranque = process_alloc_event();

  etimer_set(&time_init, 3*CLOCK_SECOND);
  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&time_init));
  printf("Timer de arranque espirado, lanzando evento a las tareas \n");

  // No se hace un poll, sino que se avisa a todos los eventos a la vez
  process_post(PROCESS_BROADCAST, evento_arranque, NULL);

  PROCESS_END();
}