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
        1 BYTE
          From MSB:
            Angle: 0 - 180 where 0 = full left
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
#define MAX_OPERANDS 2

typedef struct
{
  int Forward;
  int Speed;
} t_DRIVE_INFO;

typedef struct
{
  int Pos;
  int Angle;
  int DestPos;
  int DestAngle;
} t_WHEEL_MOVEMENT;

typedef struct
{  
  int Recalibrating; //this it to force a byte aligned struct, otherwise could have been bool
  t_DRIVE_INFO Drive; 
  t_WHEEL_MOVEMENT Wheel;
} t_STATUS;

// create an instance of the stepper class, specifying
// the number of steps of the motor and the pins it's
// attached to
Stepper stepper(STEPS, 3, 4, 5, 6);

// the previous reading from the analog input
t_STATUS Status;
t_WHEEL_MOVEMENT* pWheel = &Status.Wheel;
t_DRIVE_INFO* pDrive = &Status.Drive;
volatile boolean StatusReq = false;
volatile boolean commandLoaded = false;
byte recBuffer[10];
int recOperands, expOperands;
const boolean EnableSerial = false;
volatile int sentBytes = 0;

void setup()
{
  // set the speed of the motor to 30 RPMs
  stepper.setSpeed(30);
  if(EnableSerial)
  {
    Serial.begin(9600);
    Serial.println(sizeof(t_STATUS));
    Serial.println(sizeof(t_DRIVE_INFO));
    Serial.println(sizeof(t_WHEEL_MOVEMENT));
  }

  //stepper.step(1200);
  pinMode(A0, OUTPUT); // Main drive power
  pinMode(A1, OUTPUT); // Main drive direction
  DDRB = DDRB | B00111111;
  PORTB = 0;
  expOperands = 0;
  digitalWrite(A0, LOW); // Default to off
  digitalWrite(A1, LOW); // Default to forward

  memset(&Status,0,sizeof(Status)); 
       
  Wire.begin(SLAVE_ADDRESS);
  Wire.onReceive(receiveData);
  Wire.onRequest(sendData);
}

void loop()
{
  int step;
  if(sentBytes != 0)
  {
    Serial.println(sentBytes);
    sentBytes = 0;
  }
  
  if(commandLoaded)
  {
    if(EnableSerial)
      Serial.println("Command loaded");
    Execute();
    commandLoaded = false;
  }
  
  if(!Status.Recalibrating)
  {
    if(pWheel->Pos != pWheel->DestPos)
    {    
      step = stepWheel();
      stepper.step(step);
      if(pWheel->Pos == pWheel->DestPos)
        pWheel->DestAngle = pWheel->Angle;    

      if(EnableSerial) PrintStatus();
    }
  }
}

int stepWheel()
{
  if(pWheel->DestPos > pWheel->Pos)
  {
    pWheel->Pos++;
    pWheel->Angle = ceil(pWheel->Pos / 6.666666);
    return 1; 
  }
  else if(pWheel->DestPos < pWheel->Pos)
  {
    pWheel->Pos--;
    pWheel->Angle = ceil(pWheel->Pos / 6.666666);
    return -1; 
  }
}

int calcStep(int angle)
{
  pWheel->DestPos = (angle * 6.666666);
  pWheel->DestAngle = angle;
  return pWheel->DestPos - pWheel->Pos;
}

void recalibrate()
{
  Status.Recalibrating = true;
  if(EnableSerial) Serial.println("Calibrating...");
  //assume starting position is 90
  stepper.step(300); // bring to 45
  stepper.step(-900);  // bring to 180 (all right)
  stepper.step(1200);  // bring to 0 (all left)
  stepper.step(-600);  // bring to 90 (center)  
  pWheel->Pos = 0;
  pWheel->Angle = 90;
  pWheel->DestPos = 0;
  pWheel->DestAngle = 90;
  if(EnableSerial) Serial.println("Calibrated.");
  Status.Recalibrating = false;
}

void Execute()
{
  switch(recBuffer[0])
  {
    case OPCODE_WHEEL_POSITION:
      WheelPositionRequest(recBuffer[1] - 90); //Convert (0 - 180) to (-90 - 90)
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
  pWheel->DestPos = pWheel->Pos;
  pWheel->DestAngle = pWheel->Angle;
}

void DriveRequest(bool forward, int reqSpeed)
{
  digitalWrite(A1, forward?LOW:HIGH);
  pDrive->Forward = forward;
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
  pDrive->Speed = reqSpeed;
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
  /*if(EnableSerial)
  {
    Serial.print("Data Received (");
    Serial.print(byteCount);  
    Serial.println(" byte[s])...");
  }*/
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
  else if(commandLoaded && data[0] != OPCODE_STATUS)
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

  if(recOperands == expOperands && recBuffer[0] != OPCODE_STATUS) commandLoaded = true;
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
      expOperands = 1; //angle (0 - 180 where 0 = full left)
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

void PrintStatus()
{
  Serial.print(Status.Recalibrating);
  Serial.print(" ");
  Serial.print(Status.Wheel.Pos);
  Serial.print(" ");
  Serial.print(Status.Wheel.Angle);
  Serial.print(" ");
  Serial.print(Status.Wheel.DestPos);
  Serial.print(" ");
  Serial.print(Status.Wheel.DestAngle);
  Serial.print(" ");
  Serial.print(Status.Drive.Forward);
  Serial.print(" ");
  Serial.println(Status.Drive.Speed);
}

void LoadOperand(int b)
{
   recBuffer[++recOperands] = b;    
}
	 
// callback for sending data
void sendData()
{  
  if(StatusReq) {
    sentBytes = Wire.write((const char*)&Status, sizeof(Status));
    StatusReq = false;
  }
}
