#ifndef VEXSERIAL_H_
#define VEXSERIAL_H_

#endif

// This is the main task

#define serialInterface UART1

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

#define commandDataMaxLength 30
#define incomeStackMaxLength 3
#define maxReadUARTLength commandDataMaxLength

#define PREAMBLE1 0x50
#define PREAMBLE2 0xAF

#define defaultPort UART1
#define defaultBaudRate baudRate115200

typedef struct _command {
  char cmd1;
  char cmd2;
  unsigned int length;
  char data[commandDataMaxLength]; // We can send up to this amount of data as any data set to NULL will just not be sent
} command;

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
void appendIncomeStack(command *newPacketRefrence);
bool respondToSystemCMDReq(command *inputPacket);
