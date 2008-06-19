/** @addtogroup _mesh_layer3_ 
  * @{ This page describes the interface between the RRC and RRM/RRCI modules for OpenAirInterface Mesh.
  */

/*!\brief MAC Logical Channel Descriptor
 */
typedef struct {
  unsigned short transport_block_size;                  /*!< \brief Minimum PDU size in bytes provided by RLC to MAC layer interface */
  unsigned short max_transport_blocks;                  /*!< \brief Maximum PDU size in bytes provided by RLC to MAC layer interface */
  unsigned long  Guaranteed_bit_rate;           /*!< \brief Guaranteed Bit Rate (average) to be offered by MAC layer scheduling*/
  unsigned long  Max_bit_rate;                  /*!< \brief Maximum Bit Rate that can be offered by MAC layer scheduling*/
  unsigned char  Delay_class;                  /*!< \brief Delay class offered by MAC layer scheduling*/
  unsigned char  Target_bler;                  /*!< \brief Target Average Transport Block Error rate*/
  unsigned char  LCHAN_t;                      /*!< \brief Logical Channel Type (BCCH,CCCH,DCCH,DTCH_B,DTCH,MRBCH)*/
} LCHAN_DESC;

/*!\brief This primitive parametrizes a MAC measurement process
 */
typedef struct {
  MAC_MEAS_T Meas_trigger;      /*!< \brief Thresholds to trigger event driven measurement reports*/
  MAC_AVG_T Mac_avg;            /*!< \brief Set of forgetting factors for the averaging of the MAC measurement process*/
  unsigned int bo_forgetting_factor; /*!< \brief Forgetting factor for RLC buffer occupancy averaging*/
  unsigned int sdu_loss_trigger; /*!< \brief Trigger for RLC sdu losses*/
  unsigned short Rep_amount;    /*!< \brief Number of Measurements for this process, 0 means infinite*/
  unsigned short Rep_interval;  /*!< \brief Reporting interval between successive measurement reports in this process*/
} MAC_RLC_MEAS_DESC;

/*!\brief Radio Bearer ID descriptor
 */
typedef unsigned short RB_ID;

/*!\brief Layer 2 Identifier
 */
typedef struct {
  unsigned char L2_id[8];
} L2_ID;

/*!\brief MAC/RLC Measurement Information
 */
typedef struct{
  char Rssi;                        /*!< \brief RSSI (dBm) on physical resources corresponding to logical channel*/
  char Sinr[NUMBER_OF_MEASUREMENT_SUBBANDS];                        /*!< \brief Average SINR (dB) on physical resources corresponding to logical channel*/
  unsigned char Harq_delay;         /*!< \brief Average number of transmission rounds (times 10) on transport channel associated with 
				      logical channel*/
  unsigned short Bler;              /*!< \brief Average block error rate (times 1000) on transport channel associated with logical channel*/
  unsigned char Spec_eff;           /*!< \brief Actual Spectral efficiency (bits/symbol times 10) of transport channel associated with logical channel*/
  unsigned char  rlc_sdu_buffer_occ;    /*!< \brief RLC SDU buffer occupancy */
  unsigned short rlc_sdu_loss_indicator; /*!< \brief RLC SDU Loss indicator */
}MAC_RLC_MEAS_T;
#define MAC_RLC_MEAS_T_SIZE sizeof(MAC_RLC_MEAS_T)


/*!\brief Measurement Mode
 */
typedef enum {
  PERIODIC=0,   /*!< Periodic measurement*/
  EVENT_DRIVEN  /*!< Event-driven measurement*/
} MEAS_MODE;

/*!\brief Sensing measurement descriptor
 */
typedef struct {
  unsigned char RSSI_Threshold;    /*!< Threshold (minus in dBm) for neighbour RSSI measurement*/
  unsigned char RSSI_F_Factor;    /*!< Forgetting factor for RSSI averaging*/
  unsigned short Rep_interval;  /*!< \brief Reporting interval between successive measurement reports in this process*/
} SENSING_MEAS_DESC;

/*!\brief Sensing measurement information
 */
typedef struct {
  unsigned char RSSI;    /*!< RSSI (minus in dBm) for neighbour*/
  L2_ID L2_id;           /*!< Layer 2 ID for neighbour*/
} SENSING_MEAS_T;

/*!\brief Layer 3 Info types for RRC messages
 */
typedef enum {
  IPv4_ADDR=0,   /*!< IPv4 Address*/
  IPv6_ADDR     /*!< IPv6 Address*/
} L3_INFO_T;

/** @} */
