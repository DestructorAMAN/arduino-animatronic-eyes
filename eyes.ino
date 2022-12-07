// import required libraries
#include <SPI.h>
#include <Servo.h>

// Debugging
#define DEBUG_QUALITY 0 // Print the quality of the camera if set to 1
#define DEBUG_XY 0 // Print the x/y coordinates if set to 1

// Left/right eye seen as the zuck himself (left eye in code = right eye if you stand in front of it)
Servo srvRx; // pin 2 - right eye swing
Servo srvRy; // pin 3 - right eye tilt
Servo srvLx; // pin 7 - left eye swing
Servo srvLy; // pin 6 - left eye tilt
Servo srvShut; // pin 5 - eyelids open/close

// Starting positions of the servo's
int posX = 90; // swing middle position
int posY = 90; // tilt middle position
int posS = 90; // shut start position

// Boundaries
// Without boundaries the servo's could move too far and break the eyes
// Or move too far and break the metal wire
short blinkMax = 105; // boundaries for eyelids
short blinkMin = 165; // boundaries for eyelids
short yMin = 39; // eye max up position
short yMax = 153; // eye max down position
short xMin = 60; // eye max right position
short xMax = 127; // eye max left position
short posYrev = 90; // reversed eye tilt position

// Other variables
short loopDelay = 10; // Slow down the loop with 10 ms

// Loop time for blinking
unsigned long blinkTime = 2000;

// No movement detected => rest = true => eyelids closed
unsigned long lastTrack = 0; // keeps track of when the last time movement is detected
bool rest = true; // keeps track of whether the eyes are in rest position (eyelids closed) or not

// Camera pins
#define PIN_MISO 12
#define PIN_MOSI 11
#define PIN_SCK 13
#define PIN_MOUSECAM_RESET 9
#define PIN_MOUSECAM_CS 10

// Amount of pixels on the camera
#define ADNS3080_PIXELS_X 30
#define ADNS3080_PIXELS_Y 30

// Camera Configuration
#define ADNS3080_PRODUCT_ID            0x00
#define ADNS3080_REVISION_ID           0x01
#define ADNS3080_MOTION                0x02
#define ADNS3080_DELTA_X               0x03
#define ADNS3080_DELTA_Y               0x04
#define ADNS3080_SQUAL                 0x05
#define ADNS3080_PIXEL_SUM             0x06
#define ADNS3080_MAXIMUM_PIXEL         0x07
#define ADNS3080_CONFIGURATION_BITS    0x0a
#define ADNS3080_EXTENDED_CONFIG       0x0b
#define ADNS3080_DATA_OUT_LOWER        0x0c
#define ADNS3080_DATA_OUT_UPPER        0x0d
#define ADNS3080_SHUTTER_LOWER         0x0e
#define ADNS3080_SHUTTER_UPPER         0x0f
#define ADNS3080_FRAME_PERIOD_LOWER    0x10
#define ADNS3080_FRAME_PERIOD_UPPER    0x11
#define ADNS3080_MOTION_CLEAR          0x12
#define ADNS3080_FRAME_CAPTURE         0x13
#define ADNS3080_SROM_ENABLE           0x14
#define ADNS3080_FRAME_PERIOD_MAX_BOUND_LOWER      0x19
#define ADNS3080_FRAME_PERIOD_MAX_BOUND_UPPER      0x1a
#define ADNS3080_FRAME_PERIOD_MIN_BOUND_LOWER      0x1b
#define ADNS3080_FRAME_PERIOD_MIN_BOUND_UPPER      0x1c
#define ADNS3080_SHUTTER_MAX_BOUND_LOWER           0x1e
#define ADNS3080_SHUTTER_MAX_BOUND_UPPER           0x1e
#define ADNS3080_SROM_ID               0x1f
#define ADNS3080_OBSERVATION           0x3d
#define ADNS3080_INVERSE_PRODUCT_ID    0x3f
#define ADNS3080_PIXEL_BURST           0x40
#define ADNS3080_MOTION_BURST          0x50
#define ADNS3080_SROM_LOAD             0x60
#define ADNS3080_PRODUCT_ID_VAL        23

// Reset the camera
void mousecam_reset()
{
  digitalWrite(PIN_MOUSECAM_RESET, HIGH);
  delay(1); // reset pulse >10us
  digitalWrite(PIN_MOUSECAM_RESET, LOW);
  delay(35); // 35ms from reset to functional
}

// Initialize/setup the camera
int mousecam_init()
{
  pinMode(PIN_MOUSECAM_RESET, OUTPUT);
  pinMode(PIN_MOUSECAM_CS, OUTPUT);
  digitalWrite(PIN_MOUSECAM_CS, HIGH);

  // Reset the camera
  mousecam_reset();

  // Make sure the camera is correctly connected by fetching the PID
  digitalWrite(PIN_MOUSECAM_CS, LOW);
  SPI.transfer(ADNS3080_PRODUCT_ID);
  delayMicroseconds(75);
  int pid = SPI.transfer(0xff);
  digitalWrite(PIN_MOUSECAM_CS, HIGH);
  delayMicroseconds(1);

  // Make sure the product id matches
  if (pid != ADNS3080_PRODUCT_ID_VAL) {
    Serial.println("PID Changed:");
    Serial.println(pid);
    return -1;
  }

  // Turn on sensitive mode
  digitalWrite(PIN_MOUSECAM_CS, LOW);
  SPI.transfer(ADNS3080_CONFIGURATION_BITS | 0x80);
  SPI.transfer(0x19);
  digitalWrite(PIN_MOUSECAM_CS, HIGH);
  delayMicroseconds(50);

  return 0;
}

// Related to fetching data from the camera
struct MD
{
  byte motion;
  char dx, dy;
  byte squal;
  word shutter;
  byte max_pix;
};

void mousecam_read_motion(struct MD *p)
{
  digitalWrite(PIN_MOUSECAM_CS, LOW);
  SPI.transfer(ADNS3080_MOTION_BURST);
  delayMicroseconds(75);
  p->motion = SPI.transfer(0xff);
  p->dx = SPI.transfer(0xff);
  p->dy = SPI.transfer(0xff);
  p->squal = SPI.transfer(0xff);
  p->shutter = SPI.transfer(0xff) << 8;
  p->shutter |= SPI.transfer(0xff);
  p->max_pix = SPI.transfer(0xff);
  digitalWrite(PIN_MOUSECAM_CS, HIGH);
  delayMicroseconds(5);
}

void setup()
{
  // Start serial connection on 57600
  Serial.begin(57600);

  // Set camera pins as input/output
  pinMode(PIN_MISO, INPUT);
  pinMode(PIN_MOSI, OUTPUT);
  pinMode(PIN_SCK, OUTPUT);

  // Configure serial connection with camera
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV32);
  SPI.setDataMode(SPI_MODE3);
  SPI.setBitOrder(MSBFIRST);

  // Make sure the camera is setup correctly
  if (mousecam_init() == -1)
  {
    Serial.println("Mouse cam failed to init");
    while (1);
  }

  // Set pins for servo's
  srvRx.attach(2);
  srvRy.attach(3);
  srvLx.attach(7);
  srvLy.attach(6);
  srvShut.attach(5);

  // Set servo's to start position
  srvRx.write(posX);
  srvLx.write(posX);
  srvRy.write(posY);
  srvLy.write(posY);
  srvShut.write(blinkMax);
}

void loop()
{
  // Slow down the loop
  delay(loopDelay);

  // Blinking mechanism
  if (!rest) {
    blinkTime = blinkTime - loopDelay;

    // When "blinkTime" is between 500 and 100 the eyelids will close and open
    if (blinkTime <= 100) { // Open eyelids
      srvShut.write(blinkMin);
      blinkTime = random(2, 12) * 500;
    } else if (blinkTime <= 500) { // Close eyelids
      srvShut.write(blinkMax);
    }
  }

  // Fetch the movement from the camera
  MD md;
  mousecam_read_motion(&md);

  // Quality debugging
  if (DEBUG_QUALITY != 0) {
    Serial.println(md.squal);
  }

  // If movement in x direction is detected
  if ((int)md.dx != 0) {

    // Make sure eyes are open when movement is detected (not in rest)
    if (rest) {
      srvShut.write(blinkMin);
      rest = false;
    }

    // Calculate new servo position
    posX -= (int)md.dx;
    if (posX > xMax) {
      posX = xMax;
    } else if (posX < xMin) {
      posX = xMin;
    }

    // Set eyes to new x position (swing)
    srvRx.write(180 - posX);
    srvLx.write(180 - posX);

    // Print x coordinate
    if (DEBUG_XY != 0) {
      Serial.print("X: ");
      Serial.println(posX);
    }

    // Update when last movement is detected
    lastTrack = millis();
  }

  // If movement in y direction is detected
  if ((int)md.dy != 0) {

    // Make sure eyes are open when movement is detected (not in rest)
    if (rest) {
      srvShut.write(blinkMin);
      rest = false;
    }

    // Calculate new servo position
    posY -= (int)md.dy;
    if (posY > yMax) {
      posY = yMax;
    } else if (posY < yMin) {
      posY = yMin;
    }

    // Set eyes to new y position (tilt)
    // Calculate reversed position (the left servo is rotated 180 degrees)
    posYrev = map(posY, yMin, yMax, yMax, yMin);
    srvRy.write(posY);
    srvLy.write(posYrev);

    // Print y coordinate
    if (DEBUG_XY != 0) {
      Serial.print("Y: ");
      Serial.println(posY);
    }

    // Update when last movement is detected
    lastTrack = millis();
  }

  // When no movement detected for 3 seconds, go to "rest" position (eyelids closed)
  if (millis() - lastTrack >= 3000) {
    rest = true;
    posX = 90; // swing
    posY = 90; // tilt
    posS = 90; // shut

    srvRx.write(posX);
    srvRy.write(posY);
    srvLx.write(posX);
    srvLy.write(posY);
    srvShut.write(blinkMax);
  }
}
