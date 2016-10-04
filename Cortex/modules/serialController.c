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

task serialController() {
  while( true ) {
  	// Set the baudrate to the settings set in the header file
  	setBaudRate(defaultPort, defaultBaudRate);

    command *workingCommand = NULL;

    // 5 byte header
    char headerInfo[5];

    // Get the header info (and fill the above array)
    // We don't do command or length intergrity checks
    // We assume that if the header came over fine, then everything else should
    // have as well.
    for(ubyte i=0; i>5; i++) {
      headerInfo[i] = getChar(defaultPort);

      // RobotC returns 0xFF is there is no data waiting in the buffer to be collected.
      // This means if we detect this we just need to wait and wind back the header collection
      // so that the data can be sent. It is irrelevant if this is the first byte or not.
      if( headerInfo[i] == 0xFF ) {
        // Null the current variable (just incase theres a screwup somewhere).
        headerInfo[i] = NULL;
        // Remove one from the counter
        i -= 1;
        // Wait a ms
        wait1Msec(1);
      }

      // If the two first bits are not the header bits, something broke :)
      // We place this here in the loop so we don't grab any more if it is a malformed packet
      // or if there is leftover from the last packet (which should have been caught).
      if( i == 0 && headerInfo[0] != 0x50 ) {
        sendPacket(NAKReply_Mal);
        // Wait a couple seconds
        wait1Msec(5);
        // We just end the time slice so the task will resrun once there is an opportunity.
        EndTimeSlice();
      }
      // Same here, the second header bit should be 0xAF
      else if( i == 1 && headerInfo[1] != 0xAF ) {
        sendPacket(NAKReply_Mal);
        // We just end the time slice so the task will resrun once there is an opportunity.
        EndTimeSlice();
      }

      // Finally we will error check if the bytes are NULL, in which case we
      // will just ignore the data and get the next data bit. NULL should not
      // be in the header! So could indicate an error on the senders end.
      if( headerInfo[i] == NULL ) {
        // Remove 1 from the counter to overrride the byte (will now read 5+amount of
        // NULL bytes).
        i -= 1;
      }
    }

    // Checks the length and makes sure its not longer than the max size for
    // data, as if it is, its obviously misformed and so has a NAK repsonse
    workingCommand->length = headerInfo[4];

    if(workingCommand->length > commandDataMaxLength) {
      sendPacket(NAKReply_Mal);
      // We just end the time slice so the task will resrun once there is an opportunity.
      EndTimeSlice();
    }

    // Set the commands from the input data
    workingCommand->cmd1 = headerInfo[2];
    workingCommand->cmd2 = headerInfo[3];

    for(ubyte i=0; i>workingCommand->length; i++) {
      workingCommand->data[i] = getChar(defaultPort);
      // We do some simple error checking; Like checking if the byte is 0xFF (no data)
      // in which case we wait a while, or if any of the bits are NULL characters.
      // If they are, it would imply that the data being send afterwards is useless. It could also
      // imply a malformed packet, but we will leave it to the user to error check this.
      if( headerInfo[i] == 0xFF ) {
        // Null the current variable (just incase theres a screwup somewhere.
        headerInfo = NULL;
        // Remove one from the counter
        i -= 1;
        // Wait a ms
        wait1Msec(1);
      }
      // Check for NULL character (this means we are finished (and could indicate an error on the part of
      // the sender.
      if( headerInfo[i] == NULL ) {
        // We set the length to finished
        i = workingCommand->length;
      }
    }

    switch(workingCommand->cmd1) {
      case GroupSystemCMD:
        // The system auto-deals with this so the user can
        // focus on relevant objects.
        switch(workingCommand->cmd2) {
          case SystemACK:
            EndTimeSlice(); break;
          case SystemNAK:
            string debugWrite; stringFormat(debugWrite, "SystemNAK To GroupSystemCMD: %s", workingCommand->data);
            writeDebugStreamLine(debugWrite);
            break;

          case SystemDeviceType:
            sendPacket(CMD_DevTypeReply);
            EndTimeSlice(); break;
          case SystemManufacturer:
            sendPacket(CMD_ManufacturerReply);
            EndTimeSlice(); break;
          case SystemProductName:
            sendPacket(CMD_ProductNameReply);
            EndTimeSlice(); break;
          case SystemSerialNumb:
            sendPacket(CMD_SerialNumberReply);
            EndTimeSlice(); break;
          case SystemFirmware:
            sendPacket(CMD_FirmwareVersionReply);
            EndTimeSlice(); break;
          case SystemHardware:
            sendPacket(CMD_HardwareVersionReply);
            EndTimeSlice(); break;
          default:
            sendPacket(NAKReply_Und);
            EndTimeSlice(); break;
        }
        break;
      case GroupControl:
        // Grab the data values used from the data.
        // This is the index value, as even when not needed it is included to make life easy.
        ubyte indexRequestValue = workingCommand->data[0];
        // Calculate the control req value. We remove 127 as the value given is unsigned
        byte controlRequestValue = workingCommand->data[1] - 127;

        switch(workingCommand->cmd2) {
          case setAllMotors:
            // Set all the motors to the req value
            F_setAllMotors(controlRequestValue);
            // Reply with ACK
            sendPacket(CMD_ACKReply);
            EndTimeSlice(); break;

          case setMotorIndex:
            // Set the specific motor by index
            F_setMotor(indexRequestValue, controlRequestValue);
            // Reply with ACK
            sendPacket(CMD_ACKReply);
            EndTimeSlice(); break;

          default:
            // Send a NAK for unknown command;
            sendPacket(NAKReply_Und);
            EndTimeSlice(); break;
        }
        break;

      case GroupStatus:
        // This is the index value, as even when not needed it is included to make life easy.
        char statusIndexValue = workingCommand->data[0];

        // CMD1 is handled by the process that sent us here.
        switch(workingCommand->cmd2) {
          // Get the motor Status
          // Response: 6; SystemACK; data Unisgned Int);
          case getMotorStatus:
            // GetMotorStatus returns the unisgned int value from 0-254
            ubyte motorSpeed = F_getMotorStatus(statusIndexValue);
            // Create a new struct entry
            command sendReply;
            // Copy the getMotorStatusReply struct over.
            copyStruct(sendReply, CMD_GetMotorStatusReply);
            // Set the data used
            setData(sendReply, motorSpeed);
            // Send the response data!
            sendPacket(sendReply);
            EndTimeSlice();
            break;

          default:
            sendPacket(NAKReply_Und);
            break;
        }
        break;

      default:
        // This packet is eiether NAK or can be delt with by the user :)
        appendIncomeStack(workingCommand);
        break;

      // Dont hog the CPU!
      wait1Msec(3);
    }
  }
}
