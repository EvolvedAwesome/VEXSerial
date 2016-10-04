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

#define setAllMotors 0x10
#define setMotorIndex 0x11

#define getMotorStatus 0x10

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
  // set to NULL will just not be sent. (So any unset data)
  char data[commandDataMaxLength];
} command;


// Function Prototypes

// This func just sets input packets data to the data supplied and
// sets the length appropriatly.
void setData(command *packet, char element);
void setData(command *packet, char *arrayP);

// This one adds data onto the the already existing array
// This means we can add or make changes to data live.
// Returns false if the data is too long to fit the packet + current data.
// ReturnType: true/false
bool addData(command *packet, char *arrayP);

// Initiates the commands that can later be sent/recieved.
// Make sure you call this a static when used
// ReturnType: command Struct
command *commandInit(char cmd1, char cmd2);
command *commandInit(char cmd1, char cmd2, char *data);
command *commandInit(char cmd1, char cmd2, char data1);
command *commandInit(char cmd1, char cmd2, char data1, char data2);

// Takes the raw packet and parses it into something that can
// be sent over serial (doing checks on the integrity). Returns false
// if it could not send. True if it sent.
// ReturnType: true/false
bool sendPacket(command *inputPacket);

// Places a fully assembled packet into the IncomeStack;
// The income stack acts as a buffer allowing you to collect the latest
// reply packet.
void appendIncomeStack(command *newPacketRefrence);

// Gets the latest fully constructed packet in the IncomeStack
// Returns an empty responseStruct if no packet is found.
// ReturnType: command Struct
command *getLatestPacket();

// Encapsulation function for user operated packets
// Use this to send packets and get a respone in a function.
// ReturnType: response command;
command *sendCommand(command *commandSend, int timeout);

// Dependency Prototypes
void F_setAllMotors(int value);
void F_setMotor(int index, int value);
ubyte F_getMotorStatus(char index);

#pragma systemFile

#ifndef VEXSERIAL_H_
#define VEXSERIAL_H_

#include "modules/commands.c"
#include "modules/incomeBuffer.c"
#include "modules/serialController.c"
#include "modules/cortexInterface.c"

#endif
