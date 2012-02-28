function r = read_results(f)
%r = read_results(f)
%
%Reads colabsim simulation results from file f.
%
%Arguments:
%  f - results file (produced with -r option to colabsim)
%
%Returns:
%  r - results structure with the following fields:
%    n_relays: number of relays
%    channel_model: the used channel model as a string
%    n_tests: the number of tests done (each with different SNR)
%    n_pdu: the number of sent MAC PDUs for each test
%    n_harq: the maximum number of HARQ rounds
%    tests: struct array of length n_tests containing the following fields:
%      snr_hop{1,2}: SNRs for each link in hop {1,2}
%      n_frames_hop{1,2}: number of transmitted LTE frames in hop {1,2}
%      n_bits_hop{1,2}: number of correctly received information bits in hop {1,2}
%      n_pdu_success_hop{1,2}: number of correctly received MAC PDUs in hop {1,2}
%      ber_hop1: vector of average raw BER at relays
%      ber_hop2: average raw BER at destination CH
%      n_harq_tries_hop{1,2}: number of transmitted MAC PDUs in each HARQ round in hop {1,2}
%      n_harq_success_hop{1,2}: number of successfully decoded MAC PDUs in each HARQ round in hop {1,2}
%      mcs_hop{1,2}: MCS used in each transmission in hop {1,2}
%      tbs_hop{1,2}: TBS used in each transmission in hop {1,2}
%      n_rb_hop{1,2}: number of resource blocks used in each transmission in hop {1,2}
%      n_transmissions: distribution of the number of required transmissions in each hop for the
%          successfully decoded MAC PDUs, n_transmissions(n1,n2) is the number of successfully decoded
%          MAC PDUs that required n1 transmissions in hop 1 and n2 transmissions in hop 2
%      relay_activity: number of transmissions with different cooperation level of relays, 
%          for n_relays==2 there are four values: [0 MR1 MR2 MR1+MR2], this is for all transmissions
%          over hop 2, even if the PDU was not finally received at the destination CH

A = dlmread(f, ' ', [0 0 0 4]);
r.n_relays = A(1);
r.channel_model = parse_channel(A(2));
r.n_tests = A(3);
r.n_pdu = A(4);
r.n_harq = A(5);

n_relays = A(1);
n_tests = A(3);
n_pdu = A(4);
n_harq = A(5);

test_row = 1;
for test = 1:n_tests
  r.tests(test).snr_hop1 = dlmread(f, ' ', [test_row 0 test_row n_relays-1]);
  r.tests(test).snr_hop2 = dlmread(f, ' ', [test_row+1 0 test_row+1 n_relays-1]);
  A = dlmread(f, ' ', [test_row+2 0 test_row+2 5]);
  r.tests(test).n_frames_hop1 = A(1);
  r.tests(test).n_frames_hop2 = A(2);
  r.tests(test).n_bits_hop1 = A(3);
  r.tests(test).n_bits_hop2 = A(4);
  r.tests(test).n_pdu_success_hop1 = A(5);
  r.tests(test).n_pdu_success_hop2 = A(6);
  r.tests(test).ber_hop1 = dlmread(f, ' ', [test_row+3 0 test_row+3 n_relays-1]);
  r.tests(test).ber_hop2 = dlmread(f, ' ', [test_row+3 n_relays test_row+3 n_relays]);
  r.tests(test).n_harq_tries_hop1 = dlmread(f, ' ', [test_row+4 0 test_row+4 n_harq-1]);
  r.tests(test).n_harq_success_hop1 = dlmread(f, ' ', [test_row+5 0 test_row+5 n_harq-1]);
  r.tests(test).n_harq_tries_hop2 = dlmread(f, ' ', [test_row+6 0 test_row+6 n_harq-1]);
  r.tests(test).n_harq_success_hop2 = dlmread(f, ' ', [test_row+7 0 test_row+7 n_harq-1]);
  r.tests(test).mcs_hop1 = dlmread(f, ' ', [test_row+8+n_pdu*0 0 test_row+8+n_pdu*1-1 n_harq-1]);
  r.tests(test).mcs_hop2 = dlmread(f, ' ', [test_row+8+n_pdu*1 0 test_row+8+n_pdu*2-1 n_harq-1]);
  r.tests(test).tbs_hop1 = dlmread(f, ' ', [test_row+8+n_pdu*2 0 test_row+8+n_pdu*3-1 n_harq-1]);
  r.tests(test).tbs_hop2 = dlmread(f, ' ', [test_row+8+n_pdu*3 0 test_row+8+n_pdu*4-1 n_harq-1]);
  r.tests(test).n_rb_hop1 = dlmread(f, ' ', [test_row+8+n_pdu*4 0 test_row+8+n_pdu*5-1 n_harq-1]);
  r.tests(test).n_rb_hop2 = dlmread(f, ' ', [test_row+8+n_pdu*5 0 test_row+8+n_pdu*6-1 n_harq-1]);
  r.tests(test).n_transmissions = dlmread(f, ' ', [test_row+8+n_pdu*6 0 test_row+8+n_pdu*6+n_harq-1 n_harq-1])';
  r.tests(test).relay_activity = dlmread(f, ' ', [test_row+8+n_pdu*6+n_harq 0 test_row+8+n_pdu*6+n_harq 2^n_relays-1]);
  test_row = test_row + 8 + n_pdu*6+n_harq+1;
end

function s = parse_channel(c)

s = sprintf('%d', c);

