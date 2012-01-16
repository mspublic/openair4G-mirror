#!/usr/bin/python
# -*- coding: utf-8 -*-

# The aim of this script is to collect some traces from oaisim stack and generate a sequence diagram image (png or jpeg).
# Actually it is a let's say a pre alpha release.
# an example of how it can be invoqued: msc_gen.py /tmp/msc_log.txt  | mscgen -T png -o /tmp/msc.png, msc_log.txt is the input text file,
# it is generated by the execution of the oaisim stack: oaisim -...(misc args) | grep MSC_ > msc_log.txt

import sys
import subprocess
import re
import socket
import datetime

SYSTEM_FRAME_NUMBER_STR  = 'FRAME'
MODULE_STR               = 'MOD'
RADIO_BEARER_STR         = 'RB'
MSC_NEW_STR              = '[MSC_NEW]'
MSC_MSG_STR              = '[MSC_MSG]'
MSC_NBOX_STR             = '[MSC_NBOX]'
MSC_BOX_STR              = '[MSC_BOX]'
MSC_ABOX_STR             = '[MSC_ABOX]'
MSC_RBOX_STR             = '[MSC_RBOX]'

g_entities_dic       = {}
g_entities         = []
g_messages         = []

#MAC and PHY valus have to be above num max radio bearer value
g_display_order_dic  = {'IP': 0, 'RRC_eNB': 1, 'RRC_UE': 1, 'PDCP': 32, 'RLC_AM': 33, 'RLC_UM': 33, 'RLC_TM': 33, 'MAC_eNB': 1024, 'MAC_UE': 1024, 'PHY': 2048}
g_display_color  = {'IP':      '\"#00ff00\"',
                    'RRC_UE':  '\"#008080\"',
                    'RRC_eNB': '\"#008080\"',
                    'PDCP':    '\"#0000ff\"',
                    'RLC_AM':  '\"#ffb000\"',
                    'RLC_UM':  '\"#ffb000\"',
                    'RLC_TM':  '\"#ffb000\"',
                    'MAC_UE':  '\"#808000\"',
                    'MAC_eNB': '\"#808000\"',
                    'PHY':     '\"#00ff00\"'}

#open TXT file that contain OAI filtered traces for mscgen
fhandle  = open(sys.argv[1], 'r')
fcontent = fhandle.read()
fhandle.close()

# split file content in lines
lines = fcontent.splitlines()
for line in lines:
    system_frame_number  = 'unknown'
    message              = 'unknown'

    module_id            = 'unknown'
    radio_bearer_id      = '-1'
    protocol_entity      = 'unknown'
    entity_name          = 'unknown'

    module_id_dest       = 'unknown'
    radio_bearer_id_dest = '-1'
    protocol_entity_dest = 'unknown'
    entity_name_dest     = 'unknown'

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
                    entity_name = item

            if radio_bearer_id == '':
                radio_bearer_id = '-1'

            if radio_bearer_id == '-1':
                protocol_entity = entity_name+'_'+str(int(module_id))
            else:
                protocol_entity = entity_name+'_'+str(int(module_id))+'_'+str(int(radio_bearer_id))

            #print ("\n\nprotocol_entity = %s " % protocol_entity)
            module_id_int = int(module_id)
            radio_bearer_id_int = int(radio_bearer_id)

            if module_id_int not in g_entities_dic:
                g_entities_dic[module_id_int] = {}
            if radio_bearer_id_int not in g_entities_dic[module_id_int]:
                g_entities_dic[module_id_int][radio_bearer_id_int] = []
            g_entities_dic[module_id_int][radio_bearer_id_int].append(entity_name)
            #print (" g_entities_dic[%d][%d].append(%s)" % (module_id_int, radio_bearer_id_int, entity_name))
            #print (" g_entities_dic= %s\n\n" % (g_entities_dic))

            protocol_entity_dic = {}
            protocol_entity_dic['name']            = entity_name
            protocol_entity_dic['module_id']       = module_id
            protocol_entity_dic['radio_bearer_id'] = radio_bearer_id
            protocol_entity_dic['creation_time']   = system_frame_number
            g_entities.append(protocol_entity)
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


            entity_name          = partition[2].strip().split(' ')[-1].strip(']')
            module_id            = partition[3].strip().split(' ')[-1].strip(']')
            radio_bearer_id      = partition[4].strip().split(' ')[-1].strip(']').strip()

            if radio_bearer_id == '':
                radio_bearer_id = '-1'

            if radio_bearer_id == '-1':
                protocol_entity = entity_name+'_'+str(int(module_id))
            else:
                protocol_entity = entity_name+'_'+str(int(module_id))+'_'+str(int(radio_bearer_id))


            message              = partition[5].strip().strip(']').strip('<').strip('>').strip('-')


            entity_name_dest     = partition[6].strip().split(' ')[-1].strip(']')
            module_id_dest       = partition[7].strip().split(' ')[-1].strip(']')
            radio_bearer_id_dest = partition[8].strip().split(' ')[-1].strip(']')

            if radio_bearer_id_dest == '':
                radio_bearer_id_dest = '-1'

            #print (" entity_name = %s module_id = %s" % (entity_name , module_id))
            if radio_bearer_id_dest == '-1':
                protocol_entity_dest = entity_name_dest+'_'+str(int(module_id_dest))
            else:
                protocol_entity_dest = entity_name_dest+'_'+str(int(module_id_dest))+'_'+str(int(radio_bearer_id_dest))

            message_dic = {}
            #print ('%s' % protocol_entity)
            message_dic['entity_src'] = protocol_entity
            message_dic['entity_dst'] = protocol_entity_dest
            message_dic['msg']        = message
            message_dic['line_color']      = g_display_color[entity_name]
            message_dic['text_color']      = g_display_color[entity_name]
            message_dic['time']       = system_frame_number
            #print ('%s' % message_dic)
            g_messages.append(message_dic)

    # if line is a trace of an information to be displayed for 1 or more protocol entities
    elif MSC_NBOX_STR in line :
        partition = line.strip().rpartition(MSC_NBOX_STR)
        msc_log_string = partition[2].strip()
        partition = msc_log_string.split('[')

        if len(partition) == 9:
            system_frame_number  = partition[1].strip().split(' ')[-1].strip(']')


            entity_name          = partition[2].strip().split(' ')[-1].strip(']')
            module_id            = partition[3].strip().split(' ')[-1].strip(']')
            radio_bearer_id      = partition[4].strip().split(' ')[-1].strip(']').strip()

            if radio_bearer_id == '':
                radio_bearer_id = '-1'

            if radio_bearer_id == '-1':
                protocol_entity = entity_name+'_'+str(int(module_id))
            else:
                protocol_entity = entity_name+'_'+str(int(module_id))+'_'+str(int(radio_bearer_id))


            box              = partition[5].strip().strip(']')

            entity_name_dest     = partition[6].strip().split(' ')[-1].strip(']')
            module_id_dest       = partition[7].strip().split(' ')[-1].strip(']')
            radio_bearer_id_dest = partition[8].strip().split(' ')[-1].strip(']')

            if radio_bearer_id_dest == '':
                radio_bearer_id_dest = '-1'

            if radio_bearer_id_dest == '-1':
                protocol_entity_dest = entity_name_dest+'_'+str(int(module_id_dest))
            else:
                protocol_entity_dest = entity_name_dest+'_'+str(int(module_id_dest))+'_'+str(int(radio_bearer_id_dest))


            message_dic = {}
            #print ('%s' % protocol_entity)
            message_dic['entity_src']  = protocol_entity
            message_dic['entity_dst']  = protocol_entity_dest
            message_dic['box']         = box
            message_dic['box_type']    = 'note'
            message_dic['text_color']  = '\"#000000\"'
            message_dic['textbg_color']  = g_display_color[entity_name]
            message_dic['time']        = system_frame_number
            #print ('%s' % message_dic)
            g_messages.append(message_dic)




print("msc {")
print("width = \"2048\";")

#print("------------------------------------")
#print ("  %s " % ( g_entities_dic ) )

final_display_order_list = []
for module_id_int in sorted(g_entities_dic.iterkeys()):
    module_display_order_dic = {}
    for radio_bearer_id_int in sorted(g_entities_dic[module_id_int].iterkeys()):
        for entity_name in g_entities_dic[module_id_int][radio_bearer_id_int]:
            #print (" entity_name =  %s module_id_int=%d" % ( entity_name,  module_id_int) )
            if radio_bearer_id_int < 0:
                module_display_order_dic[g_display_order_dic[entity_name]] = entity_name + '_' + str(module_id_int)
            else:
                module_display_order_dic[g_display_order_dic[entity_name]*(radio_bearer_id_int+1)] = entity_name + '_' + str(module_id_int) + '_' + str(radio_bearer_id_int)

    #print("------------------------------------")
    #print("  %s " % (module_display_order_dic))

    for display_priority in sorted(module_display_order_dic.iterkeys()):
        final_display_order_list.append(module_display_order_dic[display_priority])

#print("------------------------------------")
#print("  %s;" % (final_display_order_list))


entity_line_list_str = ''
for entity in final_display_order_list:
    entity_line_list_str = entity_line_list_str + ' ' + entity + ','

entity_line_list_str = entity_line_list_str.rstrip().strip(',')
print("  %s;" % (entity_line_list_str))


#for entity in g_entities:
#    print("  %s rbox %s [label=\"%s\"], " % (entity, entity, entity))

for message in g_messages:
    if 'msg' in message:
        print("  %s=>%s [ label = \"%s %s\", linecolour=%s , textcolour=%s ] ;" % (message['entity_src'], message['entity_dst'], message['time'], message['msg'], message['line_color'], message['text_color']))
    elif 'box' in message:
        print("  %s %s %s [ label = \"%s\", textbgcolour=%s , textcolour=%s ] ;" % (message['entity_src'], message['box_type'], message['entity_dst'], message['box'], message['textbg_color'], message['text_color']))
print("}")
