#!/bin/sh
rm -f /usr/local/src/mipv6-daemon-umip-0.4/logs/pcap2msc.output ; 
rm -f /usr/local/src/mipv6-daemon-umip-0.4/logs/pmip.png ; 

python /usr/local/src/mipv6-daemon-umip-0.4/extras/output-process/pcap2msc /usr/local/src/mipv6-daemon-umip-0.4/logs/lma2mags.pcap \
			/usr/local/src/mipv6-daemon-umip-0.4/logs/lma2cn.pcap \
			/usr/local/src/mipv6-daemon-umip-0.4/logs/mag12ap.pcap \
			/usr/local/src/mipv6-daemon-umip-0.4/logs/mag12lma.pcap  \
			/usr/local/src/mipv6-daemon-umip-0.4/logs/mag22ap.pcap \
			/usr/local/src/mipv6-daemon-umip-0.4/logs/mag22lma.pcap    > /usr/local/src/mipv6-daemon-umip-0.4/logs/pcap2msc.output ;
sync;

python /usr/local/src/mipv6-daemon-umip-0.4/extras/output-process/pcap2msc_check.py  /usr/local/src/mipv6-daemon-umip-0.4/logs/pcap2msc.output

python /usr/local/src/mipv6-daemon-umip-0.4/extras/output-process/pcap2msc_filter.py /usr/local/src/mipv6-daemon-umip-0.4/logs/pcap2msc.output


cat /usr/local/src/mipv6-daemon-umip-0.4/logs/pcap2msc.output.icmp_filtered | mscgen -T png -o /usr/local/src/mipv6-daemon-umip-0.4/logs/pmip_icmp_filtered.png


eog /usr/local/src/mipv6-daemon-umip-0.4/logs/pmip_icmp_filtered.png &
