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
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_DBG

#define UDP_CLIENT_PORT 8765
#define UDP_SERVER_PORT 5678

/* Variable para hacer test local sin dos dongle */
#define MODO_TEST_LOCAL 1

#if MODO_TEST_LOCAL == 0
  static struct simple_udp_connection udp_conn;
#endif

/* Definición de la estructura de la trama */
typedef struct {
    uint8_t start_flag;   // Byte 0: 0x55
    uint8_t frame_id;     // Byte 1: 0x01
    uint8_t node_id;      // Byte 2: ID nodo cliente
    uint8_t unit;         // Byte 3: 0x01 (C) o 0x02 (F)
    int16_t value;        // Byte 4-5: Temperatura en F12.4
    uint8_t alarm_type;   // Byte 6: 0x01
    uint8_t alarm_status; // Byte 7: 0x00 o 0x01
} __attribute__((packed)) metric_msg_t;

/* Para hacer las pruebas, lo que hacemos es crear una trama de test que contenga lo mismo que una trama del cli */
static uint8_t test_frame[] = { 
    0x55,       
    0x01,       
    0xAA,       
    0x01,      
    0x6C, 0x01, 
    0x01,       
    0x00        
};

PROCESS(udp_server_process, "UDP server");
AUTOSTART_PROCESSES(&udp_server_process);
/*---------------------------------------------------------------------------*/
static void procesar_trama(const uint8_t *data, uint16_t len) 
{
  metric_msg_t msg;
  int16_t parte_entera;
  int32_t parte_decimal;
  int32_t remainder;

  if(len != sizeof(metric_msg_t)) {
    LOG_INFO("Error: Longitud incorrecta (%d bytes).\n", len);
    return;
  }

  memcpy(&msg, data, sizeof(metric_msg_t));

  if(msg.start_flag != 0x55) {
    LOG_INFO("Error: Flag invalido (0x%02X)\n", msg.start_flag);
    return;
  }

  parte_entera = msg.value / 16;
  
  remainder = (int32_t)(msg.value % 16);
  if(remainder < 0) remainder = -remainder;
  
  parte_decimal = (remainder * 10000) / 16;

  printf("%d;%s;%d.%04ld;%d;%d\n", 
         msg.node_id, 
         (msg.unit == 0x01) ? "C" : "F",
         parte_entera, (long)parte_decimal,
         msg.alarm_type, 
         msg.alarm_status);
}

#if MODO_TEST_LOCAL == 0
  static void udp_rx_callback(struct simple_udp_connection *c,
          const uip_ipaddr_t *sender_addr,
          uint16_t sender_port,
          const uip_ipaddr_t *receiver_addr,
          uint16_t receiver_port,
          const uint8_t *data,
          uint16_t datalen)
  {
    /* Simplemente le pasamos los datos a nuestra función de procesado */
    LOG_INFO("Recibido paquete UDP de ");
    LOG_INFO_6ADDR(sender_addr);
    LOG_INFO_("\n");
    
    procesar_trama(data, datalen);
  }
#endif
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_server_process, ev, data) {
  /* Timer solo necesario para el modo test */
  static struct etimer timer_simulacion;

  PROCESS_BEGIN();

#if MODO_TEST_LOCAL == 0
  /* --- MODO REAL (CLASE) --- */
  /* Inicializamos la red como ROOT y abrimos el puerto UDP */
  
  NETSTACK_ROUTING.root_start();
  
  simple_udp_register(&udp_conn, UDP_SERVER_PORT, NULL,
                      UDP_CLIENT_PORT, udp_rx_callback);
                      
  LOG_INFO("--- SERVIDOR UDP ONLINE (Esperando Clientes) ---\n");

  /* El servidor se queda aquí esperando eventos de red infinitamente */
  while(1) {
    PROCESS_WAIT_EVENT();
  }

#else
  /* --- MODO SIMULACIÓN (CASA) --- */
  /* No iniciamos red para evitar bloqueos. Simulamos recepción local */
  
  LOG_INFO("--- SERVIDOR MODO TEST LOCAL (Sin Red) ---\n");
  LOG_INFO("Simulando datos del array estatico...\n");

  etimer_set(&timer_simulacion, 5 * CLOCK_SECOND);

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer_simulacion));

    /* Llamamos a la lógica como si hubiera llegado un paquete */
    procesar_trama(test_frame, sizeof(test_frame));

    etimer_reset(&timer_simulacion);
  }
#endif

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
