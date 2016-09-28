#VexSerial Library :satellite:
###Solid packet based communication between a VEX Cortex and a Raspberry Pi through UART (Serial).

####Brief
This library was designed as a holiday project in order to establish the first steps to a robust communication protocol in order to easily make changes to the cortex from the pi (and anything that it is connected to). The protocol is based on the p3cortex adaption by jpearman [link.](http://www.vexforum.com/index.php/10032-communicating-between-two-cortex-dual-cortex-robot/0). This is the basis for several projects that are upcoming using the cortex for myself as well as in debugging and testing of my teams competition robot.

####Components
This library is made up of a RobotC component and a python program written that utilises pyserial on the Raspberry Pi's UART ports. Instructions for using the UART port on the Pi can be found with a quick google search: [example instructions.](https://www.google.co.nz/search?q=raspberry+pi+uart&oq=raspberry+pi+uart&aqs=chrome..69i57j69i60j69i59j69i60j69i59j69i60.2736j0j7&sourceid=chrome&ie=UTF-8). The 2 UART ports on the cortex are utalised singularly for using a VEX LCD. However since the cortex only supports one LCD at a time (without writing your own secondary controller), we are usually left with a spare UART port that we can have a play with. 

*Note: This is not legal in Competition (unless you're a lucky VexU-er).*

What your gonna need:
* Raspberry Pi (w/Power Source)
* Vex Cortex (w/battery)
* Jumper wires (for connecting the two electronically)
* RobotC (or any language if you feel like porting it)
* A micro SD card loaded with the latest version of Raspbian

####Connections
>> Gnd -> Gnd
>> Rx  -> Tx
>> Tx  -> Rx
>> 5v is left unconnected on both sides.

####Commands
Currently, the commands are very similar as the commands used in p3 (with a datasheet avalaible [here.](https://github.com/jpearman/p3cortex/blob/master/Protocol_d1.pdf)).
