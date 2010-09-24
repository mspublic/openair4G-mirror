This file contains the details of the simulators used for BLER  and Throughput measurements for a 2 Relay Distributed System:

1. relays_sim: To plot the overall BLER vs Uplink SNR, uplink BLER vs Uplink SNR for a fixed value of downlink SNR. It also plots Throughput vs Uplink SNR for the entire system.

4. relays_sim_harq: It implements harq for cooperative schemes and plots the BLER vs Uplink SNR and Throughput vs Uplink SNR for the entire system


Flags to change: 1. COLLABRATIVE_SCHEME: If not set, then the simulator is for a single relay case. If set, then it performs cooperative schemes
                 2. relay_flag: If 1, then only one relay. If 2, then 2 relays   
                 3. diversity_scheme: If 0, then no diversity scheme. If 1, then Delay Diversity Scheme and if 2, then Distributed Alamouti Scheme
     
 
 
