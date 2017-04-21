# Networks-Simulator-ns3-
Network Simulator ns3

#Assignment
Compare the performance of TCP over wired and wireless networks. Consider a topology as described below. Network consists of two TCP sources Node0 and Node2 and corresponding to two TCP destinations Node1 and Node3 respectively. Node2 and Node3 come in wired domain with two routers R1 and R2 (connected by a {10 Mbps, 50 ms} wired link) in between them. Both the routers use drop-tail queues with queue size set according to bandwidth - delay product. Node0 comes in domain of Base Station 1 (BS1) and Node1 comes in domain of Base Station 2 (BS2). BS1 and BS2 are connected by a (10 Mbps, 100 ms) wired link. The hosts (i.e., Node0, Node1, Node2, Node3 are attached with (100 Mbps, 20ms) links to routers or base stations. The sources use TCP Reno as the sending TCP agent. Study and plot the fairness index (Jain's fairness index) and throughput change when the TCP packet size is varied; all the other parameter values are kept constant. You should use the following TCPpacket size values (in Bytes): 40, 44, 48, 52, 60, 552, 576, 628, 1420 and 1500 for your experiments. The throughput (in Kbps) and fairness index must be calculated at steady-state. Make appropriate assumptions whereveris necessary.

#Simulation
Place the files in the scratch directory of your ns3 directory and execute the files using :
'''
./waf run WiredTCP
'''
'''
./waf run Wireless TCP
'''
This will generate plt files for plotting graphs.
'''
gnuplot wiredthroughput.plt
'''
The above command will generate Plot images.
