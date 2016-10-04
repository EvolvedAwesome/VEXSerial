// Adds another packet to the income stack and reshuffles the rest of the stack
void appendIncomeStack(command *newPacketRefrence) {
	// Gets rid of the 3rd entry, pushes the 2nd to third then
	// pushes the 1st to 2nd and pushes a new 1st to the front.
  incomeStack[2] = incomeStack[1];
  incomeStack[1] = incomeStack[0];
  incomeStack[0] = newPacketRefrence;
}

// Gets the latest packet from the packetStack "Buffer"
command *getLatestPacket() {
	// Create a return structure
  command *returnCMD;
  // This means that there is nothing on the incomeStack
  if(incomeStack[0] == NULL) {
		// We are simply going to return the CMD structure without any
  	// information (which means we couldn't find any info)
    return returnCMD;
  }
  // Otherwise we place the last addition to the incomeStack as a return
  returnCMD = incomeStack[0];
  // Reshuffle the stack
  incomeStack[0] = incomeStack[1];
  incomeStack[1] = incomeStack[2];
  // Return the filled structure.
  return returnCMD;
}
