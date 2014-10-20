ZEP SNIFFER and GPS SYNCED timestamps
=====================================


Overview
--------

4 components:
- sniffer outputing zep
- gps timestamping lib
- serial2loopback.py client script
- `send_absolute_time.py` client script

Running a demo
--------------

- 2 a8 nodes
  - 1 sniffer node (M3 on a8)
  - 1 emmiter node (M3)

sniffer node needs python script serial2loopback.py on A8

0. build emmiter and sniffer firmwares
1. flash emmiter node (a8-3)
2. flash sniffer node (a8-2)
3. run ./send_absolute_time.py
4. run ./serial2loopback.py
5. run tcpdump -vvv -i lo
6. send packet on emmiter node
7. check tcpdump output to see zep packet
```

cd build.a8
make tutorial_m3 zep_sniffer

scp bin/zep_sniffer.elf a8-2:
scp bin/tutorial_m3.elf a8-3:

ssh a8-2 flash_a8.sh zep_sniffer.elf

ssh a8-3 flash_a8.sh tutorial_m3.elf

scp send_absolute_time.py a8-2:
scp serial2loopback.py a8-2:
ssh a8-2 ./send_absolute_time.py &
ssh a8-2 ./serial2loopback.py &
ssh a8-2 tcpdump -vvv -i lo


check sniffer packets using wireshark:
ssh a8-2 tcpdump -vvv -i lo -w dump.pcap
^C
scp a8-2:dump.pcap .
wireshark dump.pcap

see timestamp, sequence number (UDP packets)
```

