#include "routing_table.h"
#include <arpa/inet.h>
uint8_t glob_next_hop_id = 0;                                                         
uint8_t tblong_idx = 0;                                                               
uint32_t routes_array_size = 1000;                                                     
uint32_t raw_routes_idx = 0; 

void resize_routes_array() {                     
    routes_array_size += 500; // Increase routes array size by 500                 
    raw_routes = realloc(raw_routes, routes_array_size * sizeof(struct raw_route));
} 

void print_routes() {
  for (uint32_t i=0;i<raw_routes_idx;i++) {
    uint8_t first = (raw_routes[i].ip >> 24) & ((1 << 8) - 1);
    uint8_t second = (raw_routes[i].ip >> 16) & ((1 << 8) - 1);
    uint8_t third = (raw_routes[i].ip >> 8) & ((1 << 8) - 1);
    uint8_t fourth = raw_routes[i].ip & ((1 << 8) - 1);
    printf("Route:%d, ip:%d.%d.%d.%d, prefix:%d, dst_port:%d\n", i, first,second,third,fourth, raw_routes[i].prefix, next_hop_array[raw_routes[i].next_hop].dst_port);
  }

}

struct routing_table_entry* get_next_hop(uint32_t ip) {
    uint32_t mask = (1<<24) - 1;
    uint32_t orig_index = ((ip>>8) & mask); // Index given by first 24 bits
    //printf("orig_index in lookup is:%d\n\n", orig_index);
    if (tbl24[orig_index]) {
	uint8_t next_hop = tbl24[orig_index]->next_hop;
	if (!tbl24[orig_index]->is_long_prefix) {
	  return &next_hop_array[next_hop]; 
	} else {
	  //printf("yoyoyo:%d, %d\n\n",next_hop*256 + (ip & ((1<<8) -1)), tbllong[next_hop + (ip & ((1<<8) -1))]->next_hop); 
	  //printf("looking up long prefix: port is :%d, damn:%d\n\n", next_hop_array[tbllong[next_hop*256 + (ip & ((1<<8) -1))]->next_hop].dst_port, next_hop);
	  return &next_hop_array[tbllong[next_hop*256 + (ip & ((1<<8) -1))]->next_hop];
	}
    }
    return NULL;
}


void build_routing_table() {
  
  for (uint32_t i=0;i<raw_routes_idx;i++) {
	  printf("===== %d\n\n", raw_routes[i].next_hop);
    if (raw_routes[i].prefix <= 24) {
      uint32_t orig_index = raw_routes[i].ip >> 8;
      orig_index &= (((1<<raw_routes[i].prefix) -1) << (24-raw_routes[i].prefix));
      // If prefix less than equal to 24, find position, write until the end until an alrady written entry is found
      //uint32_t mask = (1<<raw_routes[i].prefix) - 1;
      //uint32_t orig_index = ((raw_routes[i].ip>>(32-raw_routes[i].prefix)) & mask); // Index given by first 24 bits
      //printf("prefix is:%d, orig_index: %d\n", raw_routes[i].prefix, orig_index);
      for (uint32_t tbl24_in=orig_index;tbl24_in<orig_index+(pow(2,24-raw_routes[i].prefix));tbl24_in++) {
        if (tbl24[tbl24_in]) {

          if (tbl24[tbl24_in]->is_long_prefix) { // if entries for this location exist in long prefix table
            // Check if a lesser specific prefixes need to be replaced with a more specific one
            uint16_t tbl_idx = tbl24[tbl24_in]->next_hop*256;
            for (uint16_t p = tbl_idx;p < tbl_idx + 256;p++) {
              if (tbllong[p]->prefix > raw_routes[i].prefix)
                continue;
              // if lesser prefix found, overwrite
              tbllong[p]->prefix = raw_routes[i].prefix;
              tbllong[p]->next_hop = raw_routes[i].next_hop;
            }
            continue;
          } else {
            if (tbl24[tbl24_in]->prefix > raw_routes[i].prefix)
              continue;
          }

        } else {
          tbl24[tbl24_in] = (struct tbl24_entry*)malloc(sizeof(struct tbl24_entry));
        }
        //printf("writing for %d\n\n",tbl24_in);
        tbl24[tbl24_in]->is_long_prefix = false;
        tbl24[tbl24_in]->next_hop = raw_routes[i].next_hop;
        tbl24[tbl24_in]->prefix = raw_routes[i].prefix;
      }

    } else { // Route prefix length > 26
      // If greater than 24, overwrite the entry in table 24, and write entries in table 26
      uint32_t mask = (1<<24)-1;
      uint32_t orig_index = ((raw_routes[i].ip>>8) & mask); // Index given by first 24 bits
      //printf("prefix is:%d, orig_index: %d\n", raw_routes[i].prefix, orig_index);
      /*
      if (tbl24[orig_index] && tbl24[orig_index]->is_long_prefix) {
        // 1.->Only need to update entries in the tllong table
      } else if (tbl24[orig_index] && !tbl24[orig_index]->is_long_prefix) {
        //1.Need to create entries in tbllong
        //2.->Update entries in tbllong
      } else if (!tbl24[orig_index]) {
        //1.Gotta create entry in tbl24.
        //2.Gotta create entries in tbllong
        //3.->update entries in tbllong
      }
      //. Update entries is common in all three cases, can be done here.
      */
      bool just_created = false;
      if (!tbl24[orig_index]) {
        // Entry needs to be created in tbl24
        tbl24[orig_index] = (struct tbl24_entry*)malloc(sizeof(struct tbl24_entry));
        tbl24[orig_index]->is_long_prefix = 0;
        tbl24[orig_index]->prefix = 0;
        just_created = true;
      }

      if (!tbl24[orig_index]->is_long_prefix) {
        //printf("<<<<<<not tbllong prefix>>>\n\n");
	uint16_t tbllong_start_idx = tblong_idx*256;
        //Entries need to be created in the tbllong for the first time
        for (uint16_t p = tbllong_start_idx; p < tbllong_start_idx+256; p++) {
          tbllong[p] = (struct tbllong_entry*)malloc(sizeof(struct tbllong_entry));
          tbllong[p]->prefix = 0;
        }
        if (!just_created) {
	  //printf("insideee is itttt>>>>> \n\n");
          // Need to copy the next_route of this tbl24 entry to tblloong entries corresponding to this entry
          for (uint16_t p = tbllong_start_idx; p < tbllong_start_idx+256; p++) {
            tbllong[p]->prefix = tbl24[orig_index]->prefix; // prefix of this tbl24 entry
            tbllong[p]->next_hop = tbl24[orig_index]->next_hop; // Next hop of this tbl24 entry
          }
        }
        // Since we create the tbllong entries for this tbl24 entry, this tbl24 entry needs to point to start of those tbllong entries
        tbl24[orig_index]->next_hop = tblong_idx;
        tbl24[orig_index]->prefix = 24; // This won't be used where is_long_prefix
        tbl24[orig_index]->is_long_prefix = 1;
	tblong_idx++;
	//printf("tblong_idx:%d, n_hop:%d\n\n", tblong_idx, tbl24[orig_index]->next_hop);
      }

      //. Update entries is common in all three cases, can be done here.
      uint16_t tbllong_start_idx = tbl24[orig_index]->next_hop*256 + (raw_routes[i].ip & ( ( ( 1<<(raw_routes[i].prefix-24) ) - 1) <<(32-raw_routes[i].prefix)));
      for (uint16_t p = tbllong_start_idx; p < tbllong_start_idx + (pow(2,32-raw_routes[i].prefix)); p++) {
        if (raw_routes[i].prefix > tbllong[p]->prefix) {
          tbllong[p]->prefix = raw_routes[i].prefix;
          tbllong[p]->next_hop = raw_routes[i].next_hop;
          //printf("aftaaar,%d, %d, %d\n\n", p, raw_routes[i].prefix, raw_routes[i].next_hop);
	}
      }
    }
  }
}

void add_route(uint32_t ip_addr, uint8_t prefix, struct ether_addr* mac_addr, uint8_t port) {
  //TODO: Validate the fields. Not sure if there is a need
  if (!tbl24) {
      tbl24 = (struct tbl24_entry**) malloc(sizeof(struct tl24_entry*)*pow(2,24)); //table for prefixes <=24 of 2^24 size
  }
  if (!tbllong) {
      tbllong = (struct tbllong_entry**) malloc(255*256*sizeof(struct tbllong_entry*)); // Assuming there can be at max of 255 subnets that require >24 prefixes
  }
  if (!raw_routes) {
      raw_routes = (struct raw_route*)malloc(routes_array_size * sizeof(struct raw_route)); // Initial routes that acn be populated are 1000
  }

  if (raw_routes_idx == routes_array_size) {
    resize_routes_array();
  }
  uint8_t next_hop_id = glob_next_hop_id;/*
  for (int i=0;i<glob_next_hop_id;i++) {
    if (strcmp(ether_ntoa(next_hop_array[i].dst_mac), ether_ntoa(*mac_addr)) == 0 && next_hop_array[i].dst_port == port) {
      next_hop_id = i;
      break;
    }
  }*/
  if (next_hop_id == glob_next_hop_id && glob_next_hop_id > 253) {
    printf("Can not add anymore unique next hops\n");
    return;
  }
  raw_routes[raw_routes_idx].ip = ip_addr;
  raw_routes[raw_routes_idx].prefix = prefix;
  raw_routes[raw_routes_idx].next_hop = next_hop_id;
  //printf("nhopid:%d, port:%d\n\n", next_hop_id, port); 
  next_hop_array[next_hop_id].dst_port = port;
  ether_addr_copy(mac_addr, &next_hop_array[next_hop_id].dst_mac);

  raw_routes_idx++;

  if (next_hop_id == glob_next_hop_id)
    glob_next_hop_id++; 
}


