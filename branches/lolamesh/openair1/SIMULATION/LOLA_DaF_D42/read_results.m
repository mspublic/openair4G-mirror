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
%      tx: struct array of length n_pdu containing the following fields:
%        n_bytes: number of transmitted source bytes for this PDU
%        n_rounds_hop1: number of rounds in hop 1
%        rounds_hop1: struct array of length n_rounds_hop1 containing the following fields:
%          frame: LTE frame number of this transmission
%          subframe: subframe number of this transmission
%          tbs: TBS used
%          mcs: MCS used
%          n_prb: number of PRBs used
%          n_sent_bits: number of transmitted coded bits
%          n_correct_bits: vector of size n_relays of correctly received bits at each relay
%          decoded: vector of size n_relays (1 if decoded at MR(k), 0 otherwise)
%        n_rounds_hop2: number of rounds in hop 2
%        rounds_hop2: struct array of length n_rounds_hop2 containing the following fields:
%          frame: LTE frame number of this transmission
%          subframe: subframe number of this transmission
%          tbs: TBS used
%          mcs: MCS used
%          n_prb: number of PRBs used
%          n_sent_bits: number of transmitted coded bits
%          n_correct_bits: number of correctly received bits at CH1
%          active: vector of size n_relays (1 if MR(k) was active, 0 otherwise)
%          decoded: 1 if decoded at MR1, 0 otherwise
%      stats: struct array containing the following fields:
%        n_tx_hop{1,2}: number of transmitted LTE frames in hop {1,2}
%        ber_hop1: vector of average raw BER at relays
%        ber_hop2: average raw BER at destination CH
%        n_harq_tries_hop1: vector of transmitted MAC PDUs in each HARQ round in hop 1
%        n_harq_success_hop1: matrix of successfully decoded MAC PDUs, rows: MR index, cols: HARQ round
%        n_harq_tries_hop2: vector of transmitted MAC PDUs in each HARQ round in hop 2
%        n_harq_success_hop2: vector of successfully decoded MAC PDUs in each HARQ round in hop 2
%        relay_activity: number of transmissions with different cooperation level of relays, 
%            for n_relays==2 there are four values: [0 MR1 MR2 MR1+MR2], this is for all transmissions
%            over hop 2, even if the PDU was not finally received at the destination CH
%        throughput: average throughput over colaborative link in bits/s
%        spectral_eff: average spectral efficiency over colaborative link in bits/s/Hz
%        latency: average latency over colaborative link in ms
%
% See colabsim --info for definition of statistics

fid = fopen(f, 'r');

A = mread(fid, 1, 5);
r.n_relays = A(1);
r.channel_model = parse_channel(A(2));
r.n_tests = A(3);
r.n_pdu = A(4);
r.n_harq = A(5);

n_tests = A(3);

for test = 1:n_tests
	r.tests(test) = read_test(fid, r.n_relays, r.n_pdu, r.n_harq);
end

fclose(fid);

for test = 1:n_tests
	r.tests(test).stats = compute_stats(r.tests(test).tx, r.n_relays, r.n_harq);
end


function test = read_test(fid, n_relays, n_pdu, n_harq)

test.snr_hop1 = mread(fid, 1, n_relays);
test.snr_hop2 = mread(fid, 1, n_relays);
for k = 1:n_pdu
	clear pdu
	A = mread(fid, 1, 3);
	pdu.n_bytes = A(1);
	pdu.n_rounds_hop1 = A(2);
	pdu.n_rounds_hop2 = A(3);
	for h = 1:pdu.n_rounds_hop1
		A = mread(fid, 1, 7+n_relays);
		pdu.rounds_hop1(h).frame = A(1);
		pdu.rounds_hop1(h).subframe = A(2);
		pdu.rounds_hop1(h).tbs = A(3);
		pdu.rounds_hop1(h).mcs = A(4);
		pdu.rounds_hop1(h).n_prb = A(5);
		pdu.rounds_hop1(h).n_sent_bits = A(6);
		pdu.rounds_hop1(h).n_correct_bits = A(7:6+n_relays);
		pdu.rounds_hop1(h).decoded = fliplr(dec2bin(A(7+n_relays),n_relays)=='1');
	end
	for h = 1:pdu.n_rounds_hop2
		A = mread(fid, 1, 9);
		pdu.rounds_hop2(h).frame = A(1);
		pdu.rounds_hop2(h).subframe = A(2);
		pdu.rounds_hop2(h).tbs = A(3);
		pdu.rounds_hop2(h).mcs = A(4);
		pdu.rounds_hop2(h).n_prb = A(5);
		pdu.rounds_hop2(h).n_sent_bits = A(6);
		pdu.rounds_hop2(h).n_correct_bits = A(7);
		pdu.rounds_hop2(h).active = fliplr(dec2bin(A(8),n_relays)=='1');
		pdu.rounds_hop2(h).decoded = A(9);
	end
	test.tx(k) = pdu;
end


function stats = compute_stats(tx, n_relays, n_harq)

% Calculate n_tx
stats.n_tx_hop1 = sum([tx.n_rounds_hop1]);
stats.n_tx_hop2 = sum([tx.n_rounds_hop2]);

% Calculate ber
n_sent_hop1 = zeros(1,n_relays);
n_correct_hop1 = zeros(1,n_relays);
n_sent_hop2 = 0;
n_correct_hop2 = 0;
for k = 1:length(tx)
	for n = 1:n_relays
		for h = 1:tx(k).n_rounds_hop1
			n_sent_hop1(n) = n_sent_hop1(n) + tx(k).rounds_hop1(h).n_sent_bits;
			n_correct_hop1(n) = n_correct_hop1(n) + tx(k).rounds_hop1(h).n_correct_bits(n);
			if tx(k).rounds_hop1(h).decoded(n)==1
				break
			end
		end
	end
	n_sent_hop2 = n_sent_hop2 + sum([tx(k).rounds_hop2.n_sent_bits]);
	n_correct_hop2 = n_correct_hop2 + sum([tx(k).rounds_hop2.n_correct_bits]);
end
stats.ber_hop1 = (n_sent_hop1-n_correct_hop1)./n_sent_hop1;
stats.ber_hop2 = (n_sent_hop2-n_correct_hop2)/n_sent_hop2;

% Calculate HARQ stats
stats.n_harq_tries_hop1 = zeros(1, n_harq);
stats.n_harq_success_hop1 = zeros(n_relays, n_harq);
stats.n_harq_tries_hop2 = zeros(1, n_harq);
stats.n_harq_success_hop2 = zeros(1, n_harq);
for k = 1:length(tx)
	stats.n_harq_tries_hop1(1:tx(k).n_rounds_hop1) = stats.n_harq_tries_hop1(1:tx(k).n_rounds_hop1) + 1;
	for h = 1:tx(k).n_rounds_hop1
		stats.n_harq_success_hop1([tx(k).rounds_hop1(h).decoded]==1, h) = stats.n_harq_success_hop1([tx(k).rounds_hop1(h).decoded]==1, h) + 1;
	end
	stats.n_harq_tries_hop2(1:tx(k).n_rounds_hop2) = stats.n_harq_tries_hop2(1:tx(k).n_rounds_hop2) + 1;
	stats.n_harq_success_hop2([tx(k).rounds_hop2.decoded]==1) = stats.n_harq_success_hop2([tx(k).rounds_hop2.decoded]==1) + 1;
end

% Calculate relay activity
stats.relay_activity = zeros(1, 2^n_relays);
for k = 1:length(tx)
	for h = 1:tx(k).n_rounds_hop2
		stats.relay_activity(1+sum(2.^(n_relays-1:-1:0).*fliplr(tx(k).rounds_hop2(h).active))) = stats.relay_activity(1+sum(2.^(n_relays-1:-1:0).*fliplr(tx(k).rounds_hop2(h).active)))+1;
	end
end

% Calculate throughput, spectral efficiency and latency
n_bytes = 0;
n_prb = 0;
n_subframes = 0;
n_pdu = 0;
for k = 1:length(tx)
	f = find([tx(k).rounds_hop2.decoded]==1);
	if(length(f) >= 1)
		sfstart = 10*tx(k).rounds_hop1(1).frame+tx(k).rounds_hop1(1).subframe;
		sfend = 10*tx(k).rounds_hop2(f(1)).frame+tx(k).rounds_hop2(f(1)).subframe;
		n_subframes = n_subframes + sfend - sfstart;
		n_pdu = n_pdu + 1;
		n_bytes = n_bytes + tx(k).n_bytes;
	end
	n_prb = n_prb + sum([tx(k).rounds_hop1.n_prb]) + sum([tx(k).rounds_hop2.n_prb]);
end

stats.throughput = 8*n_bytes/(max(stats.n_tx_hop1,stats.n_tx_hop2)*0.01);
stats.spectral_eff = 8*n_bytes/(n_prb*12*15e3*1e-3);
stats.latency = n_subframes/n_pdu;


function s = parse_channel(c)

if c == 0
	s = 'custom';
elseif c == 1
	s = 'SCM_A';
elseif c == 2
	s = 'SCM_B';
elseif c == 3
	s = 'SCM_C';
elseif c == 4
	s = 'SCM_D';
elseif c == 5
	s = 'EPA';
elseif c == 6
	s = 'EVA';
elseif c == 7
	s = 'ETU';
elseif c == 8
	s = 'Rayleigh8';
elseif c == 9
	s = 'Rayleigh1';
elseif c == 10
	s = 'Rayleigh1_corr';
elseif c == 12
	s = 'Rayleigh1_anticorr';
elseif c == 13
	s = 'Rice8';
elseif c == 14
	s = 'Rice1';
elseif c == 15
	s = 'Rice1_corr';
elseif c == 16
	s = 'Rice1_anticorr';
elseif c == 17
	s = 'AWGN';
end


function A = mread(fid, nrow, ncol)

A = fscanf(fid, '%f', [ncol,nrow])';

