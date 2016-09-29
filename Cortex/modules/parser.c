#ifndef PARSER_C_
#define PARSER_C_
#endif

// This is a simple and easy way to copy a struct; If you want to use memcpy and
// blow my small fragile mind, then go ahead :)
// Declared as static and not prototyped, don't try and use elsewhere!
static void copyStruct(command *newStruct, command *fromStruct) {
	newStruct->cmd1 = fromStruct->cmd1;
	newStruct->cmd2 = fromStruct->cmd2;
	newStruct->length = fromStruct->length;
	newStruct->data = fromStruct->data;
}

// Returns either data or True/False
returnVar *respondToSystemCMDReq(command *inputPacket) {
	// Create a union for the return
	returnVar *returnInput;
	// CMD1 is handled by the process that sent us here.
	switch(inputPacket->cmd2) {
	case SystemACK:
		returnInput->result = true; break;
	case SystemNAK:
		returnInput->data = getData(inputPacket); break;

	case SystemDeviceType:
		sendPacket(CMD_DevTypeReply);
		returnInput->result = true; break;
	case SystemManufacturer:
		sendPacket(CMD_ManufacturerReply);
		returnInput->result = true; break;
	case SystemProductName:
		sendPacket(CMD_ProductNameReply);
		returnInput->result = true; break;
	case SystemSerialNumb:
		sendPacket(CMD_SerialNumberReply);
		returnInput->result = true; break;
	case SystemFirmware:
		sendPacket(CMD_FirmwareVersionReply);
		returnInput->result = true; break;
	case SystemHardware:
		sendPacket(CMD_HardwareVersionReply);
		returnInput->result = true; break;
	default:
		sendPacket(NAKReply_Und);
		returnInput->data = getData(inputPacket); break;
	}
	return returnInput;
}

returnVar *respondToControlReq(command *inputPacket) {
	// Create a union for the return
	returnVar *returnInput;

	// This is the index value, as even when not needed it is included to make life easy.
	signed int indexRequestValue = inputPacket->data[0];
	// Calculate the control req value. We remove 127 as the value given is unsigned
	signed int controlRequestValue = inputPacket->data[1] - 127;

	// CMD1 is handled by the process that sent us here.
	switch(inputPacket->cmd2) {
	case setAllMotors:
		F_setAllMotors(controlRequestValue);
		sendPacket(CMD_ACKReply);
		returnInput->result = true; break;
	case setMotorIndex:
		F_setMotor(indexRequestValue, controlRequestValue);
		sendPacket(CMD_ACKReply);
		returnInput->result = true; break;
	default:
		sendPacket(NAKReply_Und); break;
	}
	return returnInput;
}

returnVar *respondToStatusReq(command *inputPacket) {
	// Create a union for the return
	returnVar *returnInput;

	// This is the index value, as even when not needed it is included to make life easy.
	char statusIndexValue = inputPacket->data[0];

	// CMD1 is handled by the process that sent us here.
	switch(inputPacket->cmd2) {
	case getMotorStatus:
		// Add 127 to make the values 0-254
		ubyte motorSpeed = F_getMotorStatus(statusIndexValue) + 127;
		// Create a new struct entry
		command sendP;
		copyStruct(sendP, CMD_GetMotorStatusReply);
		// Set the data used
		setData(sendP, motorSpeed);
		// Send the response data!
		sendPacket(sendP);
		returnInput->result = true; break;

	default:
		sendPacket(NAKReply_Und); break;
	}
	return returnInput;
}

// Tasks.

task serialController() {

	command *workingCommand;
	workingCommand = internalGetPacket();
	switch(workingCommand->cmd1) {
	case GroupSystemCMD:
		// The system auto-deals with this so the user can
		// focus on relevant objects.
		respondToSystemCMDReq(workingCommand);
		break;
	case GroupControl:
		// The system can auto-deal with this one as well.
		respondToControlReq(workingCommand);
		break;

	case GroupStatus:
		// The system can even auto-deal with this one
		respondToStatusReq(workingCommand);
		break;

	default:
		// This packet is eiether NAK or can be delt with by the user :)
		appendIncomeStack(workingCommand);
		break;

		// Dont hog the CPU!
		wait1Msec(3);
	}
}
