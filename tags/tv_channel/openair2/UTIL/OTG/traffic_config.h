




#define MAX_NUM_NODE_TYPES 2 // enb/ch and ue/mr
#define MAX_NUM_TRAFFIC_STATE 3 // enb/ch and ue/mr
#define MAX_NUM_NODES  2// 12 total number of nodes in the emulation scneario, we may use grouping to aggregate nodes, at rlc this could be rb id


//IDT DISTRIBUTION PARAMETERS
#define IDT_DIST POISSON
#define IDT_MIN 2 // unit second
#define IDT_MAX 10
#define IDT_STD_DEV 1
#define IDT_LAMBDA 3

//DATA PACKET SIZE DISTRIBUTION PARAMETERS
#define PKTS_SIZE_DIST UNIFORM  //unit packet per second 
#define PKTS_SIZE_MIN 17
#define PKTS_SIZE_MAX 1024
#define PKTS_SIZE_STD_DEV 1
#define PKTS_SIZE_LAMBDA 6

//SOCKET MODE
#define DST_PORT 1234;
#define DST_IP "127.0.0.1"
