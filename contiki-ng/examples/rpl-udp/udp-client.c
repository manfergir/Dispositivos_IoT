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

/* Button HAL */
#include "dev/button-hal.h"

#define WITH_SERVER_REPLY  1
#define UDP_CLIENT_PORT 8765
#define UDP_SERVER_PORT 5678

/* Se fija el envio cada 5 segundos */
#define SEND_INTERVAL      (5 * CLOCK_SECOND)


/* Unidad */
#define UNIT_C 0x01
#define UNIT_F 0x02

static struct simple_udp_connection udp_conn;
static uint32_t rx_count = 0;

/* Por defecto Celsius */
static uint8_t unit = UNIT_C; 

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
 char buf[16];
 int len;
 int temp_f;


 /* Copiar datos y terminar en '\0' */
 len = datalen < (sizeof(buf) - 1) ? datalen : (sizeof(buf) - 1);
 memcpy(buf, data, len);
 buf[len] = '\0';


 temp_f = atoi(buf);


 /* Mensaje como en la captura */
 LOG_INFO(" Temperatura en Fahrenheit calculada por el servidor: '%d' grados.\n", temp_f);


 rx_count++;
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_client_process, ev, data)
{
  
 static struct etimer periodic_timer;
 static char str[32];
 uip_ipaddr_t dest_ipaddr;
 static uint32_t tx_count;
 static uint32_t missed_tx_count;


 PROCESS_BEGIN();


 /* Inicializar conexión UDP */
 simple_udp_register(&udp_conn, UDP_CLIENT_PORT, NULL,
                     UDP_SERVER_PORT, udp_rx_callback);


 etimer_set(&periodic_timer, random_rand() % SEND_INTERVAL);

 while(1) {
   PROCESS_YIELD();

    /* Botón: alterna unidad */
    if(ev == button_hal_press_event) {
      unit = (unit == UNIT_C) ? UNIT_F : UNIT_C;
      LOG_INFO("Botón pulsado -> unidad ahora: %s\n", (unit == UNIT_C) ? "Celsius" : "Fahrenheit");
    }


       /* Envío cada 5 segundos */
    if(etimer_expired(&periodic_timer)) {

      if(NETSTACK_ROUTING.node_is_reachable() &&
         NETSTACK_ROUTING.get_root_ipaddr(&dest_ipaddr)) {

        int temp_c;

        tx_count++;

        if(tx_count % 10 == 0) {
          LOG_INFO("Tx/Rx/MissedTx: %" PRIu32 "/%" PRIu32 "/%" PRIu32 "\n",
                   tx_count, rx_count, missed_tx_count);
        }

        /* Valores de prueba como tenías */
        if(tx_count % 2 == 1) {
          temp_c = 42;
          LOG_INFO("Enviando temperatura interna en Celsius = %d\n", temp_c);
        } else {
          temp_c = 27;
          LOG_INFO("Enviando temperatura externa en Celsius = %d\n", temp_c);
        }

        /* Payload texto por ahora */
        snprintf(str, sizeof(str), "%d", temp_c);
        simple_udp_sendto(&udp_conn, str, strlen(str), &dest_ipaddr);

      } else {
        LOG_INFO("Not reachable yet\n");
        if(tx_count > 0) {
          missed_tx_count++;
        }
      }

      /* Reset fijo */
      etimer_reset(&periodic_timer);
    }
  }


 PROCESS_END();
}
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
