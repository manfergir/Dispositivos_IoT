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
/*---------------------------------------------------------------------------*/
PROCESS(parpadeo_1_process, "Parpadeo LED 1 cada 2 segundos");
PROCESS(parpadeo_2_process, "Parpadeo LED 2 cada 3 segundos");
PROCESS(timer_process, "Proceso de arranque");
AUTOSTART_PROCESSES(&parpadeo_1_process, &parpadeo_2_process, &timer_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(parpadeo_1_process, ev, data)
{
  // creamos una estructura tipo etimer para el contador de 2 segundos
  static struct etimer timer_2sec; 

  PROCESS_BEGIN();

  // Esperamos el evento 
  PROCESS_WAIT_EVENT_UNTIL(ev==PROCESS_EVENT_POLL);

  while (1)
  {
    etimer_set(&timer_2sec, 2*CLOCK_SECOND);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer_2sec));
    leds_toggle(LEDS_GREEN);
    printf("Cambiaddo LED VERDE\n");
  }
  
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

PROCESS_THREAD(parpadeo_2_process, ev, data)
{
  // creamos una estructura tipo etimer para el contador de 2 segundos
  static struct etimer timer_4sec; 

  PROCESS_BEGIN();

  // Esperamos el evento 
  PROCESS_WAIT_EVENT_UNTIL(ev==PROCESS_EVENT_POLL);

  while (1)
  {
    etimer_set(&timer_4sec, 4*CLOCK_SECOND);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer_4sec));
    leds_toggle(LEDS_YELLOW);
    printf("Cambiaddo LED AZUL\n");
  }
  
  PROCESS_END();
}

PROCESS_THREAD(timer_process, ev, data)
{
  static struct etimer time_3sec;

  PROCESS_BEGIN();

  etimer_set(&time_3sec, 3*CLOCK_SECOND);
  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&time_3sec));;

  process_poll(&parpadeo_1_process);
  process_poll(&parpadeo_2_process);

  PROCESS_END();
}