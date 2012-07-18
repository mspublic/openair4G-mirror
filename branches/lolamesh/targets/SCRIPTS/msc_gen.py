#!/usr/bin/python
# -*- coding: utf-8 -*-

# The aim of this script is to collect some traces from oaisim stack and generate a sequence diagram image (png or jpeg).
# Actually it is a let's say a pre alpha release.
# an example of how it can be invoqued: msc_gen.py /tmp/msc_log.txt msc_log.txt is the input text file,
# it is generated by the execution of the oaisim stack: oaisim -...(misc args) | grep MSC_ > msc_log.txt
# Note L.G. to help: I put in script start_one_eNB_multi_UE_nas:
#          $OPENAIR_TARGETS/SIMU/USER/oaisim -a -C1 -u $1  | tee /tmp/oai_log.txt | grep MSC_ > /tmp/msc_log.txt

# Now, examples of a mscgen trace in oaisim look like this:
#       LOG_D(RRC, "[MSC_NEW][FRAME 00000][RRC_UE][MOD %02d][]\n", Mod_id+NB_eNB_INST); for declaring a new instance of a protocol
#
#       LOG_D(RLC, "[MSC_MSG][FRAME %05d][RLC_AM][MOD %02d][RB %02d][--- MAC_DATA_REQ/ %d TB(s) --->][MAC_%s][MOD %02d][]\n", ....)
#       for tracing a message between 2 protocol entities on the sequence diagram chart.
#
#       LOG_D(RRC, "[MSC_NBOX][FRAME %05d][RRC_UE][MOD %02d][][Tx RRCConnectionReconfigurationComplete to ENB index %d][RRC_UE][MOD %02d][]\n", ...)
#       for generating a notice box on the protocol entity sequence diagram chart.


import sys
import subprocess
import re
import socket
import datetime
import os.path
from datetime import date

MSCGEN_OUTPUT_TYPE       = "png"
MAX_MESSAGES_PER_PAGE    = 36

SYSTEM_FRAME_NUMBER_STR  = 'FRAME'
MODULE_STR               = 'MOD'
RADIO_BEARER_STR         = 'RB'
MSC_NEW_STR              = '[MSC_NEW]'
MSC_MSG_STR              = '[MSC_MSG]'
MSC_NBOX_STR             = '[MSC_NBOX]'
MSC_BOX_STR              = '[MSC_BOX]'
MSC_ABOX_STR             = '[MSC_ABOX]'
MSC_RBOX_STR             = '[MSC_RBOX]'

# This dic is filled as follow : g_entities_dic[module_id_int][radio_bearer_id_int].append(entity_name)
# where entity name can be any of the following IP, RRC_eNB, RRC_UE, PDCP, RLC_AM, RLC_UM, RLC_TM, MAC_eNB, MAC_UE, PHY
g_entities_dic       = {}

# This list is filled as follow : g_entities.append(protocol_entity)
# where protocol_entity is made with a entity name (IP, RRC_eNB, RRC_UE, PDCP, RLC_AM, ...) and '_'`module identifier` and
# optionnaly '_'`radio bearer identifier` or any other identifier(s)
# examples: RLC_AM_02_03, RRC_eNB_00
g_entities         = []

# g_messages is filled with dictionnaries:
# message_dic['entity_src'] = protocol_entity_src
# message_dic['entity_dst'] = protocol_entity_dest
# message_dic['msg']        = message
# message_dic['line_color'] = g_display_color[entity_name]
# message_dic['text_color'] = g_display_color[entity_name]
# message_dic['time']       = system_frame_number
# message_dic['seq_no']     = sequence_number_generator()
# g_messages.append(message_dic)
g_messages         = []

# g_display_order_dic is a dictionnary where the order of the display of entities sharing the same module identifier is hardcoded
#MAC and PHY valus have to be above num max radio bearer value
g_display_order_dic  = {'IP': 0, 'RRC_eNB': 1, 'RRC_UE': 1, 'PDCP': 32, 'RLC_AM': 33, 'RLC_UM': 33, 'RLC_TM': 33, 'MAC_eNB': 1024, 'MAC_UE': 1024, 'PHY': 2048}

# Display color of messages of sending entities
g_display_color  = {'IP':      '\"teal\"',
                    'RRC_UE':  '\"red\"',
                    'RRC_eNB': '\"red\"',
                    'PDCP':    '\"blue\"',
                    'RLC_AM':  '\"navy\"',
                    'RLC_UM':  '\"navy\"',
                    'RLC_TM':  '\"navy\"',
                    'MAC_UE':  '\"indigo\"',
                    'MAC_eNB': '\"indigo\"',
                    'PHY':     '\"purple\"'}


g_final_display_order_list = []

g_sequence_generator = 0

def sequence_number_generator():
    global g_sequence_generator
    l_seq = g_sequence_generator
    g_sequence_generator = g_sequence_generator + 1
    return l_seq


# The aim of this procedure is to find the entity_type_name for the parsing part of code treating 'messages' or 'boxes'
# from whatever is the name of the protocol entity (RLC, RLC_AM, RLC_UM_03_02) all string identifying this protocol entity
# are returned.
# This has been written because for example when the RRC send a message to a RLC entity, it may not know if it is for a UM, AM, or TM
def getEntityNames( entity_name = 'unknown',  module_id = -1, other_id = -1):
    global g_entities_dic
    #print ("## getEntityNames( entity_name %s module_id %d other_id %d)" % (entity_name , module_id, other_id))

    entity_main_name  = entity_name.partition('_')[0]

    for entity_name in g_entities_dic[module_id][other_id].iterkeys():
        if entity_name.count(entity_main_name) > 0:
            return g_entities_dic[module_id][other_id][entity_name]

    print ("## WARNING  getEntityNames( entity_name %s module_id %d other_id %d)  UNKNOWN" % (entity_name , module_id, other_id))

    return {}


def is_entity_declared( entity_name = 'unknown',  module_id = -1, other_id = -1):
   return 0


def parse_oai_log_file():
    global g_entities_dic
    global g_entities
    global g_messages
    global g_final_display_order_list
    #open TXT file that contain OAI filtered traces for mscgen
    fhandle  = open(sys.argv[1], 'r')
    fcontent = fhandle.read()
    fhandle.close()

    # split file content in lines
    lines = fcontent.splitlines()
    for line in lines:
        system_frame_number   = 'unknown'
        message               = 'unknown'

        module_id             = 'unknown'
        radio_bearer_id       = '-1'
        protocol_entity_src   = 'unknown'
        entity_name_src       = 'unknown'
        entity_name_main_src  = 'unknown'
        entity_name_type_src  = 'unknown'
        entity_tuple_name_src = 'unknown'

        module_id_dest        = 'unknown'
        radio_bearer_id_dest  = '-1'
        protocol_entity_dest  = 'unknown'
        entity_name_dest      = 'unknown'
        entity_name_main_dest = 'unknown'
        entity_name_type_dest = 'unknown'
        entity_tuple_name_dest= 'unknown'

        # if line is a trace of the creation of a new protocol instance
        if MSC_NEW_STR in line:
            partition = line.rpartition(MSC_NEW_STR)
            msc_log_string = partition[2]
            #print (" %s " % msc_log_string)
            partition = msc_log_string.split('[')
            #print ("\n\n %s \n" % partition)
            if len(partition) == 5:

                for item in partition:
                    item = item.strip()
                    item = item.strip(']')

                    if SYSTEM_FRAME_NUMBER_STR in item:
                        system_frame_number = item.split(' ')[-1]

                    elif MODULE_STR in item:
                        module_id = item.split(' ')[-1]

                    elif RADIO_BEARER_STR in item:
                        radio_bearer_id = item.split(' ')[-1]

                    elif item != '':
                        entity_name_src      = item
                        entity_partition = item.partition('_')
                        entity_name_main_src = entity_partition[0]
                        entity_name_type_src = entity_partition[2]

                if radio_bearer_id == '':
                    radio_bearer_id = '-1'

                if radio_bearer_id == '-1':
                    protocol_entity_src = entity_name_src+'_'+str(int(module_id))
                else:
                    protocol_entity_src = entity_name_src+'_'+str(int(module_id))+'_'+str(int(radio_bearer_id))

                #print ("\n\nprotocol_entity = %s " % protocol_entity)
                module_id_int = int(module_id)
                radio_bearer_id_int = int(radio_bearer_id)

                if module_id_int not in g_entities_dic:
                    g_entities_dic[module_id_int] = {}
                if radio_bearer_id_int not in g_entities_dic[module_id_int]:
                    g_entities_dic[module_id_int][radio_bearer_id_int] = {}

                protocol_entity_dic = {}
                protocol_entity_dic['full_name']       = protocol_entity_src
                protocol_entity_dic['name_main']       = entity_name_main_src
                protocol_entity_dic['name_type']       = entity_name_type_src
                protocol_entity_dic['creation_time']   = system_frame_number
                g_entities_dic[module_id_int][radio_bearer_id_int][protocol_entity_src] = protocol_entity_dic
                #print (" g_entities_dic[%d][%d].append(%s)" % (module_id_int, radio_bearer_id_int, entity_name_src))
                #print (" g_entities_dic= %s\n\n" % (g_entities_dic))

                g_entities.append(protocol_entity_src)
                #print (" %s \n" % protocol_entity)


        # if line is a trace of a message between 2 protocol entities or layers
        elif MSC_MSG_STR in line:
            partition = line.strip().rpartition(MSC_MSG_STR)
            msc_log_string = partition[2].strip()
            #print (" %s " % msc_log_string)
            partition = msc_log_string.split('[')
            #print (" %s " % partition)

            if len(partition) == 9:
                system_frame_number  = partition[1].strip().split(' ')[-1].strip(']')
                module_id            = partition[3].strip().split(' ')[-1].strip(']')
                radio_bearer_id      = partition[4].strip().split(' ')[-1].strip(']').strip()

                if radio_bearer_id == '':
                    radio_bearer_id = '-1'


                entity_name_src      = partition[2].strip().split(' ')[-1].strip(']')
                entity_dic = getEntityNames(entity_name_src, int(module_id), int(radio_bearer_id))
                protocol_entity_src  = entity_dic['full_name']
                entity_name_main_src = entity_dic['name_main']
                entity_name_type_src = entity_dic['name_type']

                if entity_name_type_src != '':
                    entity_tuple_name_src = entity_name_main_src + '_' + entity_name_type_src
                else:
                    entity_tuple_name_src = entity_name_main_src


                message              = partition[5].strip().strip(']').strip('<').strip('>').strip('-')


                module_id_dest        = partition[7].strip().split(' ')[-1].strip(']')
                radio_bearer_id_dest  = partition[8].strip().split(' ')[-1].strip(']')

                if radio_bearer_id_dest == '':
                    radio_bearer_id_dest = '-1'

                entity_name_dest      = partition[6].strip().split(' ')[-1].strip(']')
                entity_dic = getEntityNames(entity_name_dest, int(module_id_dest), int(radio_bearer_id_dest))
                protocol_entity_dest  = entity_dic['full_name']
                entity_name_main_dest = entity_dic['name_main']
                entity_name_type_dest = entity_dic['name_type']

                message_dic = {}
                #print ('%s' % protocol_entity)
                message_dic['entity_src']      = protocol_entity_src
                message_dic['entity_dst']      = protocol_entity_dest
                message_dic['msg']             = message
                message_dic['line_color']      = g_display_color[entity_tuple_name_src]
                message_dic['text_color']      = g_display_color[entity_tuple_name_src]
                message_dic['time']            = system_frame_number
                message_dic['seq_no']          = sequence_number_generator()
                #print ('%s' % message_dic)
                g_messages.append(message_dic)

        # if line is a trace of an information to be displayed for 1 or more protocol entities
        elif MSC_NBOX_STR in line :
            partition = line.strip().rpartition(MSC_NBOX_STR)
            msc_log_string = partition[2].strip()
            partition = msc_log_string.split('[')

            if len(partition) == 9:
                system_frame_number  = partition[1].strip().split(' ')[-1].strip(']')


                module_id            = partition[3].strip().split(' ')[-1].strip(']')
                radio_bearer_id      = partition[4].strip().split(' ')[-1].strip(']').strip()

                if radio_bearer_id == '':
                    radio_bearer_id = '-1'

                entity_name_src      = partition[2].strip().split(' ')[-1].strip(']')
                entity_dic           = getEntityNames(entity_name_src, int(module_id), int(radio_bearer_id))
                protocol_entity_src  = entity_dic['full_name']
                entity_name_main_src = entity_dic['name_main']
                entity_name_type_src = entity_dic['name_type']

                if entity_name_type_src != '':
                    entity_tuple_name_src = entity_name_main_src + '_' + entity_name_type_src
                else:
                    entity_tuple_name_src = entity_name_main_src


                box                   = partition[5].strip().strip(']')

                module_id_dest        = partition[7].strip().split(' ')[-1].strip(']')
                radio_bearer_id_dest  = partition[8].strip().split(' ')[-1].strip(']')
                if radio_bearer_id_dest == '':
                    radio_bearer_id_dest = '-1'

                entity_name_dest      = partition[6].strip().split(' ')[-1].strip(']')
                entity_dic            = getEntityNames(entity_name_dest, int(module_id_dest), int(radio_bearer_id_dest))
                protocol_entity_dest  = entity_dic['full_name']
                entity_name_main_dest = entity_dic['name_main']
                entity_name_type_dest = entity_dic['name_type']


                message_dic = {}
                #print ('%s' % protocol_entity)
                message_dic['entity_src']       = protocol_entity_src
                message_dic['entity_dst']       = protocol_entity_dest
                message_dic['box']              = box
                message_dic['box_type']         = 'note'
                message_dic['text_color']       = '\"#000000\"'
                message_dic['textbg_color']     = g_display_color[entity_tuple_name_src]
                message_dic['time']             = system_frame_number
                message_dic['seq_no']           = sequence_number_generator()
                #print ('%s' % message_dic)
                g_messages.append(message_dic)
    #print("------------------------------------")
    #print ("  %s " % ( g_entities_dic ) )

    for module_id_int in sorted(g_entities_dic.iterkeys()):
        module_display_order_dic = {}
        for radio_bearer_id_int in sorted(g_entities_dic[module_id_int].iterkeys()):
            for entity_name in g_entities_dic[module_id_int][radio_bearer_id_int]:
                entity_main_name = g_entities_dic[module_id_int][radio_bearer_id_int][entity_name]['name_main']
                entity_type_name = g_entities_dic[module_id_int][radio_bearer_id_int][entity_name]['name_type']
                if entity_type_name != '':
                    entity_tuple_name = entity_main_name + '_' + entity_type_name
                else:
                    entity_tuple_name = entity_main_name
                #print (" entity_name =  %s entity_main_name =  %s module_id_int=%d" % ( entity_name,  entity_main_name, module_id_int) )
                if radio_bearer_id_int < 0:
                    module_display_order_dic[g_display_order_dic[entity_tuple_name]] = entity_name
                else:
                    module_display_order_dic[g_display_order_dic[entity_tuple_name]*(radio_bearer_id_int+1)] = entity_name

        #print("------------------------------------")
        #print("  %s " % (module_display_order_dic))

        for display_priority in sorted(module_display_order_dic.iterkeys()):
            g_final_display_order_list.append(module_display_order_dic[display_priority])

    #print("------------------------------------")
    #print("  %s;" % (g_final_display_order_list))


def msc_chart_write_header(fileP):
    global g_final_display_order_list
    fileP.write("msc {")
    fileP.write("width = \"2048\";")

    entity_line_list_str = ''
    for entity in g_final_display_order_list:
        entity_line_list_str = entity_line_list_str + ' ' + entity + ','

    entity_line_list_str = entity_line_list_str.rstrip().strip(',')
    fileP.write("  %s;" % (entity_line_list_str))


def msc_chart_write_footer(fileP):
    fileP.write("}")

def msc_chart_generate(file_nameP):
    global  MSCGEN_OUTPUT_TYPE
    command_line = "mscgen -T " + MSCGEN_OUTPUT_TYPE + " -i " + file_nameP
    fi,fo,fe=os.popen3(command_line)
    for i in fe.readlines():
        print "error:",i

def get_nem_file_descriptor():
    global g_base_file_name
    global g_page_index
    l_file_name = g_base_file_name + str(g_page_index)+'.txt'
    l_file = open(l_file_name, "wb")
    return l_file


###### MAIN STAR HERE #################
parse_oai_log_file()

g_page_index    = 0
g_message_index = 0
g_now = datetime.datetime.now()
g_now_formated = g_now.strftime("%Y-%m-%d_%H.%M.%S")
g_currentdir = os.curdir
g_resultdir = os.path.join(g_currentdir, g_now_formated)
os.mkdir(g_resultdir)
os.chdir(g_resultdir)

g_base_file_name = 'oai_mscgen_page_'

g_file = get_nem_file_descriptor()
msc_chart_write_header(g_file)

for message in g_messages:

    if 'msg' in message:
        g_file.write("  %s=>%s [ label = \"(%d) Frm%s %s\", linecolour=%s , textcolour=%s ] ;" % (message['entity_src'], message['entity_dst'], message['seq_no'], message['time'], message['msg'], message['line_color'], message['text_color']))
    elif 'box' in message:
        g_file.write("  %s %s %s [ label = \"%s\", textbgcolour=%s , textcolour=%s ] ;" % (message['entity_src'], message['box_type'], message['entity_dst'], message['box'], message['textbg_color'], message['text_color']))

    g_message_index = g_message_index + 1

    if ((g_message_index % MAX_MESSAGES_PER_PAGE) == 0):
        msc_chart_write_footer(g_file)
        g_file.close()
        msc_chart_generate(g_file.name)
        g_page_index = g_page_index + 1

        g_file = get_nem_file_descriptor()
        msc_chart_write_header(g_file)


msc_chart_write_footer(g_file)
g_file.close()
msc_chart_generate(g_file.name)
