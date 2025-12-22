#ifndef PROJECT_CONF_H_
#define PROJECT_CONF_H_

/* PANID distinto al resto, > 0x0700 */
//#undef IEEE802154_CONF_PANID
#define IEEE802154_CONF_PANID 0x07A5   

/* Canal 802.15.4 (entre 11 y 26, distinto si queréis evitar interferencias) */
//#undef IEEE802154_CONF_DEFAULT_CHANNEL
#define IEEE802154_CONF_DEFAULT_CHANNEL 25

#endif /* PROJECT_CONF_H_ */
