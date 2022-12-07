# Arduino Animatronic Eyes
A computer science project I've build with a friend of mine. It's a pair of eyes which follows you through the room.

The original code is (mostly) copied from [github.com/Nimrod-Galor/eye-animatronics](https://github.com/Nimrod-Galor/eye-animatronics)

# Material list
* Arduino UNO
* 5x 9g servo
* ADNS 3080
* Jumper cables
* Optionally: IR LED's (+ matching resistors) for better visibility

# Schematics
![Schematics](/images/schematics.svg)

# Capturing frames from the camera
You can use [github.com/RCmags/ADNS3080_frame_capture](https://github.com/RCmags/ADNS3080_frame_capture) to capture frames from the ADNS 3080. It requires you to first flash the `arduino_serial_output.ino` to the arduino and then run the python script. Make sure to set the correct serial port in the python script.
