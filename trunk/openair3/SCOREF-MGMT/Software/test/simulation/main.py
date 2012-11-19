import sys
import signal
import time
from packet import * # Packet generator for MGMT -> CLIENT packets
from responder import * # Responder for CLIENT -> MGMT packets

# Handle SIGINT to exit nicely
def signal_handler(signal, frame):
	print 'CTRL+C caught, exiting...'
	sys.exit(0)
signal.signal(signal.SIGINT, signal_handler)

# Check command-line parameters
if len(sys.argv) == 1:
	print "Usage: ", sys.argv[0], " <scenario-file>"
	sys.exit()
else:
	print "Running scenario file: ", sys.argv[1]

scenarioFile = open(sys.argv[1])

# State checks
startLineRead = False
endLineRead = False
# Server information
serverPort = 1402
serverAddress = "127.0.0.1"
# Client information
clientType = ""
clientPacketType = ""
clientPort = 8000
# If incoming packets are going to be replied with 
# corresponding response packets or simply ignored
# False: Ignore, True: Process and Reply
clientReply = False

# Traverse scenario file line by line
line = scenarioFile.readline()
line = line.strip('\n')

while line:
	commands = line.split()
	print "DEBUG = ", commands
	# Every scenario file has to start with "START" command
	if commands[0] == "START":
		startLineRead = True

	# We are asked to SEND a packet
	elif commands[0] == "SEND":
		clientPacketType = commands[1]
		print clientPacketType, "packet is going to be sent"
		# Configuration Request Packet
		if clientPacketType == "GET_CONFIGURATION":
			if Packet.sendConfigurationRequest(serverAddress, serverPort, clientPort):
				print "CONFIGURATION_REQUEST packet sent successfully"
			else:
				print "ERROR: Cannot send CONFIGURATION_REQUEST"
		# Communication Profile Request Packet
		if clientPacketType == "COMMUNICATION_PROFILE_REQUEST":
			if Packet.sendCommunicationProfileRequest(serverAddress, serverPort, clientPort):
				print "COMMUNICATION_PROFILE_REQUEST packet sent successfully"
			else:
				print "ERROR: Cannot send COMMUNICATION_PROFILE_REQUEST"
		# Network State Packet
		elif clientPacketType == "NETWORK_STATE":
			if Packet.sendNetworkState(serverAddress, serverPort, clientPort):
				print "NETWORK_STATE packet sent successfully"
			else:
				print "ERROR: Cannot send NETWORK_STATE"
		# Location Update
		elif clientPacketType == "LOCATION_UPDATE":
			if Packet.sendLocationUpdate(serverAddress, serverPort, clientPort):
				print "LOCATION_UPDATE packet sent successfully"
			else:
				print "ERROR: Cannot send LOCATION_UPDATE"

	# Wait command
	elif commands[0] == "WAIT":
		howManySeconds = int(commands[1])
		print "Waiting for", howManySeconds, "seconds"
		time.sleep(howManySeconds)

	# Server port is being defined
	elif commands[0] == "DEFINE_SERVER_PORT":
		serverPort = int(commands[1])
		print "Server port defined as", serverPort

	# Client port is being defined
	elif commands[0] == "DEFINE_CLIENT_PORT":
		clientPort = int(commands[1])
		print "Client port defined as", clientPort

	# Server address is being defined
	elif commands[0] == "DEFINE_ADDRESS":
		serverAddress = commands[1]
		print "Server address defined as", serverAddress

	# Client type is being defined
	elif commands[0] == "DEFINE_TYPE":
		clientType = commands[1]
		print "Client type defined as", clientType

	# Are incoming packets going to be responded
	elif commands[0] == "DEFINE_REPLY":
		clientReply = True if commands[1] == "TRUE" else False
		if clientReply:
			print "Those incoming messages that require a response will be handled and a response will be sent"
			responder = Responder()
# TODO			responder.start()
		else:
			print "Those incoming messages that require a response will be ignored"

	# Scenario ends with an END command
	elif commands[0] == "END":
		if not startLineRead:
			print "Syntax error: No START command given"
			sys.exit()
		endLineRead = True

	# Read a new line and remove newline
	line = scenarioFile.readline()
	line = line.strip('\n')

sys.exit()
