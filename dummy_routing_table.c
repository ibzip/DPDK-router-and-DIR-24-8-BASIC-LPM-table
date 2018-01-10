//#include "routing_table.h"

#include <rte_config.h>
#include <rte_ip.h>
#include <stdbool.h>                                                            
                                                                                
#include <rte_config.h>                                                         
#include <rte_ether.h>                                                          
#include <stdio.h>                                                              
#include <stdlib.h> 
// do nothing :)
//void build_routing_table() {
//}
/*
static struct routing_table_entry hop_info1 = { // for svm521 with ip 10.0.0.2
    .dst_mac = {.addr_bytes = {0x52, 0x54, 0x00, 0x63, 0x56, 0x20}},
    .dst_port = 0
};
static struct routing_table_entry hop_info2 = { // for svm522 with ip 192.168.0.2
    .dst_mac = {.addr_bytes = {0x52, 0x54, 0x00, 0x45, 0x64, 0xea}},
    .dst_port = 1
};

static struct routing_table_entry hop_info3 = { // for svm523 with ip 172.16.0.2                                
    .dst_mac = {.addr_bytes = {0x52, 0x54, 0x00, 0x92, 0x07, 0x6f}},            
    .dst_port = 2                                                               
};

struct routing_table_entry* get_next_hop(uint32_t ip) {
	if (ip == rte_cpu_to_be_32(IPv4(10,0,0,2))) {
		return &hop_info1;
	} else if (ip == rte_cpu_to_be_32(IPv4(192,168,0,2))) {
		return &hop_info2;
	} else if (ip == rte_cpu_to_be_32(IPv4(172,16,0,2))){
	        return &hop_info3;
	} else {
		return NULL;
	}
}
*/
//void add_route(uint32_t ip_addr, uint8_t prefix, struct ether_addr* mac_addr, uint8_t port) {
//	return; // do nothing
//}

