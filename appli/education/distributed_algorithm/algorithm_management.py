# -*- coding:utf-8 -*-
import time
import random


def broadcast_slow(aggregator, message, delay=0.05):
    """ Send message to all nodes with 'delay' between sends """
    for node in aggregator.iterkeys():
        aggregator._send(node, message)
        time.sleep(delay)


def random_gossip_send(aggregator, message, delay=0.05):
    """ Send message to a random node """
    node = random.choice(aggregator.keys())
    aggregator._send(node, message)
    time.sleep(delay)


def init_network(aggregator):
    """ Init the values, create the network and print the original values """
    broadcast_slow(aggregator, 'i', 0)   # init sensors values and reset graphs
    broadcast_slow(aggregator, 'v', 0) # get values

    # create network
    broadcast_slow(aggregator, 't', 0)  # set low tx power
    time.sleep(0.5)
    broadcast_slow(aggregator, 'g')  # create connection graph

    # validate graph
    broadcast_slow(aggregator, 'T', 0)  # set high tx power
    time.sleep(0.5)
    broadcast_slow(aggregator, 'G')  # validate graph with neighbours

    # print graph
    broadcast_slow(aggregator, 'p') # get nodes graph


def syncronous_mode(aggregator, loop_number):
    """ Run messages sending and all in syncronous mode """
    init_network(aggregator)

    for _ in range(0, loop_number):
        broadcast_slow(aggregator, 's')  # send values to all nodes
        broadcast_slow(aggregator, 'c', 0)  # compute new value
        broadcast_slow(aggregator, 'v', 0)  # get values

def gossip_mode(aggregator, loop_number):
    """ Run messages sending and all in syncronous mode """
    init_network(aggregator)

    for _ in range(0, loop_number):
        random_gossip_send(aggregator, 'S')
        broadcast_slow(aggregator, 'v', 0)  # get values

def find_num_node_gossip_mode(aggregator, loop_number):
    """ Find the number of nodes after having run in gossip """
    gossip_mode(aggregator, loop_number)
    broadcast_slow(aggregator, 'V', 0)
