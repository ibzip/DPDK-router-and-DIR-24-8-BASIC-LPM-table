#ifndef ROUTER_H__
#define ROUTER_H__

#include <stdint.h>
#include <unistd.h>
#include <inttypes.h>

#include <rte_config.h>
#include <rte_mbuf.h>
#include <rte_ethdev.h>
#include <rte_arp.h>
#include <rte_ether.h>
#include <rte_ip.h>
#include <rte_byteorder.h>
#include <rte_launch.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include "dpdk_init.h"
#include "routing_table.h"
//---------------------------------------------------
//--------Definitions of needed data tructures-------
//---------------------------------------------------
struct ifc {
    uint32_t ip;
    uint8_t vport;
};

struct thread_config {                                                          
    uint8_t src_ifc_port;                                                       
    uint32_t src_ip;                                                            
};

struct ifc interfaces[3]; // This router got only 3 ifcs
//struct route* routes;


//---------------------------------------------------
//--------Definitions of needed functions------------
//---------------------------------------------------
void init_routes_array();
int router_thread(void* arg);
bool parse_route(char *route);
bool parse_args(int argc, char **argv);
bool parse_ifc(char* ifc_config);
void start_thread(uint8_t port);
bool route_exists(uint32_t ip, uint8_t prefix);
void print_ifc_configs();
bool is_hex_digit(char a);
bool is_valid_mac(char* mac);
void configure_interfaces();
bool parse_packet(struct rte_mbuf* packet, struct thread_config* conf);
bool process_ip_packet(struct rte_mbuf* packet, struct thread_config* conf);
bool process_arp_packet(struct rte_mbuf* packet, struct thread_config* conf);
#endif

