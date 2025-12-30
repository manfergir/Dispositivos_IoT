/*
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
#define LOG_LEVEL LOG_LEVEL_DBG // Modificación del nivel de verbose para poner el debugging

/* Button HAL */
#include "dev/button-hal.h"
#include "dev/leds.h"  /* Driver LED rojo */
#include "../arch/platform/nrf52840/common/temperature-sensor.h" /* Driver del sensor */

#define UDP_CLIENT_PORT 8765
#define UDP_SERVER_PORT 5678

/* Se fija el envio cada 5 segundos */
#define SEND_INTERVAL (5 * CLOCK_SECOND)

/* Se fija el parpadeo del LED cada 2 segundos */
#define BLINK_INTERVAL (CLOCK_SECOND / 2)

/* Contantes para el protocolo */
#define START_FLAG_VAL 0x55
#define FRAME_ID_VAL   0x01
#define NODE_ID        0xAA  /* Identificador a nuestra elección */

#define UNIT_C 0x01
#define UNIT_F 0x02

#define ALARM_TYPE_TEMP 0x01
#define ALARM_ACTIVE    0x01
#define ALARM_INACTIVE  0x00

/* Umbral de alarma en Grados C */
#define TEMP_THRESHOLD_C 23
#define F_MULT 45
#define F_SUMM 3200

/* Definimos nuestra propia estructura de trama que contenga todos los campos que se 
   indican en el enunciado. NOTA: según hemos visto, se recomienda usar __attribute__((packed)) 
   cuando se programan estructuras de tramas para protocolos de comunicación ya que de esta forma
   obligamos al compilador a que no deje huecos vacíos entre variables, es decir, que haga
   padding de forma automática
*/
typedef struct {
    uint8_t start_flag;   // Byte 0: 0x55
    uint8_t frame_id;     // Byte 1: 0x01
    uint8_t node_id;      // Byte 2: ID nodo cliente
    uint8_t unit;         // Byte 3: 0x01 (C) o 0x02 (F)
    int16_t value;        // Byte 4-5: Temperatura en F12.4
    uint8_t alarm_type;   // Byte 6: 0x01
    uint8_t alarm_status; // Byte 7: 0x00 o 0x01
} __attribute__((packed)) metric_msg_t;

static struct simple_udp_connection udp_conn;
// static uint32_t rx_count = 0;

/* Por defecto Celsius */
static uint8_t current_unit = UNIT_C;

/* Para el temporizador de los LEDs */
static struct ctimer blink_timer;
static uint8_t is_blinking = 0;

/*---------------------------------------------------------------------------*/
PROCESS(udp_client_process, "UDP client");
AUTOSTART_PROCESSES(&udp_client_process);
/*---------------------------------------------------------------------------*/
static void udp_rx_callback(struct simple_udp_connection *c,
        const uip_ipaddr_t *sender_addr,
        uint16_t sender_port,
        const uip_ipaddr_t *receiver_addr,
        uint16_t receiver_port,
        const uint8_t *data,
        uint16_t datalen)
{
/* EL enunciado especifica que el servidor NUNCA envía respuesta ya que se considera
que el sistema no es crítico. No obstante, la estructura simple_udp_register necesita
que se le pase como argumento un puntero a funcion de callback para la rx. 

Posiblemente se le de uso en el futuro
*/
}

static void blink_callback(void *ptr) {
  if(is_blinking) {
    leds_toggle(LEDS_RED);
    ctimer_reset(&blink_timer);
  }
  else {
    leds_off(LEDS_RED);
    ctimer_stop(&blink_timer);
  }
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_client_process, ev, data)
{
  
 static struct etimer periodic_timer;
 uip_ipaddr_t dest_ipaddr;

 /* Usamos nuestra estreuctura definida anteriormente */
 static metric_msg_t msg;

 /* Definimos una serie de variables para realizar los cálculos de las temperaturas */
 int32_t raw_value;
 int32_t temp_c_whole; 
 int32_t f_cent; 

/* Para el debugging */
int16_t temp_print;

 /* Auxiar para imprimir la trama e enviar byte a byte, debugging */
 uint8_t *byte_ptr;
 int i = 0;

 PROCESS_BEGIN();

 /* Activación del sensor de temperatura */
 SENSORS_ACTIVATE(temperature_sensor);

 /* Inicializar conexión UDP */
 simple_udp_register(&udp_conn, UDP_CLIENT_PORT, NULL,
                     UDP_SERVER_PORT, udp_rx_callback);

 etimer_set(&periodic_timer, random_rand() % SEND_INTERVAL);

 while(1) {
   PROCESS_YIELD();

    /* Botón: alterna unidad */
    if(ev==button_hal_press_event) {
      current_unit = (current_unit == UNIT_C) ? UNIT_F : UNIT_C;
      LOG_INFO("Botón pulsado, cambio de unidad a: %s\n", (current_unit == UNIT_C) ? "Celsius" : "Fahrenheit");
    }

       /* Envío cada 5 segundos */
    if(etimer_expired(&periodic_timer)) {
      
      /* De momento, para hacer pruebas en local, bypass en la conexión con el servidor */
      /* if(NETSTACK_ROUTING.node_is_reachable() &&
         NETSTACK_ROUTING.get_root_ipaddr(&dest_ipaddr)) {
      */
      {
        raw_value = temperature_sensor.value(0);
        temp_c_whole = raw_value / 4;

        /* Primera parte de la construcción de la trama */
        msg.start_flag = START_FLAG_VAL;
        msg.frame_id = FRAME_ID_VAL;
        msg.node_id = NODE_ID;
        msg.unit = current_unit;
        msg.alarm_type = ALARM_TYPE_TEMP;

        if(temp_c_whole > TEMP_THRESHOLD_C) {
          msg.alarm_status = ALARM_ACTIVE;

          if(!is_blinking) {
            is_blinking = 1;
            blink_callback(NULL);
            ctimer_set(&blink_timer, BLINK_INTERVAL, blink_callback, NULL);
            LOG_INFO("ALARMA: Temperatura mayor de %d, LED activado.\n", TEMP_THRESHOLD_C);
          }
        } 
        else {
          msg.alarm_status = ALARM_INACTIVE;
          
          if(is_blinking) {
            is_blinking = 0;
            leds_off(LEDS_RED);
            ctimer_stop(&blink_timer);
            LOG_INFO("Temperatura normal, LED desactivado.\n");
          }
        }

        if(current_unit==UNIT_C) {
          msg.value = (int16_t)(raw_value * 4);
          temp_print = temp_c_whole;
        }
        else {
          f_cent = (raw_value * F_MULT) + F_SUMM;
          msg.value = (int16_t)((f_cent * 16) /100);
          temp_print = f_cent/100;
        }
        
        /* LOG_INFO (Datos legibles) */
        LOG_INFO("Lectura: %ld (aprox %ld %s) -> F12.4: %d\n", 
                 (long)raw_value, (long)temp_print, (current_unit == UNIT_C) ? "ºC":"ºF", msg.value);

        /* LOG_DBG */
        LOG_DBG("TRAMA HEX (%d bytes): [ ", sizeof(metric_msg_t));
        byte_ptr = (uint8_t *)&msg;

        for(i = 0; i < sizeof(metric_msg_t); i++) {
            LOG_DBG_("%02X ", byte_ptr[i]);
        }
        LOG_DBG_("]\n");

        LOG_INFO("Enviando trama a ");
        LOG_INFO_6ADDR(&dest_ipaddr);
        LOG_INFO_("\n");

        /* Transmisión */
        simple_udp_sendto(&udp_conn, &msg, sizeof(msg), &dest_ipaddr);

      } 

      /*
      else {
        LOG_INFO("Not reachable yet\n");  
      }
      */

      /* COnfiguramos el timer a 5 segundos */
    etimer_set(&periodic_timer, SEND_INTERVAL);    
  }
}

 SENSORS_DEACTIVATE(temperature_sensor);
 PROCESS_END();
}