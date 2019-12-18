#include <Wire.h>

#define SLAVE_ADDRESS 0x06

#define OPCODE_ENABLE       0x01
#define OPCODE_DISABLE      0x02

#define UNDEFINED_DISTANCE 0xFFFFFFFF
#define TIMEOUT 100000
const boolean EnableSerial = true;
volatile unsigned long gSent, gRecvd;
volatile boolean gEnabledReq = false;
volatile boolean gEnabled = false;
volatile unsigned long gDistance;

volatile int x = 0;

void AttachInterrupts()
{
  attachInterrupt(digitalPinToInterrupt(2), sonar_receive, RISING );
  attachInterrupt(digitalPinToInterrupt(3), sonar_transmit, RISING );    
}

void DetatchInterrupts()
{
  detachInterrupt(digitalPinToInterrupt(2));
  detachInterrupt(digitalPinToInterrupt(3));  
}

void setup() {
  // put your setup code here, to run once:
  if(EnableSerial)
  {
    Serial.begin(9600);
    Serial.println("Started!");
  }
  pinMode(A0, OUTPUT); // Sonar drive power
  pinMode(2, INPUT);   // Recv timer (goes high when received)
  pinMode(3, INPUT);   // Send timer (goes high when transmitted)
  digitalWrite(A0, gEnabled); // Default to off
  
  AttachInterrupts();

  Wire.begin(SLAVE_ADDRESS);
  Wire.onReceive(receiveData);
  Wire.onRequest(sendData);
  Serial.println("xx");
}

void loop() {
  Serial.println(x);
  unsigned long rcv = gRecvd, snd = gSent;
  if(gEnabledReq != gEnabled)
  {
    if(EnableSerial) Serial.println(gEnabledReq);
    gEnabled = gEnabledReq;
    digitalWrite(A0, gEnabled);
  }
  
  if(rcv > snd)
    gDistance = rcv - snd;
  else if(micros() - snd > TIMEOUT)
    gDistance = UNDEFINED_DISTANCE;
  /*if(EnableSerial)
  {
    if(gDistance == UNDEFINED_DISTANCE)
      Serial.println("UNDEFINED DISTANCE");
    else
      Serial.println(gDistance);
  }*/
}

void sonar_transmit()
{
  gSent = micros();
}

void sonar_receive()
{
  gRecvd = micros();
}

void LoadOpcode(byte opcode)
{    
  switch(opcode)
  {
    case OPCODE_ENABLE:
      gEnabledReq = true;      
      break;
    case OPCODE_DISABLE:
      gEnabledReq = false;
      break;
    default:
      if(EnableSerial) Serial.println("UNEXPECTED OPCODE Received!");
  }
}

void receiveData(int byteCount)
{
  if(Wire.available())
    LoadOpcode(Wire.read());
}

// callback for sending data
void sendData()
{  
  unsigned long d = gDistance;
  if(d != gDistance)
    d = gDistance;
  Wire.write((const char*)&d, sizeof(unsigned long));
}
