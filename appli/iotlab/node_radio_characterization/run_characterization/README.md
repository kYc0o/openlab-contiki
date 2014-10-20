Running radio characterization
==============================

The script should be run from within iot-lab repository on the IoT-Lab server.

Howto
-----

 1. Start an experiment with the nodes you want to characterize.
 2. Flash `node_radio_characterization` on all the nodes
 3. The characterization can be configured in the `run_characterization.py` main
    function:
    * Channel
    * Power
    * Number of sent packets
    * Delay between packets sending
 4. Run the `run_test.sh` script. It will prepare the python path, and run the
    python script with the result of `experiment-cli get -r` as input and with
    given arguments.


### Output ###

Only the JSON is printed on `stdout`, the remaining info is on `stderr` so it
could be directly parsed by a json compatible program.

    harter@devgrenoble:~/iot-lab/parts/openlab/appli/iotlab/  \
        node_radio_characterization/run_characterization$     \
        ./run_test.sh  | head -n 40
    sending node m3-359
    sending node m3-364
    sending node m3-365
    sending node m3-366
    sending node m3-367
    sending node m3-360
    sending node m3-361
    sending node m3-362
    sending node m3-363
    sending node m3-368
    {
        "config": {
            "failure": [], 
            "success": [
                "m3-359", 
                "m3-365", 
                "m3-364", 
                "m3-361", 
                "m3-367", 
                "m3-366", 
                "m3-360", 
                "m3-362", 
                "m3-363", 
                "m3-368"
            ]
        }, 
        "options": {
            "channel": 16, 
            "delay": 10, 
            "num_pkts": 32, 
            "power": "-17dBm"
        }, 
        "radio": {
            "m3-359": {
                "m3-360": {
                    "errors": [], 
                    "success": [
                        [
                            "-17dBm",     # TX power
                            "0",          # Packet num
                            "-64",        # RSSI
                            "255"         # LQI
                        ], 
                        [
                            "-17dBm", 
                            "1", 
                            "-64", 
                            "255"
                        ], 



Options
-------

The script can be run with the following options:

 * --summary
 * --summary -h


### summary output ###

It provides:

 * the average of RSSI for the received packets
 * the percentage of received packets
 * the number of errors, if any (it was only CRC errors when I tested)

It does not provide:

 * any sort of average for the LQI, I don't know if it can be averaged so I did
   nothing with it. However it becomes interesting when nodes get packets with
   LQI < 255, so something could be made with it.



    harter@devgrenoble:~/iot-lab/parts/openlab/appli/iotlab/  \
        node_radio_characterization/run_characterization$     \
        ./run_test.sh  --summary
    sending node m3-359
    sending node m3-364
    sending node m3-365
    sending node m3-366
    sending node m3-367
    sending node m3-360
    sending node m3-361
    sending node m3-362
    sending node m3-363
    sending node m3-368
    {
        "config": {
            "failure": [], 
            "success": [
                "m3-359", 
                "m3-365", 
                "m3-364", 
                "m3-366", 
                "m3-361", 
                "m3-367", 
                "m3-360", 
                "m3-362", 
                "m3-363", 
                "m3-368"
            ]
        }, 
        "options": {
            "channel": 16, 
            "delay": 10, 
            "num_pkts": 32, 
            "power": "-17dBm"
        }, 
        "radio": {
            "m3-359": {
                "m3-360": {
                    "average_rssi": -64.0, 
                    "pkt_reception": "100.0 %"
                }, 
                "m3-361": {
                    "average_rssi": -59.0, 
                    "pkt_reception": "100.0 %"
                }, 

                ...

                "m3-366": {
                    "average_rssi": -76.61290322580645, 
                    "errors": 1, 
                    "pkt_reception": "96.9 %"
                ...


#### Summary with -h trigger ####

It removes the JSON compatibility to print nodes outputs on one line to be
human readable.

        ...
            "power": "-17dBm"
        }, 
        "radio": {
            "m3-359": {
                "m3-360": "{'average_rssi': -64.0, 'pkt_reception': '100.0 %'}", 
                "m3-361": "{'average_rssi': -59.03125, 'pkt_reception': '100.0 %'}", 
                "m3-362": "{'average_rssi': -70.0, 'pkt_reception': '100.0 %'}", 
                "m3-363": "{'average_rssi': -66.8125, 'pkt_reception': '100.0 %'}", 
                "m3-364": "{'average_rssi': -74.09375, 'pkt_reception': '100.0 %'}", 
                "m3-365": "{'average_rssi': -76.65625, 'pkt_reception': '100.0 %'}", 
                "m3-366": "{'average_rssi': -76.09677419354838, 'pkt_reception': '96.9 %', 'errors': 1}", 
                "m3-367": "{'average_rssi': -82.22222222222223, 'pkt_reception': '84.4 %'}", 
                "m3-368": "{'average_rssi': -90.96153846153847, 'pkt_reception': '81.2 %', 'errors': 1}"
            }, 
            "m3-360": {
                "m3-359": "{'average_rssi': -64.0, 'pkt_reception': '100.0 %'}", 
                "m3-361": "{'average_rssi': -62.34375, 'pkt_reception': '100.0 %'}", 
                "m3-362": "{'average_rssi': -66.96875, 'pkt_reception': '100.0 %'}", 
                "m3-363": "{'average_rssi': -70.0, 'pkt_reception': '100.0 %'}", 
                "m3-364": "{'average_rssi': -67.0, 'pkt_reception': '100.0 %'}", 
                "m3-365": "{'average_rssi': -72.125, 'pkt_reception': '100.0 %'}", 
                "m3-366": "{'average_rssi': -78.96774193548387, 'pkt_reception': '96.9 %'}", 
                "m3-367": "{'average_rssi': -85.96428571428571, 'pkt_reception': '87.5 %'}", 
                "m3-368": "{'average_rssi': -86.92592592592592, 'pkt_reception': '84.4 %', 'errors': 1}"
        ...


Notes
-----

The 'failure' field is not correctly managed. It only reports failures that
happen during the run, not the silent errors where nothing is reported by the
node.

The output can be updated, I didn't know what to do with the data, but if you
can do better, it would be great. Just think about updating this README if you
do it.

