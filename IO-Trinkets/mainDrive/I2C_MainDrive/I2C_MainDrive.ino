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
    0x01: WHEEL POSITION
      OPERAND:
        2 BYTES
          From MSB:
            Direction (0:left; 1:right)
            Angle: 0 - 90 where 0 = straight
    0x02: DRIVE
      OPERAND:
        2 BYTES
         From MSB: 
          Direction (byte)
          Speed (byte)
         
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
#define OPCODE_WHEEL_POSITION 0x01
#define OPCODE_DRIVE 0x02
#define OPCODE_RECALIBRATE 0x03
#define OPCODE_ABORT 0xFE

// change this to the number of steps on your motor
#define STEPS 300
#define NORECAL -1
#define MAX_OPERANDS 2

enum STATUS {READY = 0x01, MOVING = 0x02, RECALIBRATING = 0x03, FAULT = 0xFF};

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

// the previous reading from the analog input
t_WHEEL_MOVEMENT Wheel;
volatile STATUS currStatus = READY;
volatile boolean StatusReq = false;
volatile boolean commandLoaded = false;
byte recBuffer[10];
int recOperands, expOperands;
int recalibrateStep = NORECAL;
const boolean EnableSerial = false;

void setup()
{
  // set the speed of the motor to 30 RPMs
  stepper.setSpeed(30);
  if(EnableSerial)
    Serial.begin(9600);
  
  //stepper.step(1200);
  pinMode(A0, OUTPUT); // Main drive power
  pinMode(A1, OUTPUT); // Main drive direction
  DDRB = DDRB | B00111111;
  PORTB = 0;
  expOperands = 0;
  digitalWrite(A0, LOW); // Default to off
  digitalWrite(A1, LOW); // Default to forward
     
  Wire.begin(SLAVE_ADDRESS);
  Wire.onReceive(receiveData);
  Wire.onRequest(sendData);
}

void loop()
{
  int step;
  if(commandLoaded)
  {
    if(EnableSerial)
      Serial.println("Command loaded");
    Execute();
    commandLoaded = false;
  }
  
  if(recalibrateStep == NORECAL)
  {
    if(Wheel.Pos != Wheel.DestPos)
    {    
      step = stepWheel();
      stepper.step(step);
      if(Wheel.Pos == Wheel.DestAngle)
        Wheel.DestAngle = Wheel.Angle;    
    }
  }
}

int stepWheel()
{
  if(Wheel.DestPos > Wheel.Pos)
  {
    Wheel.Pos++;
    Wheel.Angle = Wheel.Pos / 6.666666;
    return 1; 
  }
  else if(Wheel.DestPos < Wheel.Pos)
  {
    Wheel.Pos--;
    Wheel.Angle = Wheel.Pos / 6.666666;
    return -1; 
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
  recalibrateStep = 1;
  if(EnableSerial) Serial.println("Calibrating...");
  //assume starting position is 90
  stepper.step(300); // bring to 45
  stepper.step(-900);  // bring to 180 (all right)
  stepper.step(1200);  // bring to 0 (all left)
  stepper.step(-600);  // bring to 90 (center)  
  Wheel.Pos = 0;
  Wheel.Angle = 90;
  Wheel.DestPos = 0;
  Wheel.DestAngle = 90;
  if(EnableSerial) Serial.println("Calibrated.");
  recalibrateStep = NORECAL;
}

void Execute()
{
  int multiplier;
  switch(recBuffer[0])
  {
    case OPCODE_WHEEL_POSITION:
      multiplier = (recBuffer[1]==0)?-1:1;
      WheelPositionRequest(multiplier * recBuffer[2]);
      break;    
    case OPCODE_DRIVE:
      DriveRequest(recBuffer[1],recBuffer[2]);
      break;
    case OPCODE_RECALIBRATE:
      recalibrate();
      break;
    case OPCODE_ABORT:
      Abort();
      break;
  }
}

void Abort()
{
  digitalWrite(A0, LOW);
  Wheel.DestPos = Wheel.Pos;
  Wheel.DestAngle = Wheel.Angle;
}

void DriveRequest(bool forward, int reqSpeed)
{
  digitalWrite(A1, forward?LOW:HIGH);
  if(EnableSerial) Serial.print("Drive: " );
  if(reqSpeed == 0)
  {
    if(EnableSerial) Serial.println("STOP");
    digitalWrite(A0, LOW);
  }
  else 
  {
    if(EnableSerial)
    {
      if(forward)
        Serial.print("FORWARD - ");
      else
        Serial.print("REVERSE - ");
      Serial.println(reqSpeed);
    }
    digitalWrite(A0, HIGH);
  }
}

void WheelPositionRequest(int angle)
{
  if(EnableSerial)
  {
    Serial.print("\t\tPosition Req: ");
    Serial.println(angle);
  }
  calcStep(angle);
}

// callback for received data
void receiveData(int byteCount)
{
  byte b;
  byte data[1 + MAX_OPERANDS]; //opp + operands
  boolean abortReq = false;
  int i = 0;
  if(EnableSerial)
  {
    Serial.print("Data Received (");
    Serial.print(byteCount);  
    Serial.println(" byte[s])...");
  }
  while(Wire.available())
  {    
    b = Wire.read();
    if(i <= MAX_OPERANDS)
      data[i] = b;
    ++i;
  }
  if(i > 1 + MAX_OPERANDS)
  {
    if(EnableSerial) Serial.println("unexpected packet");
    return;
  }
  else if(commandLoaded)
  {
    if(EnableSerial) Serial.println("currently processing command");
    return;
  }
  
  LoadOpcode(data[0]);
  if(i-1 == expOperands) //Account for opcode
  { 
    for(i=1; i<=expOperands; i++)
    {
      LoadOperand(data[i]);
    }
  }
  else if(EnableSerial)
  {
    Serial.print("unexpected operand coun - exp:");
    Serial.print(expOperands);
    Serial.print("; recv:");
    Serial.print(i+1);
  }

  if(recOperands == expOperands) commandLoaded = true;
}

void LoadOpcode(int b)
{    
  recOperands = 0;
  recBuffer[0] = b;
  expOperands = 0;
  switch(recBuffer[0])
  {
    case OPCODE_STATUS:
      StatusReq = true;
      expOperands = 0;
      break;
    case OPCODE_WHEEL_POSITION:
      if(EnableSerial) Serial.println("\tWheel Position...");
      expOperands = 2; //direction(0:left;1:right), angle
      break;
    case OPCODE_DRIVE:        
      expOperands = 2; //reverse(0,1), speed
      break;
    case OPCODE_RECALIBRATE:
      break;
    case OPCODE_ABORT:
      digitalWrite(A0, LOW); //Stop main drive as soon as we get this
      break;
    default:
      if(EnableSerial) Serial.println("UNEXPECTED OPCODE Received!");
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
