#include "contiki.h"
#include "net/routing/routing.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"


#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO


#define WITH_SERVER_REPLY  1
#define UDP_CLIENT_PORT 8765
#define UDP_SERVER_PORT 5678


static struct simple_udp_connection udp_conn;


PROCESS(udp_server_process, "UDP server");
AUTOSTART_PROCESSES(&udp_server_process);
/*---------------------------------------------------------------------------*/
static void
udp_rx_callback(struct simple_udp_connection *c,
        const uip_ipaddr_t *sender_addr,
        uint16_t sender_port,
        const uip_ipaddr_t *receiver_addr,
        uint16_t receiver_port,
        const uint8_t *data,
        uint16_t datalen)
{
 char buf[16];
 int len;
 int temp_c;
 int temp_f;


 /* Copiar datos recibidos y terminar en '\0' */
 len = datalen < (sizeof(buf) - 1) ? datalen : (sizeof(buf) - 1);
 memcpy(buf, data, len);
 buf[len] = '\0';


 /* Temperatura recibida en Celsius */
 temp_c = atoi(buf);


 /* Conversión simplificada: F = 2*C + 32 (como en el ejemplo) */
 temp_f = temp_c * 2 + 32;


 /* Mensajes como en la captura */
 LOG_INFO(" Info recibida del nodo: '%d' grados\n", temp_c);


 LOG_INFO(" Direccion = ");
 LOG_INFO_6ADDR(sender_addr);
 LOG_INFO_("\n");


 LOG_INFO(" TEMPERATURA RECIBIDA = %d\n", temp_c);


 LOG_INFO(" Enviando Temperatura en Fahrenheit = %d\n", temp_f);


 LOG_INFO(" Destino = ");
 LOG_INFO_6ADDR(sender_addr);
 LOG_INFO_("\n");


#if WITH_SERVER_REPLY
 /* Enviar la temperatura en Fahrenheit al cliente */
 snprintf(buf, sizeof(buf), "%d", temp_f);
 simple_udp_sendto(&udp_conn, buf, strlen(buf), sender_addr);
#endif /* WITH_SERVER_REPLY */
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_server_process, ev, data)
{
 PROCESS_BEGIN();


 /* Inicializar como root del DAG RPL */
 NETSTACK_ROUTING.root_start();


 /* Inicializar conexión UDP */
 simple_udp_register(&udp_conn, UDP_SERVER_PORT, NULL,
                     UDP_CLIENT_PORT, udp_rx_callback);


 PROCESS_END();
}
/*---------------------------------------------------------------------------*/
