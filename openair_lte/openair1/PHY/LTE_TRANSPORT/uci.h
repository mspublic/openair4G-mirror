typedef struct __attribute__((packed)) {
  unsigned char cqi1:4;
  unsigned short pmi:14; 
} HLC_wideband_cqi_rank1_2A_5MHz ; 

typedef struct __attribute__((packed)) {
  unsigned char cqi1:4;
  unsigned char cqi2:4;
  unsigned short pmi:7; 
} HLC_wideband_cqi_rank2_2A_5MHz ; 

typedef struct __attribute__((packed)) { 
  unsigned char cqi1:4;
  unsigned short diffcqi1:14;   
} HLC_subband_cqi_nopmi_5MHz;

typedef struct __attribute__((packed)) { 
  unsigned char cqi1:4;
  unsigned short diffcqi1:14;   
  unsigned char pmi:2;
} HLC_subband_cqi_rank1_5MHz;

typedef struct __attribute__((packed)) { 
  unsigned char cqi1:4;
  unsigned short diffcqi1:14;   
  unsigned char cqi2:4;
  unsigned short diffcqi2:14;   
  unsigned char pmi:1;
} HLC_subband_cqi_rank2_2A_5MHz;

typedef struct __attribute__((packed)) { 
  unsigned char cqi1:4;
  unsigned short diffcqi1:14;   
} HLC_subband_cqi_modes123_2A_5MHz;


#define MAX_CQI_PAYLOAD (sizeof(HLC_subband_cqi_rank2_2A_5MHz)*8*20)
#define MAX_CQI_BITS (sizeof(HLC_subband_cqi_rank2_2A_5MHz))
#define MAX_ACK_PAYLOAD 18
#define MAX_RI_PAYLOAD 6
