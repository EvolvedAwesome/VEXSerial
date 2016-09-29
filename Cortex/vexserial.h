// ## This is the vexSerial Header File 									## //
// ## This file contains the base const variables (defined as preprocessors ## //
// ## as well as the base structures and all the function protoypes. 		## //

// ## Attribution: 															## //
// ## - EvolvedAwesome (Author) 											## //
// ## - jPearman (protocol basis, inspiration and generally being awesome) 	## //

#ifndef VEXSERIAL_H_
#define VEXSERIAL_H_

#endif

// Well I changed this to enum types :)
// These are the group 1 commands
#define GroupSystemCMD 0
#define GroupSystemReply 1
#define GroupControl 2
#define GroupPreset 4
#define GroupStatus 6
#define GroupStatusReply 7
#define GroupFactory 0x0F

// These are the group 2 commands (so the subcommands)
#define  SystemACK 0x10
#define  SystemDeviceType 0x11
#define  SystemNAK 0x12
#define  SystemManufacturer 0x13
#define  SystemProductName 0x14
#define  SystemSerialNumb 0x15

#define  SystemFirmware 0x20
#define  SystemHardware 0x21

#define  CortexDeviceID 0x00
#define  GlobalDeviceID 0x0f

// This is the maximum size of data in a command packet.
// I have to enforce this due to limitations in RobotC
#define commandDataMaxLength 30

// This is the max length of the structure refrence stack that you
// can grab incoming user-maluable information from.
#define incomeStackMaxLength 3

// This is the maximim size of the information the function readUART
// can take as an input and return as an output.
// Note: It returns an array this size EVERY time it is called.
#define maxReadUARTLength commandDataMaxLength

// This is the maxmimum size of the debug data that can be returned in
// the return Value Union. If you run into isues set this to commandDataMaxLength.
#define maxDataSize 5

// These are the two header preambles, you could change these, but these two
// work well (Thanks jpearman :)
#define PREAMBLE1 0x50
#define PREAMBLE2 0xAF

// The output port details.
// defaultPort: The TUARTs port that the user will recieve and output on.
// defaultBaudRate: The rate at which data is sent and recieved. This needs
// to be the same to what the PI has the baudrate set to.
#define defaultPort UART1
#define defaultBaudRate baudRate115200

// This is the struct holding the command and packet information
// for every sent and recieved packet.
typedef struct _command {
  char cmd1;
  char cmd2;
  unsigned int length;
  // We can send up to this amount of data as any data 
  // set to NULL will just not be sent.
  // Note: This is a limitation of C and I don't really want
  // to use Dynamic Memory Allocation as this is robotc (although you could).
  char data[commandDataMaxLength]; 
} command;

// This union is designed so that I can return not only True/False
// for good or bad transactions. But when data (up to maxDataSize)
// is avaliable, I can alternativly return that.
union _returnVar {
	bool result;
	char data[maxDataSize];
} returnVar;


// Function Prototypes
void setCMD1(command *packet, char cmd1Value);
void setCMD2(command *packet, char cmd2Value);
char getCMD1(command *packet);
char getCMD2(command *packet);

void setData(command *packet, char *arrayP);
void addData(command *packet, char *arrayP);
char *getData(command *packet);

command *commandInit(char cmd1, char cmd2);
command *commandInit(char cmd1, char cmd2, char *data);
command *commandInit(char cmd1, char cmd2, char data1);
command *commandInit(char cmd1, char cmd2, char data1, char data2);
command *internalGetPacket();
command *getLatestpacket();

void sendPacket(command *inputPacket);
char readUART(TUARTs uart, int numb);
char readUART(TUARTs uart);
void appendIncomeStack(command *newPacketRefrence);
bool respondToSystemCMDReq(command *inputPacket);
