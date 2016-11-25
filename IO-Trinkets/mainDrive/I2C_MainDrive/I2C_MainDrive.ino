/*
  - PROTOCOL - 
  All Responses first byte is OPCODE that it is responding to
  First byte received is the opcode:
    0x00: STATUS
      OPERAND: 
        none
      RESPONSE: 
        STATUS_READY 0x01
        STATUS_MOVING_NONBLOCKED 0x02
        STATUS_MOVING 0xF2
        STATUS_RECALIBRATING 0xF3
        STATUS_FAULT 0xFF
    0x01/0xF1 (blocking/non-blocking): WHEEL POSITION
      OPERAND:
        0 - 180 where 90 = straight
    0x02/0xF2 (blocking/non-blocking): DRIVE
      OPERAND:
        4 BYTES
         first:
               LS 6 bits = speed
               bit 7 - direction (0 forward, 1 reverse)
         second:
               duration most significant byte
         third:
               duration least significant byte
         fourth:
               check byte (XOR of first 3 bytes)
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

#define NONBLOCKING_MASK 0xF0
#define OPCODE_STATUS 0x00
#define OPCODE_WHEEL_POSTITION 0x01
#define OPCODE_DRIVE 0x02
#define OPCODE_RECALIBRATE 0x03
#define OPCODE_ABORT 0xFE

#define BUFFER_SIZE 255

// change this to the number of steps on your motor
#define STEPS 300

#define NOOPS 0 //No expected operands

enum STATUS {READY = 0x01, MOVING_NONBLOCKED = 0x02, MOVING = 0xF2, RECALIBRATING = 0x03, FAULT = 0xFF};

void clearOps();
void addOp(byte code);

typedef struct
{
  int Pos;
  int Angle;
  int DestPos;
  int DestAngle;
} t_WHEEL_MOVEMENT;

// create an instance of the stepper class, specifying
// the number of steps of the motor and the pins it's
// attached to
Stepper stepper(STEPS, 3, 4, 5, 6);

void recalibrate();

// the previous reading from the analog input
int previous = 0;
int i = 0;
int newSpeed = 0;
boolean nextOp;
t_WHEEL_MOVEMENT Wheel;
const int maxSteps = 1200; 
volatile STATUS currStatus = READY;
volatile boolean StatusReq = false;
volatile byte buffer[BUFFER_SIZE];
volatile byte* loopOp = &buffer[BUFFER_SIZE - 1];
volatile byte* pCurrOp = buffer;
volatile byte* pEndOp = buffer;
byte recBuffer[10];
int recOperands, expOperands;

void setup()
{
  // set the speed of the motor to 30 RPMs
  stepper.setSpeed(30);
  Serial.begin(9600);
  
  //stepper.step(1200);
  pinMode(A0, OUTPUT); 
  DDRB = B00111111;
  PORTB = 0;
  nextOp = true;
  expOperands = NOOPS;
  //addOp(OPCODE_RECALIBRATE);
   
  Wire.begin(SLAVE_ADDRESS);
  Wire.onReceive(receiveData);
  Wire.onRequest(sendData);
}

void loop()
{
  byte op;
  int steps;
  int multiplier;
  if(!hasOp()) return;
  if(peekOp() == OPCODE_ABORT || nextOp)  
  op = getOp();
  switch(op)
  {
    case OPCODE_ABORT:
      PORTB = 0;
      return;    
    case OPCODE_RECALIBRATE:
      recalibrate();
      break;    
    case OPCODE_WHEEL_POSTITION:
      nextOp = false;
    case OPCODE_WHEEL_POSTITION | NONBLOCKING_MASK:
      //recalibrate();
      multiplier = (getOp()==0)?-1:1;
      op = getOp();
      Serial.print("\t\tPosition Req: ");
      Serial.println(op);
      steps = calcStep(multiplier * op);  
      stepper.step(steps);
      Wheel.Pos = Wheel.DestPos;
      Wheel.Angle = Wheel.DestAngle;
      nextOp = true;
      break; 
  } 
}

int calcStep(int angle)
{
  Wheel.DestPos = (angle * 6.666666);
  Wheel.DestAngle = angle;
  return Wheel.DestPos - Wheel.Pos;
}

void recalibrate()
{
  Serial.println("Calibrating...");
  //assume starting position is 90
  stepper.step(300); // bring to 45
  stepper.step(-900);  // bring to 180 (all right)
  stepper.step(1200);  // bring to 0 (all left)
  stepper.step(-600);  // bring to 90 (center)  
  Wheel.Pos = 0;
  Wheel.Angle = 90;
  Wheel.DestPos = 0;
  Wheel.DestAngle = 90;
  Serial.println("Calibrated.");
}

boolean hasOp()
{
   return pCurrOp != pEndOp;
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

void addOp(const byte code)
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
   *pCurrOp = OPCODE_ABORT;
}

// callback for received data
void receiveData(int byteCount)
{
  byte a[4];
  byte chk;
  byte b;
  int i;
  boolean abort;
  Serial.print("Data Received (");
  Serial.print(byteCount);  
  Serial.println(" byte[s])...");
  while(Wire.available())
  {
    abort = false;
    b = Wire.read();
    Serial.println(b);
    if (recOperands == expOperands || expOperands == NOOPS)
      LoadOpcode(b);
    else
      LoadOperand(b);
    
    if(recOperands >= expOperands || expOperands == NOOPS)
    {
      PushBuffer(expOperands);
      expOperands = NOOPS;
    }
  }
}

void PushBuffer(int length)
{
  Serial.print("Pushing op string: ");
  for(int i = 0; i<=length; i++)
  {
    Serial.print(recBuffer[i]);
    Serial.print(" ");
    addOp(recBuffer[i]); 
  }
  Serial.println();
}

void LoadOpcode(int b)
{    
  recOperands = 0;
  recBuffer[0] = b;
  expOperands = NOOPS;
  switch(recBuffer[0])
  {
    case OPCODE_STATUS:
      StatusReq = true;
      expOperands = NOOPS;
      break;
    case OPCODE_WHEEL_POSTITION:
      Serial.println("\tWheel Position...");
      expOperands = 2; //direction(0:left;1:right), angle
      break;
    case OPCODE_DRIVE:
    case OPCODE_DRIVE | NONBLOCKING_MASK:        
      expOperands = 2; //speed, duration
      break;
    case OPCODE_RECALIBRATE:
    case OPCODE_RECALIBRATE | NONBLOCKING_MASK:
      break;
    case OPCODE_ABORT:
      clearOps();
      break;
    default:
      Serial.println("UNEXPECTED OPCODE Received!");
  }
}

void LoadOperand(int b)
{
   recBuffer[++recOperands] = b;    
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
