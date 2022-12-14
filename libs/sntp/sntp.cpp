#include <tos/semaphore.hpp>
#include <lwip_sntp/sntp.h>
#include "lwip/opt.h"
#include "lwip/timers.h"
#include "lwip/udp.h"
#include "lwip/dns.h"
#include "lwip/ip_addr.h"
#include "lwip/pbuf.h"
#include <lwip_sntp/time.h>

static sntp_res_t req_result;
static tos::semaphore req_sync {0};

static_assert(LWIP_UDP, "");
extern "C"
{
#include "osapi.h"
#include "os_type.h"
#include <user_interface.h>

int ets_printf(const char *format, ...)  __attribute__ ((format (printf, 1, 2)));
//int os_printf_plus(const char* format, ...)  { return 0; }
/**
 * SNTP_DEBUG: Enable debugging for SNTP.
 */
#ifndef SNTP_DEBUG
#define SNTP_DEBUG                  LWIP_DBG_ON
#endif

/** SNTP server port */
#ifndef SNTP_PORT
#define SNTP_PORT                   123
#endif

/** Set this to 1 to allow config of SNTP server(s) by DNS name */
#ifndef SNTP_SERVER_DNS
#define SNTP_SERVER_DNS             0
#endif

/** Handle support for more than one server via NTP_MAX_SERVERS,
 * but catch legacy style of setting SNTP_SUPPORT_MULTIPLE_SERVERS, probably outside of this file
 */
#ifndef SNTP_SUPPORT_MULTIPLE_SERVERS
#if SNTP_MAX_SERVERS > 1
#define SNTP_SUPPORT_MULTIPLE_SERVERS 1
#else /* NTP_MAX_SERVERS > 1 */
#define SNTP_SUPPORT_MULTIPLE_SERVERS 0
#endif /* NTP_MAX_SERVERS > 1 */
#else /* SNTP_SUPPORT_MULTIPLE_SERVERS */
/* The developer has defined SNTP_SUPPORT_MULTIPLE_SERVERS, probably from old code */
#if SNTP_MAX_SERVERS <= 1
#error "SNTP_MAX_SERVERS needs to be defined to the max amount of servers if SNTP_SUPPORT_MULTIPLE_SERVERS is defined"
#endif /* SNTP_MAX_SERVERS <= 1 */
#endif /* SNTP_SUPPORT_MULTIPLE_SERVERS */


/** Sanity check:
 * Define this to
 * - 0 to turn off sanity checks (default; smaller code)
 * - >= 1 to check address and port of the response packet to ensure the
 *        response comes from the server we sent the request to.
 * - >= 2 to check returned Originate Timestamp against Transmit Timestamp
 *        sent to the server (to ensure response to older request).
 * - >= 3 @todo: discard reply if any of the LI, Stratum, or Transmit Timestamp
 *        fields is 0 or the Mode field is not 4 (unicast) or 5 (broadcast).
 * - >= 4 @todo: to check that the Root Delay and Root Dispersion fields are each
 *        greater than or equal to 0 and less than infinity, where infinity is
 *        currently a cozy number like one second. This check avoids using a
 *        server whose synchronization source has expired for a very long time.
 */
#ifndef SNTP_CHECK_RESPONSE
#define SNTP_CHECK_RESPONSE         0
#endif

/** According to the RFC, this shall be a random delay
 * between 1 and 5 minutes (in milliseconds) to prevent load peaks.
 * This can be defined to a random generation function,
 * which must return the delay in milliseconds as u32_t.
 * Turned off by default.
 */
#ifndef SNTP_STARTUP_DELAY
#define SNTP_STARTUP_DELAY          0
#endif

/** If you want the startup delay to be a function, define this
 * to a function (including the brackets) and define SNTP_STARTUP_DELAY to 1.
 */
#ifndef SNTP_STARTUP_DELAY_FUNC
#define SNTP_STARTUP_DELAY_FUNC     SNTP_STARTUP_DELAY
#endif

/** SNTP receive timeout - in milliseconds
 * Also used as retry timeout - this shouldn't be too low.
 * Default is 3 seconds.
 */
#ifndef SNTP_RECV_TIMEOUT
#define SNTP_RECV_TIMEOUT           3000
#endif

/** SNTP update delay - in milliseconds
 * Default is 1 hour.
 */
#ifndef SNTP_UPDATE_DELAY
#define SNTP_UPDATE_DELAY           3600000
#endif
#if (SNTP_UPDATE_DELAY < 15000) && !SNTP_SUPPRESS_DELAY_CHECK
#error "SNTPv4 RFC 4330 enforces a minimum update time of 15 seconds!"
#endif

/** SNTP macro to change system time and/or the update the RTC clock */
#ifndef SNTP_SET_SYSTEM_TIME
#define SNTP_SET_SYSTEM_TIME(sec) ((void)sec)
#endif

/** SNTP macro to change system time including microseconds */
uint8_t sntp_receive_time_size = 2;
#define SNTP_RECEIVE_TIME_SIZE      sntp_receive_time_size
#define SNTP_SET_SYSTEM_TIME_US(sec, us)    sntp_update_rtc(sec, us)

/** SNTP macro to get system time, used with SNTP_CHECK_RESPONSE >= 2
 * to send in request and compare in response.
 */
#ifndef SNTP_GET_SYSTEM_TIME
#define SNTP_GET_SYSTEM_TIME(sec, us)     do { (sec) = 0; (us) = 0; } while(0)
#endif

/** Default retry timeout (in milliseconds) if the response
 * received is invalid.
 * This is doubled with each retry until SNTP_RETRY_TIMEOUT_MAX is reached.
 */
#ifndef SNTP_RETRY_TIMEOUT
#define SNTP_RETRY_TIMEOUT          SNTP_RECV_TIMEOUT
#endif

/** Maximum retry timeout (in milliseconds). */
#ifndef SNTP_RETRY_TIMEOUT_MAX
#define SNTP_RETRY_TIMEOUT_MAX      (SNTP_RETRY_TIMEOUT * 10)
#endif

/** Increase retry timeout with every retry sent
 * Default is on to conform to RFC.
 */
#ifndef SNTP_RETRY_TIMEOUT_EXP
#define SNTP_RETRY_TIMEOUT_EXP      1
#endif

/* the various debug levels for this file */
#define SNTP_DEBUG_TRACE        (SNTP_DEBUG | LWIP_DBG_TRACE)
#define SNTP_DEBUG_STATE        (SNTP_DEBUG | LWIP_DBG_STATE)
#define SNTP_DEBUG_WARN         (SNTP_DEBUG | LWIP_DBG_LEVEL_WARNING)
#define SNTP_DEBUG_WARN_STATE   (SNTP_DEBUG | LWIP_DBG_LEVEL_WARNING | LWIP_DBG_STATE)
#define SNTP_DEBUG_SERIOUS      (SNTP_DEBUG | LWIP_DBG_LEVEL_SERIOUS)

#define SNTP_ERR_KOD                1

/* SNTP protocol defines */
#define SNTP_MSG_LEN                48

#define SNTP_OFFSET_LI_VN_MODE      0
#define SNTP_LI_MASK                0xC0
#define SNTP_LI_NO_WARNING          0x00
#define SNTP_LI_LAST_MINUTE_61_SEC  0x01
#define SNTP_LI_LAST_MINUTE_59_SEC  0x02
#define SNTP_LI_ALARM_CONDITION     0x03 /* (clock not synchronized) */

#define SNTP_VERSION_MASK           0x38
#define SNTP_VERSION                (4/* NTP Version 4*/<<3)

#define SNTP_MODE_MASK              0x07
#define SNTP_MODE_CLIENT            0x03
#define SNTP_MODE_SERVER            0x04
#define SNTP_MODE_BROADCAST         0x05

#define SNTP_OFFSET_STRATUM         1
#define SNTP_STRATUM_KOD            0x00

#define SNTP_OFFSET_ORIGINATE_TIME  24
#define SNTP_OFFSET_RECEIVE_TIME    32
#define SNTP_OFFSET_TRANSMIT_TIME   40

/* number of seconds between 1900 and 1970 */
#define DIFF_SEC_1900_1970         (2208988800UL)

/**
 * SNTP packet format (without optional fields)
 * Timestamps are coded as 64 bits:
 * - 32 bits seconds since Jan 01, 1970, 00:00
 * - 32 bits seconds fraction (0-padded)
 * For future use, if the MSB in the seconds part is set, seconds are based
 * on Feb 07, 2036, 06:28:16.
 */
#ifdef PACK_STRUCT_USE_INCLUDES
#  include "arch/bpstruct.h"
#endif
PACK_STRUCT_BEGIN
#define PACK_STRUCT_FLD_8 PACK_STRUCT_FIELD
struct sntp_msg {
    PACK_STRUCT_FLD_8(u8_t li_vn_mode);
    PACK_STRUCT_FLD_8(u8_t stratum);
    PACK_STRUCT_FLD_8(u8_t poll);
    PACK_STRUCT_FLD_8(u8_t precision);
    PACK_STRUCT_FIELD(u32_t root_delay);
    PACK_STRUCT_FIELD(u32_t root_dispersion);
    PACK_STRUCT_FIELD(u32_t reference_identifier);
    PACK_STRUCT_FIELD(u32_t reference_timestamp[2]);
    PACK_STRUCT_FIELD(u32_t originate_timestamp[2]);
    PACK_STRUCT_FIELD(u32_t receive_timestamp[2]);
    PACK_STRUCT_FIELD(u32_t transmit_timestamp[2]);
} PACK_STRUCT_STRUCT;
PACK_STRUCT_END
#ifdef PACK_STRUCT_USE_INCLUDES
#  include "arch/epstruct.h"
#endif

/* function prototypes */
static void sntp_request(void *arg);

/** The UDP pcb used by the SNTP client */
static struct udp_pcb *sntp_pcb;

static sint8 time_zone = 0;
/** Names/Addresses of servers */
struct sntp_server {
#if SNTP_SERVER_DNS
    char name[32];
#endif /* SNTP_SERVER_DNS */
    ip_addr_t addr;
};

static struct sntp_server sntp_servers[SNTP_MAX_SERVERS];

#if SNTP_SUPPORT_MULTIPLE_SERVERS
/** The currently used server (initialized to 0) */
static u8_t sntp_current_server;
#else /* SNTP_SUPPORT_MULTIPLE_SERVERS */
#define sntp_current_server 0
#endif /* SNTP_SUPPORT_MULTIPLE_SERVERS */

#if SNTP_RETRY_TIMEOUT_EXP
#define SNTP_RESET_RETRY_TIMEOUT() sntp_retry_timeout = SNTP_RETRY_TIMEOUT
/** Retry time, initialized with SNTP_RETRY_TIMEOUT and doubled with each retry. */
static u32_t sntp_retry_timeout;
#else /* SNTP_RETRY_TIMEOUT_EXP */
#define SNTP_RESET_RETRY_TIMEOUT()
#define sntp_retry_timeout SNTP_RETRY_TIMEOUT
#endif /* SNTP_RETRY_TIMEOUT_EXP */

#if SNTP_CHECK_RESPONSE >= 1
/** Saves the last server address to compare with response */
static ip_addr_t sntp_last_server_address;
#endif /* SNTP_CHECK_RESPONSE >= 1 */

#if SNTP_CHECK_RESPONSE >= 2
/** Saves the last timestamp sent (which is sent back by the server)
 * to compare against in response */
static u32_t sntp_last_timestamp_sent[2];
#endif /* SNTP_CHECK_RESPONSE >= 2 */

static uint64 realtime_stamp = 0;

sntp_res_t do_sntp_request()
{
    sntp_request(nullptr);
    req_sync.down();
    return req_result;
}

uint64 sntp_get_current_timestamp() {
    return realtime_stamp;
}

sint8 ICACHE_FLASH_ATTR
sntp_get_timezone(void) {
    return time_zone;
}

bool ICACHE_FLASH_ATTR
sntp_set_timezone(sint8 timezone) {
    if (timezone >= -11 || timezone <= 13) {
        time_zone = timezone;
        return true;
    } else {
        return false;
    }
}

static void ICACHE_FLASH_ATTR
sntp_process(u32_t *receive_timestamp) {
    /* convert SNTP time (1900-based) to unix GMT time (1970-based)
     * @todo: if MSB is 1, SNTP time is 2036-based!
     */

    time_t t = (ntohl(receive_timestamp[0]) - DIFF_SEC_1900_1970);
    auto frac_ms = (uint64_t(ntohl(receive_timestamp[1])) * 1000) >> 32;
    ets_printf("sec: %u frac: %u\n", t, ntohl(receive_timestamp[1]));
    /* change system time and/or the update the RTC clock */
    SNTP_SET_SYSTEM_TIME(t);
    /* display local time from GMT time */
    t += time_zone * 60 * 60;// format GMT + time_zone TIME ZONE
    realtime_stamp = t;
    req_result = sntp_res_t {
        uint64_t (t) * 1000 + frac_ms,
        system_get_time()
    };
    req_sync.up_isr();
    system_os_post(tos::esp82::main_task_prio, 0, 0);
}

/**
 * Initialize request struct to be sent to server.
 */
static void ICACHE_FLASH_ATTR
sntp_initialize_request(struct sntp_msg *req) {
    memset(req, 0, SNTP_MSG_LEN);
    req->li_vn_mode = SNTP_LI_NO_WARNING | SNTP_VERSION | SNTP_MODE_CLIENT;

#if SNTP_CHECK_RESPONSE >= 2
    {
      u32_t sntp_time_sec, sntp_time_us;
      /* fill in transmit timestamp and save it in 'sntp_last_timestamp_sent' */
      SNTP_GET_SYSTEM_TIME(sntp_time_sec, sntp_time_us);
      sntp_last_timestamp_sent[0] = htonl(sntp_time_sec + DIFF_SEC_1900_1970);
      req->transmit_timestamp[0] = sntp_last_timestamp_sent[0];
      /* we send/save us instead of fraction to be faster... */
      sntp_last_timestamp_sent[1] = htonl(sntp_time_us);
      req->transmit_timestamp[1] = sntp_last_timestamp_sent[1];
    }
#endif /* SNTP_CHECK_RESPONSE >= 2 */
}

/**
 * Retry: send a new request (and increase retry timeout).
 *
 * @param arg is unused (only necessary to conform to sys_timeout)
 */
static void ICACHE_FLASH_ATTR
sntp_retry(void*) {
    ets_printf("timed out :(\n");

    //LWIP_DEBUGF(SNTP_DEBUG_STATE, "sntp_retry: Next request will be sent in %" U32_F " ms\n", sntp_retry_timeout);

    /* set up a timer to send a retry and increase the retry delay */
    sys_timeout(sntp_retry_timeout, sntp_request, NULL);

#if SNTP_RETRY_TIMEOUT_EXP
    {
        u32_t new_retry_timeout;
        /* increase the timeout for next retry */
        new_retry_timeout = sntp_retry_timeout << 1;
        /* limit to maximum timeout and prevent overflow */
        if ((new_retry_timeout <= SNTP_RETRY_TIMEOUT_MAX) &&
            (new_retry_timeout > sntp_retry_timeout)) {
            sntp_retry_timeout = new_retry_timeout;
        }
    }
#endif /* SNTP_RETRY_TIMEOUT_EXP */
}

#if SNTP_SUPPORT_MULTIPLE_SERVERS
/**
 * If Kiss-of-Death is received (or another packet parsing error),
 * try the next server or retry the current server and increase the retry
 * timeout if only one server is available.
 * (implicitly, SNTP_MAX_SERVERS > 1)
 *
 * @param arg is unused (only necessary to conform to sys_timeout)
 */
static void
sntp_try_next_server(void* arg)
{
  u8_t old_server, i;
  LWIP_UNUSED_ARG(arg);

  old_server = sntp_current_server;
  for (i = 0; i < SNTP_MAX_SERVERS - 1; i++) {
    sntp_current_server++;
    if (sntp_current_server >= SNTP_MAX_SERVERS) {
      sntp_current_server = 0;
    }
    if (!ip_addr_isany(&sntp_servers[sntp_current_server].addr)
#if SNTP_SERVER_DNS
        || (sntp_servers[sntp_current_server].name != NULL)
#endif
        ) {
      LWIP_DEBUGF(SNTP_DEBUG_STATE, ("sntp_try_next_server: Sending request to server %"U16_F"\n",
        (u16_t)sntp_current_server));
      /* new server: reset retry timeout */
      SNTP_RESET_RETRY_TIMEOUT();
      /* instantly send a request to the next server */
      sntp_request(NULL);
      return;
    }
  }
  /* no other valid server found */
  sntp_current_server = old_server;
  sntp_retry(NULL);
}
#else /* SNTP_SUPPORT_MULTIPLE_SERVERS */
/* Always retry on error if only one server is supported */
#define sntp_try_next_server    sntp_retry
#endif /* SNTP_SUPPORT_MULTIPLE_SERVERS */


/** UDP recv callback for the sntp pcb */
static void ICACHE_FLASH_ATTR
sntp_recv(void *, struct udp_pcb *, struct pbuf *p, ip_addr_t *addr, u16_t port) {
    u8_t mode;
    u8_t stratum;
    u32_t receive_timestamp[SNTP_RECEIVE_TIME_SIZE];
    err_t err;

    /* packet received: stop retry timeout  */
    sys_untimeout(sntp_try_next_server, NULL);
    sys_untimeout(sntp_request, NULL);

    err = ERR_ARG;
#if SNTP_CHECK_RESPONSE >= 1
    /* check server address and port */
    if (ip_addr_cmp(addr, &sntp_last_server_address) &&
      (port == SNTP_PORT))
#else /* SNTP_CHECK_RESPONSE >= 1 */
    LWIP_UNUSED_ARG(addr);
    LWIP_UNUSED_ARG(port);
#endif /* SNTP_CHECK_RESPONSE >= 1 */
    {
        /* process the response */
        if (p->tot_len == SNTP_MSG_LEN) {
            pbuf_copy_partial(p, &mode, 1, SNTP_OFFSET_LI_VN_MODE);
            mode &= SNTP_MODE_MASK;
            /* if this is a SNTP response... */
            if ((mode == SNTP_MODE_SERVER) ||
                (mode == SNTP_MODE_BROADCAST)) {
                pbuf_copy_partial(p, &stratum, 1, SNTP_OFFSET_STRATUM);
                if (stratum == SNTP_STRATUM_KOD) {
                    /* Kiss-of-death packet. Use another server or increase UPDATE_DELAY. */
                    err = SNTP_ERR_KOD;
                    LWIP_DEBUGF(SNTP_DEBUG_STATE, ("sntp_recv: Received Kiss-of-Death\n"));
                } else {
#if SNTP_CHECK_RESPONSE >= 2
                    /* check originate_timetamp against sntp_last_timestamp_sent */
                    u32_t originate_timestamp[2];
                    pbuf_copy_partial(p, &originate_timestamp, 8, SNTP_OFFSET_ORIGINATE_TIME);
                    if ((originate_timestamp[0] != sntp_last_timestamp_sent[0]) ||
                        (originate_timestamp[1] != sntp_last_timestamp_sent[1]))
                    {
                      LWIP_DEBUGF(SNTP_DEBUG_WARN, ("sntp_recv: Invalid originate timestamp in response\n"));
                    } else
#endif /* SNTP_CHECK_RESPONSE >= 2 */
                    /* @todo: add code for SNTP_CHECK_RESPONSE >= 3 and >= 4 here */
                    {
                        /* correct answer */
                        err = ERR_OK;
                        pbuf_copy_partial(p, &receive_timestamp, SNTP_RECEIVE_TIME_SIZE * 4, SNTP_OFFSET_RECEIVE_TIME);
                    }
                }
            } else {
                //LWIP_DEBUGF(SNTP_DEBUG_WARN, ("sntp_recv: Invalid mode in response: %" U16_F "\n", (u16_t) mode));
            }
        } else {
            //LWIP_DEBUGF(SNTP_DEBUG_WARN, ("sntp_recv: Invalid packet length: %" U16_F "\n", p->tot_len));
        }
    }
    pbuf_free(p);
    if (err == ERR_OK) {
        SNTP_RESET_RETRY_TIMEOUT();

        sntp_process(receive_timestamp);
    } else if (err == SNTP_ERR_KOD) {
        /* Kiss-of-death packet. Use another server or increase UPDATE_DELAY. */
        sntp_try_next_server(NULL);
    } else {
        /* another error, try the same server again */
        sntp_retry(NULL);
    }
}

/** Actually send an sntp request to a server.
 *
 * @param server_addr resolved IP address of the SNTP server
 */
static void ICACHE_FLASH_ATTR
sntp_send_request(ip_addr_t *server_addr) {
    struct pbuf *p;
    os_printf("sntp_send_request\n");
    p = pbuf_alloc(PBUF_TRANSPORT, SNTP_MSG_LEN, PBUF_RAM);
    if (p != NULL) {
        auto sntpmsg = (sntp_msg *) p->payload;
        ets_printf ("sntp_send_request: Sending request to server\n");
        /* initialize request message */
        sntp_initialize_request(sntpmsg);
        /* send request */
        udp_sendto(sntp_pcb, p, server_addr, SNTP_PORT);
        ets_printf ("sntp_send_request: sendto ret\n");
        /* free the pbuf after sending it */
        pbuf_free(p);
        /* set up receive timeout: try next server or retry on timeout */
        sys_timeout((u32_t) SNTP_RECV_TIMEOUT, sntp_try_next_server, NULL);
#if SNTP_CHECK_RESPONSE >= 1
        /* save server address to verify it in sntp_recv */
        ip_addr_set(&sntp_last_server_address, server_addr);
#endif /* SNTP_CHECK_RESPONSE >= 1 */
    } else {
        //LWIP_DEBUGF(SNTP_DEBUG_SERIOUS, ("sntp_send_request: Out of memory, trying again in %" U32_F " ms\n", (u32_t) SNTP_RETRY_TIMEOUT));
        /* out of memory: set up a timer to send a retry */
        sys_timeout((u32_t) SNTP_RETRY_TIMEOUT, sntp_request, NULL);
    }
}

#if SNTP_SERVER_DNS
/**
 * DNS found callback when using DNS names as server address.
 */
static void
sntp_dns_found(const char *hostname, ip_addr_t *ipaddr, void *arg) {
    LWIP_UNUSED_ARG(hostname);
    LWIP_UNUSED_ARG(arg);

    if (ipaddr != NULL) {
        /* Address resolved, send request */
        LWIP_DEBUGF(SNTP_DEBUG_STATE, ("sntp_dns_found: Server address resolved, sending request\n"));
        sntp_send_request(ipaddr);
    } else {
        /* DNS resolving failed -> try another server */
        LWIP_DEBUGF(SNTP_DEBUG_WARN_STATE,
                    ("sntp_dns_found: Failed to resolve server address resolved, trying next server\n"));
        sntp_try_next_server(NULL);
    }
}
#endif /* SNTP_SERVER_DNS */

/**
 * Send out an sntp request.
 *
 * @param arg is unused (only necessary to conform to sys_timeout)
 */
static void ICACHE_FLASH_ATTR
sntp_request(void *) {
    ip_addr_t sntp_server_address;
    err_t err;

    /* initialize SNTP server address */
#if SNTP_SERVER_DNS

    if (sntp_servers[sntp_current_server].name) {
        /* always resolve the name and rely on dns-internal caching & timeout */
        ip_addr_set_any(&sntp_servers[sntp_current_server].addr);
        err = dns_gethostbyname(sntp_servers[sntp_current_server].name, &sntp_server_address,
                                sntp_dns_found, NULL);
        while (err == ERR_INPROGRESS) {
            /* DNS request sent, wait for sntp_dns_found being called */
            LWIP_DEBUGF(SNTP_DEBUG_STATE, ("sntp_request: Waiting for server address to be resolved.\n"));
            tos::this_thread::yield();
        }

        if (err == ERR_OK) {
            sntp_servers[sntp_current_server].addr = sntp_server_address;
        }
    } else
#endif /* SNTP_SERVER_DNS */
    {
        sntp_server_address = sntp_servers[sntp_current_server].addr;
//    os_printf("sntp_server_address ip %d\n",sntp_server_address.addr);
        err = (ip_addr_isany(&sntp_server_address)) ? ERR_ARG : ERR_OK;
    }

    if (err == ERR_OK) {
        //LWIP_DEBUGF(SNTP_DEBUG_TRACE, ("sntp_request: current server address is %u.%u.%u.%u\n", ip4_addr1(&sntp_server_address), ip4_addr2(&sntp_server_address), ip4_addr3( &sntp_server_address), ip4_addr4(&sntp_server_address)));
        sntp_send_request(&sntp_server_address);
    } else {
        /* address conversion failed, try another server */
        //LWIP_DEBUGF(SNTP_DEBUG_WARN_STATE, ("sntp_request: Invalid server address, trying next server.\n"));
        sys_timeout((u32_t) SNTP_RETRY_TIMEOUT, sntp_try_next_server, NULL);
    }
}

void ICACHE_FLASH_ATTR
sntp_init(void) {
#ifdef SNTP_SERVER_ADDRESS
#if SNTP_SERVER_DNS
    sntp_setservername(0, SNTP_SERVER_ADDRESS);
#else
#error SNTP_SERVER_ADDRESS string not supported SNTP_SERVER_DNS==0
#endif
#endif /* SNTP_SERVER_ADDRESS */

    if (sntp_pcb == NULL) {
        SNTP_RESET_RETRY_TIMEOUT();
        sntp_pcb = udp_new();
        LWIP_ASSERT("Failed to allocate udp pcb for sntp client", sntp_pcb != NULL);
        if (sntp_pcb != NULL) {
            udp_recv(sntp_pcb, sntp_recv, NULL);
        }
    }
}

void ICACHE_FLASH_ATTR
sntp_stop(void) {
    if (sntp_pcb != NULL) {
        sys_untimeout(sntp_request, NULL);
        udp_remove(sntp_pcb);
        sntp_pcb = NULL;
    }
    realtime_stamp = 0;
}

void ICACHE_FLASH_ATTR
sntp_setserver(u8_t idx, ip_addr_t *server) {
    if (idx < SNTP_MAX_SERVERS) {
        if (server != NULL) {
            sntp_servers[idx].addr = (*server);
        } else {
            ip_addr_set_any(&sntp_servers[idx].addr);
        }
#if SNTP_SERVER_DNS
        //sntp_servers[idx].name = NULL;
        os_memset(sntp_servers[idx].name, 0x0, sizeof(sntp_servers[idx].name));
#endif
    }
}

ip_addr_t ICACHE_FLASH_ATTR
sntp_getserver(u8_t idx) {
    if (idx < SNTP_MAX_SERVERS) {
        return sntp_servers[idx].addr;
    }
    return *IP_ADDR_ANY;
}

#if SNTP_SERVER_DNS
void ICACHE_FLASH_ATTR
sntp_setservername(u8_t idx, const char *server) {
    if (idx < SNTP_MAX_SERVERS) {
        // sntp_servers[idx].name = server;
        os_strcpy(sntp_servers[idx].name, server);
    }
}

const char *ICACHE_FLASH_ATTR
sntp_getservername(u8_t idx) {
    if (idx < SNTP_MAX_SERVERS) {
        return sntp_servers[idx].name;
    }
    return nullptr;
}
#endif /* SNTP_SERVER_DNS */

void sntp_set_update_delay(int) {
    tos_debug_print("sntp_set_update_delay\n");
}
void sntp_setservername(unsigned char, const char*){
    tos_debug_print("sntp_setservername\n");
}
void sntp_set_daylight(bool){
    tos_debug_print("sntp_set_daylight\n");
}
void sntp_set_timetype(bool){
    tos_debug_print("sntp_set_timetype\n");
}
bool sntp_get_timetype(){
    tos_debug_print("sntp_get_timetype\n");

    return false;
}
void sntp_set_receive_time_size(char){
    tos_debug_print("sntp_set_receive_time_size\n");
}
const char* sntp_get_real_time()
{
    tos_debug_print("sntp_get_real_time\n");

    return nullptr;
}
}