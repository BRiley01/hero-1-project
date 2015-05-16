/*
  - PROTOCOL - 
  All Responses first byte is OPCODE that it is responding to
  First byte received is the opcode:
    0x00: STATUS
      OPERAND: 
        none
      RESPONSE: 
        STATUS_READY 0x01
        STATUS_MOVING 0x02
        STATUS_FAULT 0xFF
    0x01: WHEEL POSITION
      OPERAND:
        0 - 180 where 90 = straight
    0x02: DRIVE
      OPERAND:
        BYTE - LS 6 bits = speed
               bit 7 - direction (0 forward, 1 reverse)
    0x03: RECALIBRATE
    ------
    0xFE: ABORT
      OPERAND:
        none
      RESPONSE:
        none
*/ 

#include <Wire.h>
#include <Stepper.h>

#define SLAVE_ADDRESS 0x05

#define OPCODE_STATUS 0x00
#define OPCODE_WHEEL_POSTITION 0x01
#define OPCODE_DRIVE 0x02
#define OPCODE_RECALIBRATE 0x03
#define OPCODE_ABORT 0xFE

#define BUFFER_SIZE 255

// change this to the number of steps on your motor
#define STEPS 300

enum STATUS {READY = 0x01, MOVING = 0x02, FAULT = 0xFF};

// create an instance of the stepper class, specifying
// the number of steps of the motor and the pins it's
// attached to
Stepper stepper(STEPS, 3, 4, 5, 6);

// the previous reading from the analog input
int previous = 0;
int i = 0;
int newSpeed = 0;
volatile STATUS currStatus = READY;
volatile boolean StatusReq = false;
volatile byte buffer[BUFFER_SIZE];
volatile byte* loopOp = &buffer[BUFFER_SIZE - 1];
volatile byte* pCurrOp = buffer;
volatile byte* pEndOp = buffer;

void setup()
{
  // set the speed of the motor to 30 RPMs
  stepper.setSpeed(30);
  
  //stepper.step(1200);
  pinMode(A0, OUTPUT); 
  DDRB = B00111111;
  PORTB = 0;
  //digitalWrite(A0, HIGH);
  PORTB = B00111111;
   
  Wire.begin(SLAVE_ADDRESS);
  Wire.onReceive(receiveData);
  Wire.onRequest(sendData);
}

void loop()
{
  
}

byte getOp()
{
  pCurrOp++;
  if(pCurrOp == loopOp)
    pCurrOp = buffer;
  return *pCurrOp; 
}

byte peekOp()
{
  if(pCurrOp + 1 == loopOp)
    return *buffer;
  return *(pCurrOp+1);
}

void addOp(byte code)
{
  pEndOp++;
  if(pEndOp == loopOp)
    pEndOp = buffer;
  *pEndOp = code; 
}

void clearOps()
{
   pCurrOp = buffer;
   pEndOp = buffer; 
}

// callback for received data
void receiveData(int byteCount)
{
  while(Wire.available())
  {
    switch(Wire.read())
    {
      case OPCODE_STATUS:
        StatusReq = true;
        break;
      case OPCODE_WHEEL_POSTITION:
        addOp(OPCODE_WHEEL_POSTITION);
        addOp(Wire.read());
        break;
      case OPCODE_DRIVE:
        addOp(OPCODE_DRIVE);
        addOp(Wire.read());
        break;
      case OPCODE_RECALIBRATE:
        addOp(OPCODE_RECALIBRATE);
        break;
      case OPCODE_ABORT:
        clearOps();
        addOp(OPCODE_ABORT);
        break;      
    }
  }
}
	 
// callback for sending data
void sendData()
{
  if(StatusReq) {
    byte resp[2] = {OPCODE_STATUS, currStatus};
    Wire.write(resp, 2);
    StatusReq = false;
  }
}
