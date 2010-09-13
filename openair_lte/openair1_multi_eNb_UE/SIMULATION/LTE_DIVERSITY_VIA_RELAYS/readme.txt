This file contains the details of the simulators used for BLER measurements for a 2 Relay Distributed System:

1. relays_sim_dl.c : To plot the BLER (phase 1) vs Downlink SNR for the two links in the 1st phase. Such plot is useful to indicate the downlink SNR for which Cooperative Schemes can be implemented at the two relays

2. relays_sim_ul.c: To plot the BLER (phase 2) vs Uplink SNR at a fixed value of downlink SNR. These plots measure the performance of the cooperative schemes with respect to a single link case in phase 2. 

3. relays_sim_overall: To plot the overall BLER vs Uplink SNR, uplink BLER vs Uplink SNR for a fixed value of downlink SNR. The number of trils run over entire system i.e. downlink and uplink rather than over individual phase. 


Flags to change: 1. COLLABRATIVE_SCHEME: If not set, then the simulator is for a single relay case. If set, then it performs cooperative schemes
                 2. relay_flag: If 1, then only one relay. If 2, then 2 relays   
                 3. diversity_scheme: If 0, then no diversity scheme. If 1, then Delay Diversity Scheme and if 2, then Distributed Alamouti Scheme
     
 
 
