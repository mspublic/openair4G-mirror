/***************************************************************************
                          mRALteInt.c (main)  -  description
                             -------------------
    copyright            : (C) 2012 by Eurecom
    email                : davide.brizzolara@eurecom.fr, michelle.wetterwald@eurecom.fr
 ***************************************************************************
 mRALlte main
 ***************************************************************************/
#define MRAL_MODULE
#define MRALLTEINT_MAIN_C
//-----------------------------------------------------------------------------
#include "mRALlteInt_main.h"
#include "mRALlteInt_constants.h"
//-----------------------------------------------------------------------------
#include "MIH_C.h"
//-----------------------------------------------------------------------------

#define NAS_UE_NETL_MAXLEN 500
// TO DO
#define SVN_REV   "0.1"
// Global variables
int sd_graal, s_nas;
struct sockaddr_un ralu_socket;
int wait_start_mihf;
int listen_mih;

char message[NAS_UE_NETL_MAXLEN];
static int  g_log_output;
//-----------------------------------------------------------------------------
static void arg_usage(char *exec_nameP) {
//-----------------------------------------------------------------------------
    fprintf(stderr,
            "Usage: %s [options]\nOptions:\n"
            "  -V,          --version             Display version information\n"
            "  -?, -h,      --help                Display this help text\n"
            "  -P <number>, --ral-listening-port  Mistening port for incoming MIH-F messages\n"
            "  -I <string>, --ral-ip-address      Binding IP(v4 or v6) address for RAL\n"
            "  -p <number>, --mihf-remote-port    MIH-H remote port\n"
            "  -i <string>, --mihf-ip-address     MIH-F IP(v4 or v6) address\n"
            "  -c,          --output-to-console   All stream outputs are redirected to console\n"
            "  -f,          --output-to-syslog    All stream outputs are redirected to file\n"
            "  -s,          --output-to-syslog    All stream outputs are redirected to syslog\n",
            exec_nameP);
}

//---------------------------------------------------------------------------
int parse_opts(int argc, char *argv[]) {
//---------------------------------------------------------------------------
    static struct option long_opts[] = {
        {"version", 0, 0, 'V'},
        {"help", 0, 0, 'h'},
        {"ral-listening-port", optional_argument, 0, 'P'},
        {"ral-ip-address",     optional_argument, 0, 'I'},
        {"mihf-remote-port",   optional_argument, 0, 'p'},
        {"mihf-ip-address",    optional_argument, 0, 'i'},
        {"link.id",            optional_argument, 0, 'l'},
        {"mihf.id",            optional_argument, 0, 'm'},
        {"output-to-console",  0, 0, 'c'},
        {"output-to-file",     0, 0, 'f'},
        {"output-to-syslog",   0, 0, 's'},
        {0, 0, 0, 0}
    };

    /* parse all other cmd line parameters than -c */
    while (1) {
        int idx, c;
        c = getopt_long(argc, argv, "PIpil:Vh?cfs", long_opts, &idx);
        if (c == -1) break;

        switch (c) {
            case 'V':
                fprintf(stderr, "SVN MODULE VERSION: %s\n", SVN_REV);
                return -1;
            case '?':
            case 'h':
                arg_usage(basename(argv[0]));
                return -1;
            case 'i':
                fprintf(stderr, "Option mihf-ip-address:\t%s\n", optarg);
                g_mihf_ip_address = optarg;
                break;
            case 'p':
                fprintf(stderr, "Option mihf-remote-port:\t%s\n", optarg);
                g_mihf_remote_port = optarg;
                break;
            case 'P':
                fprintf(stderr, "Option ral-listening-port:\t%s\n", optarg);
                g_ral_listening_port_for_mihf = optarg;
                break;
            case 'I':
                fprintf(stderr, "Option ral-ip-address:\t%s\n", optarg);
                g_ral_ip_address = optarg;
                break;
            case 'l':
                fprintf(stderr, "Option link.id:\t%s\n", optarg);
                g_link_id = optarg;
                break;
            case 'm':
                fprintf(stderr, "Option mihf.id:\t%s\n", optarg);
                g_mihf_id = optarg;
                break;
            case 'c':
                fprintf(stderr, "Option output-to-console\n");
                g_log_output = LOG_TO_CONSOLE;
                break;
            case 'f':
                fprintf(stderr, "Option output-to-file\n");
                g_log_output = LOG_TO_FILE;
                break;
            case 's':
                fprintf(stderr, "Option output-to-syslog\n");
                g_log_output = LOG_TO_SYSTEM;
                break;
            default:
                WARNING("UNKNOWN OPTION\n");
                break;
        };
    }
    return 0;
}
//---------------------------------------------------------------------------
int inits(int argc, char *argv[]) {
    //---------------------------------------------------------------------------
    // Initialize defaults
    g_ral_ip_address                = DEFAULT_IP_ADDRESS_RAL;
    g_ral_listening_port_for_mihf   = DEFAULT_LOCAL_PORT_RAL;
    g_mihf_remote_port              = DEFAULT_REMOTE_PORT_MIHF;
    g_mihf_ip_address               = DEFAULT_IP_ADDRESS_MIHF;
    g_link_id                       = DEFAULT_LINK_ID;
    g_mihf_id                       = DEFAULT_MIHF_ID;
    sockd_mihf                      = -1;
    g_log_output                    = LOG_TO_CONSOLE;

    MIH_C_init(g_log_output);

    parse_opts( argc, argv);


    if (mRALlte_mihf_connect() < 0 ) {
        ERR("Could not connect to MIH-F...exiting\n");
        exit(-1);
    }
    return 0;
}

//---------------------------------------------------------------------------
int main(int argc, char *argv[]){
//---------------------------------------------------------------------------
    int            rc, done;
    fd_set         readfds;
    struct timeval tv;

    inits(argc, argv);



    done=0;

    do{
    	// Create fd_set and wait for input;
    	FD_ZERO(&readfds);
    	FD_SET(sockd_mihf, &readfds);
    	tv.tv_sec  = MIH_C_RADIO_POLLING_INTERVAL_SECONDS;
    	tv.tv_usec = MIH_C_RADIO_POLLING_INTERVAL_MICRO_SECONDS;

    	rc= select(FD_SETSIZE, &readfds, NULL, NULL, &tv);
    	if(rc == -1){
    		perror("select");
    		done = 1;
    	}

    	//something received!
    	if(rc >= 0){
    		if(FD_ISSET(sockd_mihf, &readfds)){
    			done=mRALlte_mih_link_process_message();
    		} else { // tick
                mRALlte_mih_fsm(NULL, 0);
            }
    	}
    }while(!done);

    close(sockd_mihf);
    MIH_C_exit();
    return 0;
}
