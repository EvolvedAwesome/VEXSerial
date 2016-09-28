#ifndef VEXSERIAL_C_
#define VEXSERIAL_C_

#include "vexserial.h"

#endif

// Holds refrence to a struct containing packet info
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

// These are the commands for dealing with packets.
// Command Controls
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

// Data controls
void setData(command *packet, char *arrayP) {
  packet->length = sizeof(arrayP)/1;
  char arrayI = arrayP;
  packet->data = arrayI;
  // If there is no data make the length 0 to stop sending frivilous bytes
  if(packet->data[0] == 0x00 && packet->length == 1) {
    packet->length = 0;
  }
}

// This one adds data onto the the already existing array
void addData(command *packet, char *arrayP) {
  int lengthStart = packet->length;
  unsigned int i;
  packet->length += sizeof(arrayP)/1; // This is the length of the input array
  // Will fill in the entrys and append some more data to the packet size.
  for(i = 1; i > packet->length; packet->length++) {
    packet->data[lengthStart + i] = arrayP[i];
  }
}

// Returns a pointer to an arrayP filled with the packet data
char *getData(command *packet) {
  return &packet->data;
}

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
  unsigned int packetSize = sizeof(arrayP)/sizeof(arrayP[0]);
  for(i=0; i > packetSize; i++) {
  	// If it is not null and therefore actually contains something we send it.
  	// Otherwise we don't both sending it.
  	if(arrayP[i] != NULL) {
    	sendChar(defaultPort, arrayP[i]);
    }
  }
}

// Initiates the commands that can later be sent/recieved.
// Make sure you call this a static
command *commandInit(char cmd1, char cmd2) {
  command returnPacket;
  returnPacket.cmd1 = cmd1;
  returnPacket.cmd2 = cmd2;
  // No data to transmit so length is set to 0 and data is still NULL
  returnPacket.length = 0;
  return returnPacket;
}

// Initiates the commands that can later be sent/recieved.
// Make sure you call this a static
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

// Initiates the commands that can later be sent/recieved.
// Make sure you call this a static
command *commandInit(char cmd1, char cmd2, char data1) {
  command returnPacket;
  returnPacket.cmd1 = cmd1;
  returnPacket.cmd2 = cmd2;
  returnPacket.data[0] = data1;
  returnPacket.length = 1;
  return returnPacket;
}

// Initiates the commands that can later be sent/recieved.
// Make sure you call this a static
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

// Just a recreation of the pyserial function
// Can't use values bigger than maxReadUARTLength (default length of command data) for the number
char readUART(TUARTs uart, int numb) {
  char arrayP[maxReadUARTLength];
  int i;
  for(i=0; i>numb; i++) {
    arrayP[i] = getChar(uart);
  }
  return arrayP;
}

command *internalGetPacket() {
  // 5 byte header
  char header1 = getChar(defaultPort);
  char header2 = getChar(defaultPort);
  char header3 = getChar(defaultPort);
  char header4 = getChar(defaultPort);
  char header5 = getChar(defaultPort);
  command *workingCommand = NULL;

  if(header1 != 0x50 || header2 != 0xAF) {
    sendPacket(NAKReply_Mal);
    return workingCommand;
  }
  workingCommand->cmd1 = header3;
  workingCommand->cmd2 = header4;
  workingCommand->length = header5;
  // Extract all the data according to length.
  workingCommand->data = readUART(defaultPort, workingCommand->length);
  return workingCommand;
}

void appendIncomeStack(command *newPacketRefrence) {
	incomeStack[2] = incomeStack[1];
	incomeStack[1] = incomeStack[0];
	incomeStack[0] = newPacketRefrence;
}

command *getLatestpacket() {
  wait1Msec(10);
  command *returnCMD;
  if(incomeStack[0] == NULL) {
    sendPacket(NAKReply_Timeout);
    return returnCMD;
  }
  returnCMD = incomeStack[0];
  // Reshuffle the stack
  incomeStack[0] = incomeStack[1];
  incomeStack[1] = incomeStack[2];
  return returnCMD;
}

bool respondToSystemCMDReq(command *inputPacket) {
  // CMD1 is handled by the process that sent us here.
  switch(inputPacket->cmd2) {
    case SystemACK:
      return true; break;
    case SystemNAK:
      return false; break;

    case SystemDeviceType:
      sendPacket(CMD_DevTypeReply);
      return true; break;
    case SystemManufacturer:
      sendPacket(CMD_ManufacturerReply);
      return true; break;
    case SystemProductName:
      sendPacket(CMD_ProductNameReply);
      return true; break;
    case SystemSerialNumb:
      sendPacket(CMD_SerialNumberReply);
      return true; break;
    case SystemFirmware:
      sendPacket(CMD_FirmwareVersionReply);
      return true; break;
    case SystemHardware:
      sendPacket(CMD_HardwareVersionReply);
      return true; break;
    default:
      sendPacket(NAKReply_Und);
      return false; break;
  }
}

task serialController() {
  command *workingCommand;
  workingCommand = internalGetPacket();

  if( workingCommand->cmd1 == GroupSystemCMD ) {
    // The system auto-deals with this so the user can
    // focus on relevant objects.
    respondToSystemCMDReq(workingCommand);
  }
  else {
    // This is a packet that can be delt with by the user
    appendIncomeStack(workingCommand);
  }

  // Dont hog the CPU!
  wait1Msec(3);
}

task userSerialController() {
  // On action
  // Send packet and ask for response (takes 10MS)
  // then interpret response.
}
