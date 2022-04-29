set(LWIP_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src/src/include)
#add_subdirectory(src)

add_library(lwipcore "")
target_sources(lwipcore PRIVATE
    src/src/core/init.c
    src/src/core/def.c
    src/src/core/dns.c
    src/src/core/inet_chksum.c
    src/src/core/ip.c
    src/src/core/mem.c
    src/src/core/memp.c
    src/src/core/netif.c
    src/src/core/pbuf.c
    src/src/core/raw.c
    src/src/core/stats.c
    src/src/core/sys.c
    src/src/core/altcp.c
    src/src/core/altcp_alloc.c
    src/src/core/altcp_tcp.c
    src/src/core/tcp.c
    src/src/core/tcp_in.c
    src/src/core/tcp_out.c
    src/src/core/timeouts.c
    src/src/core/udp.c
    src/src/core/ipv4/autoip.c
    src/src/core/ipv4/dhcp.c
    src/src/core/ipv4/etharp.c
    src/src/core/ipv4/icmp.c
    src/src/core/ipv4/igmp.c
    src/src/core/ipv4/ip4_frag.c
    src/src/core/ipv4/ip4.c
    src/src/core/ipv4/ip4_addr.c
    src/src/core/ipv6/dhcp6.c
    src/src/core/ipv6/ethip6.c
    src/src/core/ipv6/icmp6.c
    src/src/core/ipv6/inet6.c
    src/src/core/ipv6/ip6.c
    src/src/core/ipv6/ip6_addr.c
    src/src/core/ipv6/ip6_frag.c
    src/src/core/ipv6/mld6.c
    src/src/core/ipv6/nd6.c
    src/src/api/api_lib.c
    src/src/api/api_msg.c
    src/src/api/err.c
    src/src/api/if_api.c
    src/src/api/netbuf.c
    src/src/api/netdb.c
    src/src/api/netifapi.c
    src/src/api/sockets.c
    src/src/api/tcpip.c
    src/src/netif/ethernet.c
    src/src/netif/bridgeif.c
    src/src/netif/bridgeif_fdb.c
    src/src/netif/slipif.c
    src/src/netif/lowpan6_common.c
    src/src/netif/lowpan6.c
    src/src/netif/lowpan6_ble.c
    src/src/netif/zepif.c
    src/src/netif/ppp/auth.c
    src/src/netif/ppp/ccp.c
    src/src/netif/ppp/chap-md5.c
    src/src/netif/ppp/chap_ms.c
    src/src/netif/ppp/chap-new.c
    src/src/netif/ppp/demand.c
    src/src/netif/ppp/eap.c
    src/src/netif/ppp/ecp.c
    src/src/netif/ppp/eui64.c
    src/src/netif/ppp/fsm.c
    src/src/netif/ppp/ipcp.c
    src/src/netif/ppp/ipv6cp.c
    src/src/netif/ppp/lcp.c
    src/src/netif/ppp/magic.c
    src/src/netif/ppp/mppe.c
    src/src/netif/ppp/multilink.c
    src/src/netif/ppp/ppp.c
    src/src/netif/ppp/pppapi.c
    src/src/netif/ppp/pppcrypt.c
    src/src/netif/ppp/pppoe.c
    src/src/netif/ppp/pppol2tp.c
    src/src/netif/ppp/pppos.c
    src/src/netif/ppp/upap.c
    src/src/netif/ppp/utils.c
    src/src/netif/ppp/vj.c
    src/src/netif/ppp/polarssl/arc4.c
    src/src/netif/ppp/polarssl/des.c
    src/src/netif/ppp/polarssl/md4.c
    src/src/netif/ppp/polarssl/md5.c
    src/src/netif/ppp/polarssl/sha1.c
)
target_include_directories(lwipcore PUBLIC src/src/include)

add_library(lwip_support "" include/tos/lwip/udp.hpp include/tos/lwip/utility.hpp include/tos/lwip/common.hpp include/tos/lwip/tcp.hpp include/tos/lwip/if_adapter.hpp)
target_sources(lwip_support PRIVATE
    support.cpp
    include/lwipopts.h
    include/arch/cc.h)
target_include_directories(lwip_support PUBLIC include)
target_include_directories(lwip_support PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/src/include)
target_link_libraries(lwip_support PUBLIC tos::drivers tos_printf)

if (${TOS_PLATFORM} MATCHES raspi)
    target_compile_definitions(lwip_support PUBLIC __ssize_t_defined)
endif()

target_link_libraries(lwipcore PUBLIC lwip_support)

if (TARGET hosted_tap_network)
    add_executable(lwip_builds build.cpp)
    target_link_libraries(lwip_builds PUBLIC tos_core arch_drivers lwipcore tos_dynamic_log)
endif()