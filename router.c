#include <stdint.h>
#include <unistd.h>
#include <inttypes.h>

#include <rte_config.h>
#include <rte_mbuf.h>
#include <rte_ethdev.h>
#include <rte_arp.h>
#include <rte_icmp.h>
#include <rte_ether.h>
#include <rte_ip.h>
#include <rte_byteorder.h>
#include <rte_launch.h>

#include <arpa/inet.h>
#include "routing_table.h"
#include "dpdk_init.h"
#include "router.h"
//#include <netinet/ether.h>
static int num_ifcs = 3;
static unsigned lcore = 1;
//############################################
//####### Packet parsing functions ###########
//############################################

bool process_arp_packet(struct rte_mbuf* packet, struct thread_config* conf) {
    struct ether_hdr* eth = rte_pktmbuf_mtod(packet, struct ether_hdr*);
    struct arp_hdr* arp = rte_pktmbuf_mtod_offset(packet, struct arp_hdr*, sizeof(struct ether_hdr));
    if (arp->arp_op == rte_cpu_to_be_16(ARP_OP_REQUEST)) {
        printf("Received ARP requiest for interface %d\n",conf->src_ifc_port);
	if (arp->arp_data.arp_tip != conf->src_ip) {
	    // No Proxy ARP implementation
	    printf("ARP request not the IP of interface:%d, dropping packet\n", conf->src_ifc_port);
	    return 1; // Request not for me, drop packet
	}

	// Formulate response now
        // 1. Modify fields of ARP header
	arp->arp_op = rte_cpu_to_be_16(ARP_OP_REPLY); // Indicate its a response
        ether_addr_copy(&arp->arp_data.arp_sha,&arp->arp_data.arp_tha); // original sender is now receiver
	// Copy this interface's mac to sha field
	struct ether_addr my_mac;
	rte_eth_macaddr_get(conf->src_ifc_port, &my_mac);
	//ether_addr_copy(&my_mac, &arp->arp_data.arp_sha);
        arp->arp_data.arp_sha = my_mac;
	// Sender ip is now the receiver ip
	arp->arp_data.arp_tip = arp->arp_data.arp_sip;
        // sender is this interface
	arp->arp_data.arp_sip = conf->src_ip; //src_ip was already stored in the network byte order 

        // 2. Modify fields of ETHERNET header
	// Correctly set SRC and DST macs in the ethernet header now
	// Previous SRC mac is now the DST mac in response
	ether_addr_copy(&eth->s_addr, &eth->d_addr);
	// And SRC mac is this interface's mac
	//ether_addr_copy(&my_mac, &eth->s_addr);
        eth->s_addr = my_mac;
    } else {
	printf("Not an ARP request packet, droping it\n");
	return 1; // Drop the response packet
    }
    // Transmit ARP reponse out of the same port
    printf("Transmitting ARP response from interface:%d\n", conf->src_ifc_port);
    // Retry if packet not transmitted due to TX queue full etc
    while(!rte_eth_tx_burst(conf->src_ifc_port, conf->src_ifc_port, &packet, 1));
    return 0;
}

bool is_valid_ip_packet(struct ipv4_hdr* hdr, uint32_t len) {
    if (len < 20)
	return 0;
    if ((hdr->version_ihl >> 4) !=4)
	return 0;
    if((hdr->version_ihl & 0x0f) < 5)
	return 0;
    if (rte_be_to_cpu_16(hdr->total_length) < 20)
	return 0;
    return 1;
}

bool process_ip_packet(struct rte_mbuf* packet, struct thread_config* conf) {
    struct ether_hdr* eth = rte_pktmbuf_mtod(packet, struct ether_hdr*);
    struct ipv4_hdr* ip = rte_pktmbuf_mtod_offset(packet, struct ipv4_hdr*, sizeof(struct ether_hdr));
    struct in_addr log_ip;

    if (!is_valid_ip_packet(ip, packet->pkt_len)) {
	printf("<process_ip_packet> Interface:%d, invalid ip packet. Dropping", conf->src_ifc_port);
        return 1;
    }

    // Get source ip in char* format
    log_ip.s_addr = ip->src_addr;
    char* temp = inet_ntoa(log_ip);
    char *src_ip = malloc(strlen(temp));
    memcpy(src_ip, temp, strlen(temp));
    
    // Get dst ip in char* format
    log_ip.s_addr = ip->dst_addr;
    temp = inet_ntoa(log_ip);
    char *dst_ip = malloc(strlen(temp));
    memcpy(dst_ip, temp, strlen(temp));

    // Get this ifc's ip in char* format
    log_ip.s_addr = conf->src_ip;
    temp = inet_ntoa(log_ip);
    char *my_ip = malloc(strlen(temp));
    memcpy(my_ip, temp, strlen(temp));
    uint8_t dst_port;
    printf("<process_ip_packet>-Interface:%d, IP packet received from:%s, intended for:%s, my_ip:%s\n", conf->src_ifc_port, src_ip, dst_ip, my_ip);
    struct ether_addr src_mac;
    struct ether_addr dst_mac;

    // Check if this is a packet intended for one of the router's ifcs. Respond if it's an icmp echo.
    bool me = false;
    for (int i=0;i<3;i++) {                                                     
        if (interfaces[i].ip && interfaces[i].ip == ip->dst_addr) {
	    // Check if an icmp packet
	    if (ip->next_proto_id == 1) {
		struct icmp_hdr *icmp = rte_pktmbuf_mtod_offset(packet, struct icmp_hdr *, (sizeof(struct ether_hdr) + sizeof(struct ipv4_hdr)));
		// Check if its an icmp echo request
	        if (icmp->icmp_type == IP_ICMP_ECHO_REQUEST) {
	            icmp->icmp_type = IP_ICMP_ECHO_REPLY;
		    printf("Interface:%d, ICMP echo Packet from src_ip:%s intended for one of router ifcs, my ip:%s, responding.\n",conf->src_ifc_port, src_ip, my_ip);
		} else {
		  return 1; // drop packet
		}
	    } else {
	      return 1; //drop packet
	    }

	    log_ip.s_addr = ip->src_addr;
	    temp = inet_ntoa(log_ip);
	    memset(dst_ip, 0, sizeof(dst_ip));
	    memcpy(dst_ip, temp, strlen(temp));
	    ip->dst_addr = ip->src_addr;
	    ip->src_addr = interfaces[i].ip;
            rte_eth_macaddr_get(conf->src_ifc_port, &src_mac);
	    dst_mac = eth->s_addr;
	    dst_port = conf->src_ifc_port;
	    me = true;
	    break;
	}                                                            
    } 

    // Logic to forward the packet from some other interface
    if (!me) { // Packet not intended for router
      if (ip->time_to_live == 1) {
          printf("TTL expired. Dropping packet\n");
	  return 1;
      }

      struct in_addr temp;
      temp.s_addr = ip->dst_addr;
      char *host_order_ip = inet_ntoa(temp);
      //printf("This is hostorder ip: %s\n\n",host_order_ip);
      uint32_t host_order_ip_bin = inet_network(host_order_ip); 
      // Do a route lookup
      struct routing_table_entry* route = get_next_hop(host_order_ip_bin);
      if (!route) {
	printf("Interface:%d, No route found for dst ip:%s, dropping packet.\n", conf->src_ifc_port, dst_ip);
	// No route for this IP. Drop.
	return 1;
      }

      dst_mac = route->dst_mac;
      dst_port = route->dst_port;
      if (dst_port > 2) {// No such interface exists
	printf("Interface:%d, wrong output port:%d, in the route lookup for DST ip:%s, dropping packet\n", conf->src_ifc_port, dst_port, dst_ip); 
	return 1;
      }
      // Get the MAC associated with the port from which packet will be sent out. 
      // Used it as source mac                                                    
      rte_eth_macaddr_get(dst_port, &src_mac);
    }

    // Modify the SRC and DST macs in ethernet header
    ether_addr_copy(&src_mac, &eth->s_addr);
    ether_addr_copy(&dst_mac, &eth->d_addr); 
    
    printf("Inteface:%d, putting packet into the TX queue of interface:%d, with DST_IP:%s\n", conf->src_ifc_port, dst_port, dst_ip);
    // Transmit the packet finally
    while(!rte_eth_tx_burst(dst_port, conf->src_ifc_port, &packet, 1));
    return 0;

}

bool parse_packet(struct rte_mbuf* packet, struct thread_config* conf) {
    struct ether_hdr* eth = rte_pktmbuf_mtod(packet, struct ether_hdr*);
    uint16_t ether_type = rte_be_to_cpu_16(eth->ether_type);
    if (ether_type == ETHER_TYPE_ARP) 
	return process_arp_packet(packet, conf);
    else if (ether_type == ETHER_TYPE_IPv4)
	return process_ip_packet(packet, conf);
    else
	return 1; // drop all other packets
}
//##################################################

//############################################
//###interface config related functions ######                                    
//############################################ 

int router_thread(void* arg) {	
    struct thread_config* conf = (struct thread_config*)arg;
    // Write the code to read from all rx queues of "port" ifc and then parse packet.
    struct rte_mbuf* packets[50];// = malloc(50*sizeof(struct rte_mbuf)); // Read 50 packets at a time for
                                                                   // this interface's RX queue
    while(1) { // Main thread loop
        uint8_t rx = recv_from_device(conf->src_ifc_port, num_ifcs, packets, 50);
        if (!rx) {
            usleep(100); // No packets read, make the tread sleep for 100 microseconds    
	    continue;
	}
	//printf("received packets");
	for (uint8_t i=0;i<rx;i++) {
            bool drop = parse_packet(packets[i], conf);
	    if (drop) {
		rte_pktmbuf_free(packets[i]);
	    }
	}
    }


    return 1;
}

void configure_interfaces() {
    for (int i=0;i<3;i++) {
	struct in_addr log_ip;
	log_ip.s_addr = interfaces[i].ip;
	if (interfaces[i].ip != 0) {// configured device
	    //uint8_t dummyport = 0;
	    //uint16_t num_queues = 3;
	    configure_device(interfaces[i].vport, 3);
            printf("Configuring interface:%d\n", i);
	    struct thread_config* conf = (struct thread_config*) malloc(sizeof(struct thread_config));
	    printf("interface port:%d\n", interfaces[i].vport);
	    conf->src_ifc_port = interfaces[i].vport;
	
	    conf->src_ip = interfaces[i].ip;
	    rte_eal_remote_launch(router_thread, conf, lcore++);
	}  
    }
}
//#####################################################

//########################################################
//############## Utility Functions #######################
//3#######################################################
void print_ifc_configs() {
    printf("Following interfaces have been configured:\n");
    for (int i=0;i<3;i++) {
	if (interfaces[i].ip != 0) {
	    struct in_addr ip_addr;
	    ip_addr.s_addr = interfaces[i].ip; 
	    printf("IFC:%d, vport:%d, ip:%s\n", interfaces[i].vport, interfaces[i].vport, inet_ntoa(ip_addr));
	}
    }
}

bool route_exists(uint32_t ip, uint8_t prefix) {
    for (uint32_t i=0;i<raw_routes_idx;i++) {
	if (raw_routes[i].ip == ip && raw_routes[i].prefix == prefix) {
	    return 1;
        }
    }
    return 0;
}

bool is_hex_digit(char a) {

    if ((int)a < 48)
	return 0;
    if ((int)a > 57)
	if ((int)a <97 || (int)a > 102)
	    return 0;
    return 1;
}

bool is_valid_mac(char* mac) {
    if (strlen(mac) != 17) {// Invalid format of MAC. Its a very crude check     
        return 0;                                                               
    }
    
    if (!is_hex_digit(mac[0]) || !is_hex_digit(mac[1]) || !is_hex_digit(mac[3]) || !is_hex_digit(mac[4])
        || !is_hex_digit(mac[6]) || !is_hex_digit(mac[7]) || !is_hex_digit(mac[9]) || !is_hex_digit(mac[10])
        || !is_hex_digit(mac[12]) || !is_hex_digit(mac[13]) || !is_hex_digit(mac[15]) || !is_hex_digit(mac[16])) {
	return 0;
    }
    return 1;

}

//##################################################
//########## Parameter parsing functions ###########
//##################################################
bool parse_ifc(char* ifc_config) {                                              
    // Parse port                                                               
    char* token = strtok(ifc_config, ",");                                      
    if (!token)                                                                 
        return 0;                                                               
                                                                                
    int vport = atoi(token);                                                    
    if (vport > 2 || vport < 0)// User can still pas some string and atoi will return 0. Anyways...
        return 0;                                                               
    // Parse IP                                                                 
    token = strtok(NULL, ",");                                                  
    if (!token)                                                                 
        return 0;                                                               
    struct in_addr *inp = (struct in_addr *)malloc(sizeof(struct in_addr));     
    if (inet_aton(token, inp) == 0) {// Wasn't able to parse any address        
        free(inp);                                                              
        return 0;                                                               
    }                                                                           
                                                                                
    // If the interface has already been configured, return error               
    if (interfaces[vport].ip) {                                
        printf("Interface with port %d has already been configured. Dual configuration not allowed", vport);
        free(inp);                                                              
        return 0;                                                               
    }                                                                           
                                                                                
    // Else configure the interface                                             
    interfaces[vport].ip = inp->s_addr;                                   
    interfaces[vport].vport = vport;                                      
    free(inp);                                                                 
    return 1;                                                                   
} 

static struct ether_addr* my_ether_aton(const char *a) {
  int i;
  char* end;
  unsigned long o[ETHER_ADDR_LEN];
  static struct ether_addr ether_addr;
  i = 0;
  do {
	  errno = 0;
	  o[i] = strtoul(a, &end, 16);
	  if (errno != 0 || end == a || (end[0] != ':' && end[0] != 0))
		  return NULL;
	  a = end + 1;
  } while (++i != sizeof(o) / sizeof(o[0]) && end[0] != 0);
  /* Junk at the end of line */
  if (end[0] != 0)
	  return NULL;
  /* Support the format XX:XX:XX:XX:XX:XX */
  if (i == ETHER_ADDR_LEN) {
	  while (i-- != 0) {
		  if (o[i] > UINT8_MAX)
			  return NULL;
		  ether_addr.addr_bytes[i] = (uint8_t)o[i];
	  }
  /* Support the format XXXX:XXXX:XXXX */
  } else if (i == ETHER_ADDR_LEN / 2) {
	  while (i-- != 0) {
		  if (o[i] > UINT16_MAX)
			  return NULL;
		  ether_addr.addr_bytes[i * 2] = (uint8_t)(o[i] >> 8);
		  ether_addr.addr_bytes[i * 2 + 1] = (uint8_t)(o[i] & 0xff);
	  }
	  /* unknown format */
  } else 
	  return NULL;
  return (struct ether_addr *)&ether_addr;

}

bool parse_route(char *route) {                                                 
    char* ip_prefix;                                                            
    // Get ip_prefix from the route                                             
    ip_prefix = strtok(route, ",");                                             
    if (!ip_prefix)                                                                   
        return 0;                                                               
                                                                                
    // Get mac from the route                                                   
    char* mac = strtok(NULL,",");                                               
    if (!mac)
	return -1;
                                                                                
    // Get out_port for this dest ip and mac                                    
    char* tok = strtok(NULL, ",");                                              
    if (!tok)// No port                                                         
        return 0;                                                               
    // Check there is nomore stuff in the route
    if (strtok(NULL, ",")) {                                                    
        printf("Is this a joke? You have way too much free time.\               
                Are you trying to give the route to another galaxy? \           
	        Come on brooo. It's just a VM.");                               
    }                                                                                  
    // Check that port is valid                                                 
    int port = atoi(tok);                                                       
    if (port < 0 || port > 2) // User can still get away with providing gibberish and atoi will return 0. Anyways...
        return 0;                                   


    // Check if mac is valid                                                    
    if (!is_valid_mac(mac))                                                     
        return 0;
    // check if ip is valid
    char *ip_prefix_copy = malloc(strlen(ip_prefix));
    memcpy(ip_prefix_copy, ip_prefix, strlen(ip_prefix));
    tok = strtok(ip_prefix, "/");      
    if (!tok)                                                                   
        return -1;                                                              
    uint32_t inp = inet_network(tok);     
    //if (inet_aton(tok, inp) == 0) {// Wasn't able to parse any address          
    //    free(inp);                                                              
    //    return 0;                                                               
    //}                                                                           
    tok = strtok(NULL, "/");
    uint8_t prefix = (uint8_t)atoi(tok);
    if (prefix <= 0 || prefix > 32)
        return 0;
    if (route_exists(inp, prefix)) {                                              
	printf("A route has already been configured for ip/prefix %s", ip_prefix);
	return 0;                                                               
    }
    printf("\n\n mac is %s %d\n\n", mac, sizeof(mac));
    struct ether_addr* mac_addr;// = malloc(sizeof(struct ether_addr));
    
    mac_addr = my_ether_aton(mac);
printf("> Successfully received Local MAC Address : %02x:%02x:%02x:%02x:%02x:%02x\n",
		  (unsigned char)mac_addr->addr_bytes[0],
		    (unsigned char) mac_addr->addr_bytes[1],
		      (unsigned char) mac_addr->addr_bytes[2],
		        (unsigned char) mac_addr->addr_bytes[3],
			  (unsigned char) mac_addr->addr_bytes[4],
			    (unsigned char) mac_addr->addr_bytes[5]);
    add_route(inp, prefix,mac_addr,port);
    //free(inp);
    return 1;                                                                   
} 

bool parse_args(int argc, char **argv) {                                        
    int opt;                                                                    
                                                                                
    while ((opt = getopt(argc, argv, "p:r:")) != EOF) {                         
        if (opt == 'p') {                                                       
            // Interface configuration                                          
            if (parse_ifc(optarg) == 0 )                                        
                return 0;                                                       
        } else if (opt == 'r') {                                                
            // Route configuration                                              
            if (parse_route(optarg) == 0)                                       
                return 0;                                                       
        }                                                                       
    }                                                                           
    return 1;                                                                   
} 

