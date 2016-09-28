#include "vexserial.c"

task main() {
  setBaudRate(defaultPort, defaultBaudRate);

  startTask(userSerialController);
  startTask(serialController, kHighPriority);
}
