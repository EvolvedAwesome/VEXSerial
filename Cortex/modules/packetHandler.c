// Just a recreation of the pyserial function
// Can't use values bigger than maxReadUARTLength (default length of command data) for the number.
// If you are calling this funciton you are not nessiserially waiting for something to come.
char readUART(TUARTs uart, int numb) {
	char arrayP[maxReadUARTLength];
	int i;
	// Get all that information goodness.
	// If there is no information it will just hang out untill there is; So this MUST be
	// in a sepeate task to all your other stuff or bad things will happen.
	for(i=0; i>numb; i++) {
		arrayP[i] = getChar(uart);
		if(arrayP[i] == 0xFF) {
			// Stay on the same array element
			i -= 1;
			// wait a MS for the data to arrive.
			wait1Msec(1);
		}
	}
	return arrayP;
}

// This auto hangs but always gives you the data. Unlike getChar nativly
char readUART(TUARTs uart) {
	// This will just repeat untill there is information ready.
	char returnValue = getChar(uart);
	while(returnValue != 0xFF) {
		if(returnValue == 0xFF) {
			// wait a MS for the data to arrive.
			wait1Msec(1);
			// Then we are going to go back and try again.
		}
	}
	return returnValue;
}
// Is an internal function to get the lastest packet, must be in seperate task
// as it will hand when getting data untill data is avaliable.
command *internalGetPacket() {
	// 5 byte header
	char header1 = readUART(defaultPort);
	char header2 = getChar(defaultPort);
	char header3 = getChar(defaultPort);
	char header4 = getChar(defaultPort);
	char header5 = getChar(defaultPort);
	command *workingCommand = NULL;

	// If the two first bits are not the header bits, something broke :)
	if(header1 != 0x50 || header2 != 0xAF) {
		// We return a NAK Malformed Packet response
		sendPacket(NAKReply_Mal);
		return workingCommand;
	}

	// Checks the length and makes sure its not longer than the max size for
	// data, as if it is, its obviously misformed and so has a NAK repsonse
	workingCommand->length = header5;
	if(workingCommand->length > commandDataMaxLength) {
		sendPacket(NAKReply_Mal);
		return workingCommand;
	}

	// Set the commands from the input data
	workingCommand->cmd1 = header3;
	workingCommand->cmd2 = header4;

	// Extract all the data according to the recorded length.
	workingCommand->data = readUART(defaultPort, workingCommand->length);

	// Parity checking would probably be a good idea right about now :)
	return workingCommand;
}

// Adds another packet to the income stack and reshuffles the rest of the stack
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
