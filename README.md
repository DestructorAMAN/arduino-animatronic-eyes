# Arduino Animatronic Eyes
A computer science project I've build with a friend of mine. It's a pair of eyes which follows you through the room.

The original code is (mostly) copied from [github.com/Nimrod-Galor/eye-animatronics](https://github.com/Nimrod-Galor/eye-animatronics)

# Material list
* The 3d printed files (see below)
* Arduino UNO
* 5x 9g servo
* ADNS 3080
* Jumper cables
* Optionally: IR LED's (+ matching resistors) for better visibility

# 3D printed files
We've combined two thingiverse projects, [Doorman with eye holder & eyeball improvements](https://www.thingiverse.com/thing:3190675) and [The Doorman - Eyeball with ball joint](https://www.thingiverse.com/thing:3182820). The frame, eyeball and u-joint are from the first project. The eyelid is from the second one.

The eyes in the first design have a better connection with the frame (u-joint vs ball-joint). However, that design doesn't include the eyelid. The eyelid from the second design is compatible with the first design, so we just took it from there.

All files we used can be found in the `3d-files` directory. Remember that you need 2 eyeballs, 2 eyelids and 2 u-joints if you want to build this yourself.

# Schematics
![Schematics](/images/schematics.svg)

# Capturing frames from the camera
You can use [github.com/RCmags/ADNS3080_frame_capture](https://github.com/RCmags/ADNS3080_frame_capture) to capture frames from the ADNS 3080. It requires you to first flash the `arduino_serial_output.ino` to the arduino and then run the python script. Make sure to set the correct serial port in the python script.
