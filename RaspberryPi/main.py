# Imports
from enum import Enum
from time import sleep
import serial

interface = serial.Serial("/dev/ttyAMA0", 115200, timeout=1 )
interface.open()

##
##
##  Command Section
##
##

#
#   This means our packet looks like:
#   2 Byte Header, 1 Byte CMD1, 1 byte CMD2, 1 byte Length, 0-N Byte Data
#   With no Parity or checksum.
#

# These are the group 1 commands
class command1CMD(Enum):
	GroupSystemCMD = 0,
	GroupSystemReply = 1,
	GroupControl = 2,
	GroupPreset = 4,
	GroupStatus = 6,
	GroupStatusReply = 7,
	GroupFactory = 0x0F

# These are the group 2 commands (so the subcommands)
class command2CMD(Enum):
	SystemACK = 0x10,
	SystemDeviceType = 0x11,
	SystemNAK = 0x12,
	SystemManufacturer = 0x13,
	SystemProductName = 0x14,
	SystemSerialNumb = 0x15,

	SystemFirmware = 0x20,
	SystemHardware = 0x21,

	CortexDeviceID = 0x00,
	GlobalDeviceID = 0x0f,

	# These are the same as above but are the command 1 (level 2) varients
	SetAllMotors = 0x10,
	SetMotorIndex = 0x11,

	# Level 6 Varient
	getMotorStatus = 0x10

# This class controls the command/packet wrapper
class command():
	def __init__(self, cmd1 = 0, cmd2 = 0, length = 0, data = [], PREAMBLE1 = 0x50, PREAMBLE2 = 0xAF):
		self.cmd1 = cmd1
		self.cmd2 = cmd2
		self.length = length
		self.data = data
		self.PREAMBLE2 = PREAMBLE2
		self.PREAMBLE1 = PREAMBLE1

	def setCMD1(self, packetChar):
		self.cmd1 = packetChar
	def setCMD2(self, packetChar):
		self.cmd2 = packetChar

	def getCommands(self):
		return self.cmd1, self.cmd2

	def setData(self, packetDataString):
		self.data  = packetDataString
	def addData(self, data):
			self.data.append(data)
	def getData(self):
			return self.data

	def send(self, serialController):
		# Create the packet that we are going to send
		if self.length == 0:
				self.length = len(self.data)
		packet = [self.PREAMBLE1, self.PREAMBLE2, self.cmd1, self.cmd2, self.length]
		packet.extend(self.data)
		return serialController.sendPacket(packet)

##
##
##  Prebuild Commands
##
##

# These are the system responses
CMD_ACKReply                =   command(command1CMD.GroupSystemReply, command2CMD.SystemACK, 0x01, [0x00])
NAKReply_Timeout            =   command(command1CMD.GroupSystemReply, command2CMD.SystemNAK, 0x01, [0x04])
NAKReply_Und                =   command(command1CMD.GroupSystemReply, command2CMD.SystemNAK, 0x01, [0x01])									# Error for undefined command
NAKReply_Mal				=	command(command1CMD.GroupSystemReply, command2CMD.SystemNAK, 0x01, [0x03])									# Malformed Packet

CMD_DevTypeReply            =   command(command1CMD.GroupSystemReply, command2CMD.SystemDeviceType, 0x02, [0x22, 0xC0])
CMD_ManufacturerReply       =   command(command1CMD.GroupSystemReply, command2CMD.SystemManufacturer, 0x01, [0x03])         # 3 to represent rounded pi
CMD_ProductNameReply        =   command(command1CMD.GroupSystemReply, command2CMD.SystemProductName, 0x02, [0x22, 0xC0])    
CMD_SerialNumberReply       =   command(command1CMD.GroupSystemReply, command2CMD.SystemSerialNumb, 0x02, [0x00, 0x03])     # Serial 03
CMD_FirmwareVersionReply    =   command(command1CMD.GroupSystemReply, command2CMD.SystemFirmware, 0x01, [0x01])             # Firmware 1
CMD_HardwareVersionReply    =   command(command1CMD.GroupSystemReply, command2CMD.SystemHardware, 0x02, [0x02, 0x02])       # 2B (Pi Ver 2 Rev B)

# These are standard system commands
CMD_DevTypeRequest          =   command(command1CMD.GroupSystemCMD, command2CMD.SystemDeviceType)
CMD_ManufacturerRequest     =   command(command1CMD.GroupSystemCMD, command2CMD.SystemManufacturer)
CMD_ProductNameRequest      =   command(command1CMD.GroupSystemCMD, command2CMD.SystemProductName)
CMD_SerialNumberRequest     =   command(command1CMD.GroupSystemCMD, command2CMD.SystemSerialNumb)
CMD_FirmwareVersionRequest  =   command(command1CMD.GroupSystemCMD, command2CMD.SystemFirmware)
CMD_HardwareVersionRequest  =   command(command1CMD.GroupSystemCMD, command2CMD.SystemHardware)

# These are cortex specific commands
# The response should be ACK
CMD_SetAllMotors            =   command(command1CMD.GroupControl, command2CMD.SetAllMotors, 0x02, [0x00, 0x00]) # 2 Bytes, one 0x00, 1 byte of data
CMD_SetMotorByIndex         =   command(command1CMD.GroupControl, command2CMD.SetMotorIndex, 0x02, [0x00, 0x00]) # 2 Bytes, 1 byte of index, one byte of speed
CMD_GetMotorStatus          =   command(command1CMD.GroupStatus, command2CMD.getMotorStatus, 0x01, [0x00]) # 1 Byte for index

##
##
##  Serial Interface and command implementations
##
##

# The idea here is that there is a controller who governs over the
# input and output packets (a sort of very simple software UART)
class serialController():
	def __init__(self, outputSerial):
		self.outputSerial = outputSerial
		self.BufferUsed = False

		# Will halt while buffer is being used
		while(self.OutputBufferUsed == True):
			sleep(2.0/1000)    # Wait 2Msec
			
		# Lock the buffer untill we are finished
		self.OutputBufferUsed = True

		for char in inputPacketList:
			outputSerial.send(char)

		# Wait a couple of MS just to make sure there is some spacing between sends
		sleep(2/1000)

		# Make sure to unlock the buffer so other programs can use it
		BufferUsed = False

		return True

	def getPacket(self, outputPacket):
		# Will halt while buffer is being used
		while(self.BufferUsed == True):
			sleep(2.0/1000)    # Wait 2Msec
			
		# Lock the buffer untill we are finished
		self.BufferUsed = True
		# We know we will always have a constant 2 bits of header
		softStorage = outputSerial.read(2)
		if softStorage[0] != 0x50 or softStorage[1] != 0xAF:
			NAKReply_Und.send(self.outputSerial)
			return False
			# We assume that the new two bits will be correct and we dont have to garbage collect
		
		# Another 3 bits of header info (cmd1, cmd2, length), not super important we keep the header
		softStorage = outputSerial.read(3)
		outputPacket.setCMD1(softStorage[0])
		outputPacket.setCMD2(softStorage[1])
		
		# Grab the next Length of data
		storageLength = softStorage[2]
		softStorage = serial.read(storageLength)
		for dataVar in softStorage:
			outputPacket.addData(dataVar )

		# Make sure to unlock the buffer so other programs can use it
		BufferUsed = False

		return True

	def appendIncomeStack(self, newPacketRefrence):
		self.IncomeStack.insert(0, newPacketRefrence)

	# In order for this to work you need to wait 10MS after your send operation
	def getLatestPacket(self):
		sleep(10.0/1000)
		try:
			returnValue = self.IncomeStack[0]
		except IndexError:
			# We know there is no value so we call a timeout and return a NAK packet
			NAKReply_Timeout.send(outputController)
			return NAKReply_Timeout
		del self.IncomeStack[0]
		return returnValue

	def respondToSystemCMDReq(self, inputClass):
		if self.cmd2 == command2CMD.SystemACK:
			return True, [0]
		# This shouldn't happen :/
		elif self.cmd2 == command2CMD.SystemNAK:
			NAKReply_Und.send(outputController)
			return SendFailure, inputClass.data

		# The rest of these are basicially switch statements for what could be asked.
		if self.cmd2 == command2CMD.SystemDeviceType:
			CMD_DevTypeReply.send(outputController)
			return True, [0]
		if self.cmd2 == command2CMD.SystemManufacturer:
			CMD_ManufacturerReply.send(outputController)
			return True, [0]
		if self.cmd2 == command2CMD.SystemProductName:
			CMD_ProductNameReply.send(outputController)
			return True, [0]
		if self.cmd2 == command2CMD.SystemSerialNumb:
			CMD_SerialNumberReply.send(outputController)
			return True, [0]
		if self.cmd2 == command2CMD.SystemFirmware:
			CMD_FirmwareVersionReply.send(outputController)
			return True, [0]
		if self.cmd2 == command2CMD.SystemHardware:
			CMD_HardwareVersionReply.send(outputController)
			return True, [0]
		else:
			NAKReply_Und.send(outputController)
			return False, [0]

inputController = serialController(interface)
outputController = serialController(interface)

# For these we expect an ACK back (or NAK if it was damaged)
def setAllCortexMotors(serialController, speed):
	setMotorPowerAll = CMD_SetAllMotors
	setMotorPowerAll.setData([speed + 127]) # We add 127 to make the range an unsigned 0-254
	setMotorPowerAll.send(outputController)
	responsePacket = inputController.getLatestPacket()
	cmd1, cmd2 = responsePacket.getCommands()
	return cmd2

def setCortexMotorByIndex(interface, index, speed):
	workingClass = setMotorByIndex
	workingClass.setData = [index, (speed+127)]
	workingClass.send(outputController)    
	responsePacket = inputController.getLatestPacket()
	cmd1, cmd2 = responsePacket.getCommands()
	return cmd2

# For these we expect a True with a collection of data as a reponse
# As a repsonse we expect 6, ACK, length of 1, data = unsigned int response
def getMotorStatusByIndex(interface, index):
	workingClass = CMD_GetMotorStatus
	workingClass.setData = [index]
	workingClass.send(outputController)
	responsePacket = inputController.getLatestPacket()
	dataResponse = responsePacket.getData()
	return dataResponse[0] # One byte response (unsigned int)

##
##
##  Main loop and controller
##
##

# Checks if the serial interface is actually Open
if interface.isOpen:

	while True:

		## This is the input Controller

		workingClass = command()
		inputController.getPacket(workingClass)

		cmd1, cmd2 = workingClass.getCommands()
		if cmd1 == command1CMD.GroupSystemCMD:
			# We deal with these automaticially and they are not added
			# to the income stack.
			inputController.respondToSystemCMDReq(workingClass)
		else:
			# This means that the incoming packet is user built and 
			# can be delt with by users.
			inputController.appendIncomeStack(workingClass)

else:
	raise SerialException
