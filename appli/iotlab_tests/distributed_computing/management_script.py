#! /usr/bin/env python
# -*- coding:utf-8 -*-

""" Get the iotlab-uip for all experiment nodes """

import sys
import time
import subprocess

import serial_aggregator
import iotlabcli
from iotlabcli.parser import common as common_parser
from iotlabcli.parser import node as node_parser

import algorithm_management


def opts_parser():
    """ Argument parser object """
    import argparse
    parser = argparse.ArgumentParser()
    common_parser.add_auth_arguments(parser)

    nodes_group = parser.add_argument_group(
        description="By default, select currently running experiment nodes",
        title="Nodes selection")

    nodes_group.add_argument('-i', '--id', dest='experiment_id', type=int,
                             help='experiment id submission')

    nodes_group.add_argument(
        '-l', '--list', type=node_parser.nodes_list_from_str,
        dest='nodes_list', help='nodes list, may be given multiple times')

    nodes_group.add_argument('-a', '--algo', default='syncronous',
                             choices=ALGOS.keys(), help='Algorithm to run')

    nodes_group.add_argument(
        '-n', '--num-loop', type=int, required=True,
        dest='num_loop', help='number_of_loops_to_run')

    nodes_group.add_argument(
        '-o', '--out-file', required=True,
        dest='outfile', help='Files where to output traces')

    return parser

class NodeResults(object):

    def __init__(self, outfilename):
        self.outfilename = outfilename
        self.neighbours = {}

        self.node_measures = {}
        self.node_finale_measures = {}

    def handle_line(self, identifier, line):
        """ Print one line prefixed by id in format: """
        _ = identifier
        if line.startswith(('DEBUG', 'INFO', 'ERROR')):
            return

        if 'Neighbours' in line:
            # A569;Neighbours;6;A869;A172;C280;9869;B679;A269
            values = line.split(';')
            node = values[0]
            neighbours = values[3:]
            self.neighbours[node] = neighbours
            return

        if 'Values' in line:
            # A869;Values;100;1.9330742E9;2.0307378E9
            items = line.split(';')
            node = items[0]
            num_compute = int(items[2])
            values = [str(float(val)) for val in items[3:]]

            values_list = self.node_measures.setdefault(node, [])
            values_d = {
                'num_compute': num_compute,
                'values': values
            }
            values_list.append(values_d)
            return

        if 'FinalValue' in line:
            # A869;FinalValue;100;32
            items = line.split(';')
            node = items[0]
            num_compute = int(items[2])
            final_value = str(int(items[3]))

            self.node_finale_measures[node] = {
                'num_compute': num_compute,
                'value': final_value,
            }
            return

    def write_results(self, use_node_compute=True):
        """ Write all the experiment results """
        self.write_neighbours_graph()
        self.write_results_values(use_node_compute)
        self.write_results_final_value()

    def write_results_final_value(self):
        """ Write the 'final_value' result files if there is some data """
        if not len(self.node_finale_measures):
            return  # No final value

        all_name = '%s_final_all.csv' % self.outfilename
        all_measures = open(all_name, 'w')
        print "Write all final value to %s" % all_name

        # write data for each node
        for node, val_d in self.node_finale_measures.items():

            name = '%s_final_%s.csv' % (self.outfilename, node)
            print "Write final value to %s" % name

            # create the lines
            line = '%s' % (val_d['value'])
            all_line = '%s,%s' % (node, val_d['value'])

            # write datas
            with open(name, 'w') as measures:
                measures.write(line + '\n')
            all_measures.write(all_line + '\n')
        all_measures.close()


    def write_results_values(self, use_node_compute=True):
        """ Write the results to files """
        all_name = '%s_all.csv' % self.outfilename
        all_measures = open(all_name, 'w')
        print "Write all values to %s" % all_name

        for node, values in self.node_measures.items():
            name = '%s_%s.csv' % (self.outfilename, node)
            print "Write values to %s" % name
            measures = open(name, 'w')
            for i, val_d in enumerate(values):
                # use remote compute number or local compute number == num line
                compute_num = val_d['num_compute'] if use_node_compute else i

                # create the lines
                csv_vals = ','.join(val_d['values'])
                line = '%s,%s' % (compute_num, csv_vals)
                all_line = '%s,%s,%s' % (node, compute_num, csv_vals)

                measures.write(line + '\n')
                all_measures.write(all_line + '\n')

            measures.close()
        all_measures.close()

    def write_neighbours_graph(self):
        neighb_graph = self.neighbours_graph()
        out_dot = '%s_graph.dot' % self.outfilename
        with open(out_dot, 'w') as dot_f:
            dot_f.write(neighb_graph)
        print "Neighbours dot-graph written to %s" % out_dot

        out_png = '%s_graph.png' % self.outfilename
        cmd = ['dot', '-T', 'png', out_dot, '-o', out_png]
        subprocess.call(cmd)
        print "Neighbours graph written to %s" % out_png

    def neighbours_graph(self):
        links = self.neighbours_links()
        res = ''
        res += 'digraph G {\n'
        res += '    center=""\n'
        res += '    concentrate=true\n'
        res += '    graph [ color=black ]\n'
        for (node, neigh) in links['double']:
            res += '    "%s" -> "%s"[color=black]\n' % (node, neigh)
            res += '    "%s" -> "%s"[color=black]\n' % (neigh, node)
        for (node, neigh) in links['simple']:
            res += '    "%s" -> "%s"[color=red]\n' % (node, neigh)
        res += '}\n'
        return res

    def neighbours_links(self):
        simple_links = set()
        double_links = set()
        for node, neighbours in self.neighbours.items():
            # detect double and one sided links
            for neigh in neighbours:
                link = (node, neigh)
                rev_link = (neigh, node)
                # put the link in 'double_links' if reverse was already present
                if rev_link not in simple_links:
                    simple_links.add(link)
                else:
                    simple_links.remove(rev_link)
                    double_links.add(link)
        return {'simple': simple_links, 'double': double_links}

ALGOS = {
    'syncronous': algorithm_management.syncronous_mode,
    'gossip': algorithm_management.gossip_mode,
    'num_nodes': algorithm_management.find_num_node_gossip_mode,
}

def main():
    """ Reads nodes from ressource json in stdin and
    aggregate serial links of all nodes
    """
    parser = opts_parser()
    opts = parser.parse_args()

    try:
        username, password = iotlabcli.get_user_credentials(
            opts.username, opts.password)
        api = iotlabcli.Api(username, password)
        nodes_list = serial_aggregator.get_nodes(
            api, opts.experiment_id, opts.nodes_list, with_a8=True)
        print "Using algorith: %r" % opts.algo
        algorithm = ALGOS[opts.algo]
    except (ValueError, RuntimeError) as err:
        sys.stderr.write("%s\n" % err)
        exit(1)

    # Connect to the nodes
    results = NodeResults(opts.outfile)
    aggregator = serial_aggregator.NodeAggregator(
        nodes_list, print_lines=True, line_handler=results.handle_line)
    aggregator.start()
    time.sleep(2)

    # Run the algorithm
    try:
        algorithm(aggregator, opts.num_loop)
        time.sleep(3)
    finally:
        aggregator.stop()
        results.write_results(opts.algo == 'syncronous')



if __name__ == "__main__":
    main()
