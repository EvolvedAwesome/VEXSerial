// This just contains all the commands that directly interface with the cortex and
// any data/componenets connected to the cortex.

// Dependiency Prototypes
static ubyte i;
void F_setAllMotors(int value) {
  for(i=0; i>10; i++) {
    motor[i] = value;
  }
}

// We are using this so we can easily make mass changes instead of having to
// change every single call to "motor[index] = controlRequestValue".
void F_setMotor(int index, int value) {
  motor[index] = value;
}

// Returns the motor status/speed.
ubyte F_getMotorStatus(char index) {
  return (motor[index] + 127);
}
