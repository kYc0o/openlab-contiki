#! /usr/bin/env python
# -*- coding: utf-8 -*-


""" Generate a C hash table that gives node number from uid """


import sys
import subprocess
import json
import textwrap
import logging
import os
CURRENT_DIRECTORY = os.path.dirname(os.path.realpath(__file__))
logging.basicConfig(level=logging.INFO, format="%(levelname)s : %(message)s")


LIB_FILE_NAME = "iotlab_uid_num_hashtable"
SOURCE_FILE_PATH = "%s/../" % CURRENT_DIRECTORY
CURRENT_FILE_REL_PATH = os.path.relpath(__file__, SOURCE_FILE_PATH)


def extract_json(json_str):
    """ Parse json input string to a python object
    >>> extract_json('{}')
    {}
    >>> extract_json('{ "a": 1, "b":[2]}')
    {u'a': 1, u'b': [2]}

    >>> import sys; sys.stderr = sys.stdout
    >>> extract_json('not json_string')
    Traceback (most recent call last):
    ...
    SystemExit: 1
    """
    try:
        answer_dict = json.loads(json_str)
    except ValueError:  # pragma: no cover
        sys.stderr.write('Could not load JSON object from input.\n')
        sys.exit(1)
    return answer_dict


def extract_nodes_uids(nodes_dict):
    """ Extract the experiment-cli nodes_dict and generate parsed uid dict """
    uids_dict = {}
    # format
    # {
    #      "at86rf231": {
    #             '<uid'>: [ <node_num>, node_num2]
    # ...

    for node_dict in nodes_dict["items"]:
        url = str(node_dict["network_address"])
        radio = node_dict["archi"].split(':')[1]
        uid = str(node_dict['uid']).lower()
        archi, num = url.split('.')[0].split('-')

        # uids stored per radio_type
        radio_dict = uids_dict.setdefault(radio, {})

        #
        # validate uids
        #

        if uid == 'unknown':
            logging.warning("UID: %s : %s", uid, url)
            continue  # invalid uid, unused node
        int_uid = int(uid, 16)
        # check values
        if int_uid >= 0xffff or int_uid <= 0:
            logging.error("UID: %s : %s", uid, url)
            continue
        # Only one node with the same uid
        if uid in radio_dict:
            logging.error("Non uniq uid: %r %r: original == %r",
                          uid, url, radio_dict[uid])
            logging.error("Dropping %r", url)
            continue

        #
        # Valid, add to dict
        #
        logging.debug("UID: %s : %s", uid, url)
        radio_dict[uid] = (archi, int(num))

    return uids_dict


def print_uids_and_nodes(nodes_uid_dict):
    """ Print the nodes_uid_dict """
    for radio, values in nodes_uid_dict.items():
        logging.info("Radio %s: len %u", radio, len(values))


def generate_table(nodes_uid_dict):
    """ Generate a hash table dict """
    archi_tables = {}
    for radio_type in nodes_uid_dict:
        radio_table = archi_tables.setdefault(radio_type, [])
        uids = nodes_uid_dict[radio_type]

        for uid, node in uids.iteritems():
            radio_table.append((uid, node))
        radio_table.sort(key=lambda x: int(x[0], 16))
    return archi_tables


def print_hash_table(radio_type, archi_tables):
    """ Print the uids hash table for radio_type """
    logging.debug("Table for %r nodes", radio_type)
    table = archi_tables[radio_type]
    for assoc in table:
        logging.debug(assoc)


def generate_a_c_hash_table(radio_type, archi_tables, lib_file_name):
    """ Generate a C hash table in SOURCE_FILE_PATH/ with
    'lib_file_name' .c and .h files """

    radio_table = archi_tables[radio_type]

    header_str = '''\
    /*
     * Generated from %s
     */

    #include <stdint.h>
    #include <stddef.h>

    #define CC1101 0x1
    #define CC2420 0x2
    #define M3 0x3
    #define A8 0x8

    struct node_entry {
        uint16_t uid;
        uint32_t node;
    };

    struct node {
        uint8_t type;
        uint32_t num;
    };

    extern const struct node_entry const nodes_uid_dict[%u];

    struct node node_from_uid(uint16_t uid);
    ''' % (CURRENT_FILE_REL_PATH, len(radio_table))

    with open(SOURCE_FILE_PATH + lib_file_name + '.h', 'w') as header:
        header.write(textwrap.dedent(header_str))

    body_str = textwrap.dedent('''\
    /*
     * Generated from %s
     */
    ''' % CURRENT_FILE_REL_PATH)

    body_str += '#include "%s.h"\n\n' % lib_file_name

    body_str += 'const struct node_entry const nodes_uid_dict[] = {\n'
    for uid, (archi, num) in radio_table:
        node_value = "%s << 24 | %u" % (archi.upper(), num)
        body_str += '    { 0x%s, %s },\n' % (uid, node_value)

    body_str += '};\n\n'
    with open(SOURCE_FILE_PATH + lib_file_name + '.c', 'w') as source:
        source.write(body_str)


def _main():  # pragma: no cover
    """ Parse nodes list and print a uid hash table """

    json_str = subprocess.check_output('experiment-cli info -l', shell=True)
    json_dict = extract_json(json_str)

    nodes_uid_dict = extract_nodes_uids(json_dict)
    print_uids_and_nodes(nodes_uid_dict)

    archi_tables = generate_table(nodes_uid_dict)

    print ''
    print_hash_table('at86rf231', archi_tables)

    generate_a_c_hash_table('at86rf231', archi_tables, LIB_FILE_NAME)


if __name__ == '__main__':  # pragma: no cover
    _main()
