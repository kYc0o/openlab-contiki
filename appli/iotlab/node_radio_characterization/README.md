Radio characterization firmware
===============================

Firmware accepted commands
--------------------------
    config_radio -c <channel:[11-26]>
    config_radio -c 11
    config_radio -c 26

    send_packets -i <node_id> -p <power> -n <num_pkts> -d <delay_between_pkts>
    send_packets -i m3-128 -p 0dBm -n 10 -d 100

    send_packets -i m3-128 -p 3dBm -n 1 -d 0
    send_packets -i m3-128 -p -17dBm -n 32 -d 10


For valid power values, see the source file in `parse_power` function.


Example
-------

    # On first node
    config_radio -c 26
    # On second node
    config_radio -c 26
    send_packets -i m3-128 -p -17dBm -n 32 -d 10

    # First node output:
    radio_rx m3-128 -17dBm 0 -61 255 sender power num rssi lqi
    radio_rx m3-128 -17dBm 1 -61 255 sender power num rssi lqi
    radio_rx m3-128 -17dBm 2 -61 255 sender power num rssi lqi
    radio_rx m3-128 -17dBm 3 -61 255 sender power num rssi lqi
    radio_rx m3-128 -17dBm 4 -61 255 sender power num rssi lqi
    radio_rx m3-128 -17dBm 5 -61 255 sender power num rssi lqi
    radio_rx m3-128 -17dBm 6 -61 255 sender power num rssi lqi
    radio_rx m3-128 -17dBm 7 -61 255 sender power num rssi lqi
    radio_rx m3-128 -17dBm 8 -61 255 sender power num rssi lqi
    radio_rx m3-128 -17dBm 9 -61 255 sender power num rssi lqi
    radio_rx m3-128 -17dBm 10 -61 255 sender power num rssi lqi
    radio_rx m3-128 -17dBm 11 -61 255 sender power num rssi lqi
    radio_rx m3-128 -17dBm 12 -61 255 sender power num rssi lqi
    radio_rx m3-128 -17dBm 13 -61 255 sender power num rssi lqi
    radio_rx m3-128 -17dBm 14 -61 255 sender power num rssi lqi
    radio_rx m3-128 -17dBm 15 -61 255 sender power num rssi lqi
    radio_rx m3-128 -17dBm 16 -61 255 sender power num rssi lqi
    radio_rx m3-128 -17dBm 17 -61 255 sender power num rssi lqi
    radio_rx m3-128 -17dBm 18 -61 255 sender power num rssi lqi
    radio_rx m3-128 -17dBm 19 -61 255 sender power num rssi lqi
    radio_rx m3-128 -17dBm 20 -61 255 sender power num rssi lqi
    radio_rx m3-128 -17dBm 21 -61 255 sender power num rssi lqi
    radio_rx m3-128 -17dBm 22 -61 255 sender power num rssi lqi
    radio_rx m3-128 -17dBm 23 -61 255 sender power num rssi lqi
    radio_rx m3-128 -17dBm 24 -61 255 sender power num rssi lqi
    radio_rx m3-128 -17dBm 25 -61 255 sender power num rssi lqi
    radio_rx m3-128 -17dBm 26 -61 255 sender power num rssi lqi
    radio_rx m3-128 -17dBm 27 -61 255 sender power num rssi lqi
    radio_rx m3-128 -17dBm 28 -61 255 sender power num rssi lqi
    radio_rx m3-128 -17dBm 29 -61 255 sender power num rssi lqi
    radio_rx m3-128 -17dBm 30 -61 255 sender power num rssi lqi
    radio_rx m3-128 -17dBm 31 -61 255 sender power num rssi lqi


