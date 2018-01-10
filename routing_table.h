#ifndef ROUTING_TABLE_H__
#define ROUTING_TABLE_H__

#include <stdbool.h>

#include <rte_config.h>
#include <rte_ether.h>
//#include <netinet/ether.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <rte_ip.h>                                                             
#include <rte_byteorder.h>

// build a new routing table
void add_route(uint32_t ip_addr, uint8_t prefix, struct ether_addr* mac_addr, uint8_t port);
void print_routes();
void print_port_id_to_mac();
void build_routing_table();
void print_next_hop_tab();
void resize_routes_array();

struct routing_table_entry {                                                    
    struct ether_addr dst_mac;                                                    
    uint8_t dst_port;                                                             
};

extern uint8_t glob_next_hop_id;
struct routing_table_entry  next_hop_array[254]; // Assuming router only needs to address 254 unique next hops

struct raw_route {
  uint32_t ip;
  uint8_t prefix;
  uint8_t next_hop;
};

struct tbl24_entry {
    bool is_long_prefix;
    uint8_t next_hop;
    uint8_t prefix; // Ideally this should be a 5-bit data type
};

struct tbllong_entry {
  uint8_t next_hop;
  uint8_t prefix; //Ideally this should be a 5-bit data type
};


//#########################################
//###### Important Tables #################
//#########################################
// LPM tables for 24 bit and longer prefixes
extern uint8_t tblong_idx;
struct tbl24_entry** tbl24;
struct tbllong_entry** tbllong;

// route table whose entries will be pointed by
// next_hop in LPM tables
extern uint32_t routes_array_size;
extern uint32_t raw_routes_idx;
struct raw_route* raw_routes;

//#########################################

void print_routing_table_entry(struct routing_table_entry* info);

struct routing_table_entry* get_next_hop(uint32_t ip);

#endif

