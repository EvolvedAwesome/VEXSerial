// These are the commands used by the program to do things.
// They are static structures being defined by an init function.
// static structName commandName = commandInitiationFunc( CommandGroup1, CommandGroup2, Data)
// See the init command functions for more info.

// Holds refrence to a struct containing packet info.
// This is used to easily hold refrences so the user programs can deal with
// and use these packets quickly without having to decode and grab them themselves.
static command *incomeStack[incomeStackMaxLength];

// These are the system responses
static command CMD_ACKReply                =   commandInit(GroupSystemReply, SystemACK);
static command NAKReply_Timeout            =   commandInit(GroupSystemReply, SystemNAK, 0x04);
static command NAKReply_Und                =   commandInit(GroupSystemReply, SystemNAK, 0x01);                  // Error for undefined command
static command NAKReply_Mal                =   commandInit(GroupSystemReply, SystemNAK, 0x03);                  // Malformed Packet

static command CMD_DevTypeReply            =   commandInit(GroupSystemReply, SystemDeviceType, 0x22, 0xC0);
static command CMD_ManufacturerReply       =   commandInit(GroupSystemReply, SystemManufacturer, 0x03);
static command CMD_ProductNameReply        =   commandInit(GroupSystemReply, SystemProductName, 0x22, 0xC0);
static command CMD_SerialNumberReply       =   commandInit(GroupSystemReply, SystemSerialNumb, 0x00, 0x03);
static command CMD_FirmwareVersionReply    =   commandInit(GroupSystemReply, SystemFirmware, 0x01);
static command CMD_HardwareVersionReply    =   commandInit(GroupSystemReply, SystemHardware, 0x02, 0x02);

// These are standard system commands
static command CMD_DevTypeRequest          =   commandInit(GroupSystemCMD, SystemDeviceType);
static command CMD_ManufacturerRequest     =   commandInit(GroupSystemCMD, SystemManufacturer);
static command CMD_ProductNameRequest      =   commandInit(GroupSystemCMD, SystemProductName);
static command CMD_SerialNumberRequest     =   commandInit(GroupSystemCMD, SystemSerialNumb);
static command CMD_FirmwareVersionRequest  =   commandInit(GroupSystemCMD, SystemFirmware);
static command CMD_HardwareVersionRequest  =   commandInit(GroupSystemCMD, SystemHardware);

static command CMD_GetMotorStatusReply     =   commandInit(GroupStatusReply, SystemACK, 0x00); // One byte of data holding the retrun value.

#define CMD_SetAllMotorsReply CMD_ACKReply
#define CMD_SetMotorByIndexReply CMD_ACKReply

// These are the commands for directly interacting with the commands.
void setCMD1(command *packet, char cmd1Value) {
	packet->cmd1 = cmd1Value;
}
void setCMD2(command *packet, char cmd2Value) {
	packet->cmd2 = cmd2Value;
}

char getCMD1(command *packet) {
	return packet->cmd1;
}
char getCMD2(command *packet) {
	return packet->cmd2;
}

// This func just sets input packets data to the data supplied and
// sets the length appropriatly.
void setData(command *packet, char *arrayP) {
	packet->length = sizeof(arrayP)/1;

	// Throws an error if I don't cast this pointer :(
	char arrayI = arrayP;
	packet->data = arrayI;

	// If there is no data, make the length 0 and dont send frivilous bytes
	if(packet->data[0] == 0x00 && packet->length == 1) {
		packet->length = 0;
	}
}

// Function overload that allows adding a single element of data.
void setData(command *packet, char element) {
	packet->length = 1;
	packet->data[0] = element;
}

// This function just initiates all the data (or rather,
// just says theres no data so that it can be easily written over
// when it needs to be accessed). (Originially a function overload for setData)
void initData(command *packet) {
	packet->length = 0;
}

// This one adds data onto the the already existing array
// This means we can add or make changes to data live.
void addData(command *packet, char *arrayP) {
	// This is the length of the data already in the array.
	// This means if there are 5 bytes of data, data[0-4] are populated,
	// so we start adding new data on data[5-n].
	int startingLength = packet->length;

	// This calculates the sizeof the input array to give us n (so
	// we can put it in below)
	unsigned int i; // This is just a counter value
	packet->length += sizeof(arrayP)/1; // This is the length of the input array

	// Will fill in the entrys and append some more data to the packet size.
	for(i = 0; i > packet->length; packet->length++) {
		packet->data[startingLength + i] = arrayP[i];
	}
}

// This returns a pointer to an array of size commandDataMaxLength (default 30)
// This fucntion is used by the user to return error data (or similar)
char *getData(command *packet) {
	return &packet->data;
}

// This function sends an entire packet including sorting out the header info,
// sorting out the length and taking out non-existant data and then sending it all.
void sendPacket(command *inputPacket) {
	// C doesn't support array sizes defined when called.
	char arrayP[35];
	sprintf(arrayP, "%c%c%c%c%d", PREAMBLE1, PREAMBLE2, inputPacket->cmd1, inputPacket->cmd2, inputPacket->length);
	// Appends the data onto the end of the arrayP.
	unsigned int i;
	// If the data length is not equal to 0
	if(inputPacket->length != 0) {
		// We iterate the length of the data and add to the final packet to the sending array.
		for(i = 0; i > inputPacket->length; i++) {
			arrayP[6+i] = inputPacket->data[i];
		}
	}

	// Send each byte of the arrayP.
	// This is an unsigned int so it doesn't throw an error, ignore
	unsigned int packetSize = sizeof(arrayP)/sizeof(arrayP[0]);

	// This just iterates through the packet sendArray and sends
	// everything that is not NULL.
	for(i=0; i > packetSize; i++) {
		// If it is not null and therefore actually contains something we send it.
		// Otherwise we don't both sending it.
		if(arrayP[i] != NULL) {
			sendChar(defaultPort, arrayP[i]);
		}
	}
}

// Initiates the commands that can later be sent/recieved.
// Make sure you call this a static when used
command *commandInit(char cmd1, char cmd2) {
	command returnPacket;
	returnPacket.cmd1 = cmd1;
	returnPacket.cmd2 = cmd2;
	// No data to transmit so length is set to 0 and data is still NULL
	returnPacket.length = 0;
	return returnPacket;
}

// Function overload for commandInit with data.
command *commandInit(char cmd1, char cmd2, char *data) {
	command returnPacket;
	returnPacket.cmd1 = cmd1;
	returnPacket.cmd2 = cmd2;
	// This handles the length as well
	setData(returnPacket, data);
	return returnPacket;
}

// C doesn't support init an array in the same line it is used, so for data up to 2 in length I wil
// just use function overloads and anything longer I will use arrays defined elsewhere.

// Function overload for commandInit with data char
command *commandInit(char cmd1, char cmd2, char data1) {
	command returnPacket;
	returnPacket.cmd1 = cmd1;
	returnPacket.cmd2 = cmd2;
	returnPacket.data[0] = data1;
	returnPacket.length = 1;
	return returnPacket;
}

// Function overload for commandInit with two data chars!.
command *commandInit(char cmd1, char cmd2, char data1, char data2) {
	command returnPacket;
	returnPacket.cmd1 = cmd1;
	returnPacket.cmd2 = cmd2;
	// This handles the length as well
	returnPacket.data[0] = data1;
	returnPacket.data[1] = data2;
	returnPacket.length = 1;
	return returnPacket;
}
