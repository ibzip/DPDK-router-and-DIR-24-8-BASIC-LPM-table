
bool parse_args(int argc, char* argv[]) {
    if (argc%2 != 0)
        return 0;
    int opt;
    while ((opt = getopt(argc, argv, "p:r:")) != EOF) {
        printf("%s->%s\n", opt, optarg)
    }
    
 }

