#include "vexserial.h"

task main() {
	// Start the task using the defauly baudrate and communiciation settings.
  startTask(serialController, kHighPriority);

  // Create a returnpacket pointer (because otherwise robotc
  // will complain about dynamic initilaization of structures).
  command *returnPacket = sendCommand(CMD_DevTypeRequest, 10);

  // Fill a string with some debug informatin for the debugStream
  string returnString; stringFormat( returnString, "Command 1: %c/nCommand 2: %c/nLength: %d/nData: %c/n",
                                    returnPacket->cmd1, returnPacket->cmd2, returnPacket->length, returnPacket->data );

  // Write the formatted string to debug stream on a new line.
  writeDebugStreamLine(returnString);

  while( true ) { wait1Msec(50); }
}
