% file: struct_template.m
% author: florian.kaltenberger@eurecom.fr
% date: 13.3.2010
% purpose: this file reconstructs the structures fifo_dump_emos_struct_UE
%          and fifo_dump_emos_struct_UE which are defined in
%          SCHED/phy_procedures_emos.h


NUMBER_OF_OFDM_CARRIERS_EMOS = 512; 
NUMBER_OF_USEFUL_CARRIERS_EMOS = 300;

N_RB_DL_EMOS = 25;
N_PILOTS_PER_RB = 4;
N_SLOTS_EMOS = 2;

N_RB_UL_EMOS = 25;
N_PILOTS_PER_RB_UL = 12;
N_SRS_SYMBOLS = 5;

MAX_DCI_PER_FRAME = 20;
MAX_UCI_PER_FRAME = 20;

PBCH_PDU_SIZE = 6;

NUMBER_OF_eNB_MAX = 3;
NUMBER_OF_UE_MAX = 1;
NUMBER_OF_SUBBANDS = 7;

NB_ANTENNAS_RX = 2;
NB_ANTENNAS_TX = 2;

MAX_CQI_BITS = 40;
MAX_DCI_SIZE_BITS = 45;

phy_measurements_struct = struct(...
    'n0_power',             uint32(zeros(1,NB_ANTENNAS_RX)),...
    'n0_power_dB',          uint16(zeros(1,NB_ANTENNAS_RX)),...
    'n0_power_tot',         uint32(0),...
    'n0_power_tot_dB',      uint16(0),...
    'rx_spatial_power',     uint32(zeros(NUMBER_OF_eNB_MAX,2,2)),...
    'rx_spatial_power_dB',  uint16(zeros(NUMBER_OF_eNB_MAX,2,2)),...
    'rx_rssi_dBm',          int16(zeros(1,NUMBER_OF_eNB_MAX)),...
    'rx_correlation',       int32(zeros(NUMBER_OF_eNB_MAX,2)),...
    'rx_correlation_dB',    int32(zeros(NUMBER_OF_eNB_MAX,2)),...
    'wideband_cqi',         int32(zeros(NUMBER_OF_eNB_MAX,NB_ANTENNAS_RX)),...
    'wideband_cqi_dB',      int32(zeros(NUMBER_OF_eNB_MAX,NB_ANTENNAS_RX)),...
    'wideband_cqi_tot',     int32(zeros(1,NUMBER_OF_eNB_MAX)),...
    'subband_cqi',          int32(zeros(NUMBER_OF_eNB_MAX,NB_ANTENNAS_RX,NUMBER_OF_SUBBANDS)),...
    'subband_cqi_dB',       int32(zeros(NUMBER_OF_eNB_MAX,NB_ANTENNAS_RX,NUMBER_OF_SUBBANDS)),...
    'subband_cqi_tot_dB',   int32(zeros(NUMBER_OF_eNB_MAX,NB_ANTENNAS_RX)),...
    'wideband_pmi_re',      int32(zeros(NUMBER_OF_eNB_MAX,NB_ANTENNAS_RX)),...
    'wideband_pmi_im',      int32(zeros(NUMBER_OF_eNB_MAX,NB_ANTENNAS_RX)),...
    'subband_pmi_re',       int32(zeros(NUMBER_OF_eNB_MAX,NUMBER_OF_SUBBANDS,NB_ANTENNAS_RX)),...
    'subband_pmi_im',       int32(zeros(NUMBER_OF_eNB_MAX,NUMBER_OF_SUBBANDS,NB_ANTENNAS_RX)),...
    'selected_rx_antennas', int8(zeros(NUMBER_OF_eNB_MAX,NUMBER_OF_SUBBANDS)),...
    'rank',                 uint8(zeros(1,NUMBER_OF_eNB_MAX)));

phy_measurements_struct_a = cstruct(phy_measurements_struct);
if (phy_measurements_struct_a.size ~= PHY_measurements_size)
    disp('PHY_measurements_size does not match');
end

uci_data_struct = struct(...
    'o',uint8(zeros(1,MAX_CQI_BITS)),...
    'O',uint8(0),...
    'o_RI',uint8(zeros(1,2)),...
    'O_RI',uint8(0),...
    'o_ACK',uint8(zeros(1,4)),...
    'O_ACK',uint8(0));

uci_data_struct_a = cstruct(uci_data_struct);
if (uci_data_struct_a.size ~= UCI_data_t_size)
    disp('UCI_data_t_size does not match');
end


dci_alloc_struct = struct(...
    'dci_length',uint8(0),...
    'L',uint8(0),...
    'rnti',uint16(0),...
    'format',uint32(0),...
    'dci_pdu',uint8(zeros(1,1+floor(MAX_DCI_SIZE_BITS/8))));

dci_alloc_struct_a = cstruct(dci_alloc_struct);
if (dci_alloc_struct_a.size ~= DCI_alloc_t_size)
    disp('DCI_alloc_t_size does not match');
end


eNb_UE_stats_struct = struct(...
    'UL_rssi',int32(zeros(1,NUMBER_OF_UE_MAX)),...
    'DL_cqi',uint8(zeros(NUMBER_OF_UE_MAX,2)),...
    'DL_diffcqi',uint8(zeros(NUMBER_OF_UE_MAX,2)),...
    'DL_pmi_single',uint16(zeros(1,NUMBER_OF_UE_MAX)),...
    'DL_pmi_dual',uint16(zeros(1,NUMBER_OF_UE_MAX)),...
    'rank',uint8(zeros(1,NUMBER_OF_UE_MAX)),...
    'UE_id',uint16(zeros(1,NUMBER_OF_UE_MAX)),...
    'UE_timing_offset',uint16(zeros(1,NUMBER_OF_UE_MAX)));

eNb_UE_stats_struct_a = cstruct(eNb_UE_stats_struct);
if (eNb_UE_stats_struct_a.size ~= eNb_UE_stats_size)
    disp('eNb_UE_stats_size does not match');
end


fifo_dump_emos_struct_UE = struct(...
    'timestamp',int64(0),...
    'frame_tx',uint32(0),...
    'frame_rx',uint32(0),...
    'phy_measurements', repmat(phy_measurements_struct,1,20),... 
    'pbch_pdu',uint8(zeros(NUMBER_OF_eNB_MAX,PBCH_PDU_SIZE)),...
    'pdu_errors',uint32(zeros(1,NUMBER_OF_eNB_MAX)),...
    'pdu_errors_last',uint32(zeros(1,NUMBER_OF_eNB_MAX)),...
    'pdu_errors_conseq',uint32(zeros(1,NUMBER_OF_eNB_MAX)),...
    'pdu_fer',uint32(zeros(1,NUMBER_OF_eNB_MAX)),...
    'dci_cnt',uint32(zeros(1,10)),...
    'dci_errors',uint32(0),...
    'dci_received',uint32(0),...,
    'dci_alloc',repmat(dci_alloc_struct,2,10),...
    'timing_offset',int32(0),...
    'timing_advance',int32(0),...
    'freq_offset',int32(0),...
    'rx_total_gain_dB',uint32(0),...
    'eNb_id',uint8(0),...
    'mimo_mode',uint8(0),...
    'channel',int32(zeros(NUMBER_OF_eNB_MAX,NB_ANTENNAS_RX*NB_ANTENNAS_TX,N_RB_DL_EMOS*N_PILOTS_PER_RB*N_SLOTS_EMOS)),...
    'uci_cnt',uint32(zeros(1,10)),...
    'uci_data',repmat(uci_data_struct,2,10));

fifo_dump_emos_struct_UE_a = cstruct(fifo_dump_emos_struct_UE);
if (fifo_dump_emos_struct_UE_a.size ~= fifo_dump_emos_UE_size)
    disp('fifo_dump_emos_UE_size does not match');
end


fifo_dump_emos_struct_eNb = struct(...
    'timestamp',int64(0),...
    'frame_tx',uint32(0),...
    'dci_cnt',uint32(zeros(1,10)),...
    'dci_alloc',repmat(dci_alloc_struct,2,10),...
    'mimo_mode',uint8(0),...
    'eNb_UE_stats',repmat(eNb_UE_stats_struct,NUMBER_OF_eNB_MAX,10),...
    'rx_total_gain_dB',uint32(0),...
    'channel',int32(zeros(N_SRS_SYMBOLS,NUMBER_OF_eNB_MAX,NB_ANTENNAS_RX,N_RB_DL_EMOS*N_PILOTS_PER_RB)));

fifo_dump_emos_struct_eNb_a = cstruct(fifo_dump_emos_struct_eNb);
if (fifo_dump_emos_struct_eNb_a.size ~= fifo_dump_emos_eNb_size)
    disp('fifo_dump_emos_eNb_size does not match');
end


