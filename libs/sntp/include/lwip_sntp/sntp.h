#ifndef LWIP_SNTP_H
#define LWIP_SNTP_H

#include <lwip/opt.h>
#include <lwip/ip_addr.h>
#include <stdint.h>

#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/** The maximum number of SNTP servers that can be set */
#ifndef SNTP_MAX_SERVERS
#define SNTP_MAX_SERVERS           1
#endif

/** Set this to 1 to implement the callback function called by dhcp when
 * NTP servers are received. */
#ifndef SNTP_GET_SERVERS_FROM_DHCP
#define SNTP_GET_SERVERS_FROM_DHCP 0//LWIP_DHCP_GET_NTP_SRV
#endif

/* Set this to 1 to support DNS names (or IP address strings) to set sntp servers */
#ifndef SNTP_SERVER_DNS
#define SNTP_SERVER_DNS            0
#endif

/** One server address/name can be defined as default if SNTP_SERVER_DNS == 1:
 * #define SNTP_SERVER_ADDRESS "pool.ntp.org"
 */
uint64_t sntp_get_current_timestamp();

struct sntp_res_t
{
    uint64_t sntp_time;
    uint32_t system_time;
};

sntp_res_t do_sntp_request();

/**
 * Initialize this module.
 * Send out request instantly or after SNTP_STARTUP_DELAY(_FUNC).
 */
void sntp_init(void);

/**
 * Stop this module.
 */
void sntp_stop(void);

int8_t sntp_get_timezone(void);

bool sntp_set_timezone(sint8 timezone);

/**
 * Initialize one of the NTP servers by IP address
 *
 * @param numdns the index of the NTP server to set must be < SNTP_MAX_SERVERS
 * @param dnsserver IP address of the NTP server to set
 */
void sntp_setserver(unsigned char idx, ip_addr_t *addr);

/**
 * Obtain one of the currently configured by IP address (or DHCP) NTP servers
 *
 * @param numdns the index of the NTP server
 * @return IP address of the indexed NTP server or "ip_addr_any" if the NTP
 *         server has not been configured by address (or at all).
 */
ip_addr_t sntp_getserver(u8_t idx);

#if SNTP_SERVER_DNS
/**
 * Initialize one of the NTP servers by name
 *
 * @param numdns the index of the NTP server to set must be < SNTP_MAX_SERVERS
 * @param dnsserver DNS name of the NTP server to set, to be resolved at contact time
 */

void sntp_setservername(u8_t idx, const char *server);

/**
 * Obtain one of the currently configured by name NTP servers.
 *
 * @param numdns the index of the NTP server
 * @return IP address of the indexed NTP server or NULL if the NTP
 *         server has not been configured by name (or at all)
 */
const char *sntp_getservername(u8_t idx);
#endif /* SNTP_SERVER_DNS */

#if SNTP_GET_SERVERS_FROM_DHCP
void sntp_servermode_dhcp(int set_servers_from_dhcp);
#else /* SNTP_GET_SERVERS_FROM_DHCP */
#define sntp_servermode_dhcp(x)
#endif /* SNTP_GET_SERVERS_FROM_DHCP */

#ifdef __cplusplus
}
#endif

#endif /* LWIP_SNTP_H */
