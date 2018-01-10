#include "router.h"

/**
 * Main function of the router.
 */
void usage() {
    printf("\n Correct Usage:\n \
    ############################################################\n\
    Router Interface Arg: -p source interface [0-2],IP \n\
    Next Hop Arg: -r IP/mask,destination mac,destination interface [0-2]\n\
    -p and -r can be used multiple time\n\
    Example:\n\
    ./router -p 0,10.0.10.1 -r 10.0.10.2/32,52:54:00:cb:ee:f4,0\n\
    ############################################################\n");   
}

int main(int argc, char* argv[]) {
    printf("%d", argc);
    if (argc == 1) {
	usage();
	return 0;
    }
    init_dpdk();
    //init_routes_array();// Initialize the memory for routes array. Thank to C. 
    int ok = parse_args(argc, argv);
    if (!ok) {
	// Print usage
	usage();
    } else {
	// Print configs
	print_ifc_configs();
	printf("printing routes \n");
	print_routes();
        printf("configuring interfaces\n");
	// Config interfaces
	configure_interfaces();
	build_routing_table();
	// Wait for threads 
	rte_eal_mp_wait_lcore();
    }
    return 0;
}

