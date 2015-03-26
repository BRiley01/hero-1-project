/*
  - PROTOCOL - 
  All Responses first byte is OPCODE that it is responding to
  First byte received is the opcode:
    0x00: STATUS
      OPERAND: 
        none
      RESPONSE: 
        STATUS_READY 0x01
        STATUS_LOADING 0x02
        STATUS_SPEACH_READY 0x03
        STATUS_SPEAKING 0x04
        STATUS_WAIT_AR 0x05
        STATUS_ABORTING 0x06
        STATUS_FAULT 0xFF
    0x01: SAY
      OPERAND:
        BYTE ARRAY TERMINATED BY 0xFF
        2 high order bits used for pitch. Remaining 6 for phoneme
      RESPONSE:
        4 bytes (big-endian - high -> low)
    0xFE: ABORT
      OPERAND:
        none
      RESPONSE:
        none
*/        
      
#include <Wire.h>
	 
#define STB 5  // Strobe need to go high to latch datas
#define AR 3  // Acknowledge/Request goes high when ready
#define POWER 4
#define PITCH1 A1
#define PITCH2 A2

#define SLAVE_ADDRESS 0x04

#define RESP_BUFFER_SIZE 5
#define BUFFER_SIZE 1024

#define OPCODE_STATUS 0x00
#define OPCODE_SAY 0x01
#define OPCODE_ABORT 0xFE

#define STATUS_READY 0x01
#define STATUS_LOADING 0x02
#define STATUS_SPEACH_READY 0x03
#define STATUS_SPEAKING 0x04
#define STATUS_WAIT_AR 0x05
#define STATUS_ABORTING 0x06
#define STATUS_FAULT 0xFF

volatile byte buffer[BUFFER_SIZE];
volatile byte respBuffer[RESP_BUFFER_SIZE];
volatile int receivedBytes, sendBytes;
volatile int state;
int speechIndex;
int waitCnt;

void setup() 
{
  Serial.begin(9600);
  Serial.println("Speech Board Loading...");
  
  pinMode(13, OUTPUT);
  // initialize i2c as slave
  Wire.begin(SLAVE_ADDRESS);
	 
  // define callbacks for i2c communication
  Wire.onReceive(receiveData);
  Wire.onRequest(sendData);
  
  // set Port B 6 lowest bit Output (Arduino Uno pin 8 to 13)
  DDRB = B00111111; 

  // Setup pins for speech
  pinMode(STB, OUTPUT);
  pinMode(POWER, OUTPUT);
  pinMode(AR, INPUT);
  pinMode(PITCH1, OUTPUT);
  pinMode(PITCH2, OUTPUT);
  digitalWrite(STB, LOW);   // must stay low
  digitalWrite(POWER, LOW);
  receivedBytes = 0;
  state = STATUS_READY;
}
	 
void loop() 
{
  if(state != STATUS_READY)
    Serial.println(state);
  switch(state)
  {
    case STATUS_ABORTING:
      state = STATUS_READY;
      break;
    case STATUS_READY:
    case STATUS_LOADING:
      delay(100);
      break;
    case STATUS_SPEACH_READY:
      digitalWrite(POWER, HIGH);
      speechIndex = 0;
      state = STATUS_SPEAKING;
      break;
    case STATUS_WAIT_AR:
      //  Wait for AR=1 when chip is ready
      if(digitalRead(AR) == 0 && waitCnt <= 1000)
      {
        delay(100);
        waitCnt++;  
      }
      else
        state = STATUS_SPEAKING;
      if(waitCnt >= 1000)
        state = STATUS_FAULT;     
      break;
    case STATUS_SPEAKING: 
      if(buffer[speechIndex] == 0xFF)
      {
        state = STATUS_READY; 
        digitalWrite(POWER, LOW);
      }
      else
      {
        noInterrupts();
        pronounce(buffer[speechIndex]);
        interrupts();
      } 
  }
  delay(100);
}

void pronounce(byte phoneme) 
{
  int i = 0;
  digitalWrite(PITCH1, (phoneme>>7 & B1 != 0)?HIGH:LOW);
  digitalWrite(PITCH2, (phoneme>>6 & B1 != 0)?HIGH:LOW);
  PORTB =  phoneme;   //  Stb = 1 for 2usec to tell the chip to read the Port
  digitalWrite(STB, HIGH);
  delayMicroseconds(2);
  digitalWrite(STB, LOW);
  waitCnt = 0;
  state = STATUS_WAIT_AR;
}

void SplitInt(int val, volatile byte* out)
{
  out[0] = (val >> 24) & 0xFF;
  out[1] = (val >> 16) & 0xFF;
  out[2] = (val >> 8) & 0xFF;
  out[3] = val & 0xFF;
}
	 
// callback for received data
void receiveData(int byteCount)
{
  while(Wire.available())
  {
    if(state != STATUS_LOADING) 
    {
      //not in process of getting bytes to speak, so waiting on next opcode      
      switch(Wire.read())
      {
        case OPCODE_STATUS:
          respBuffer[0] = OPCODE_STATUS;
          respBuffer[1] = state;
          sendBytes = 2;
          break;        
        case OPCODE_SAY:
          if(state != STATUS_SPEACH_READY)
          {
            state = STATUS_LOADING;
            receivedBytes = 0; 
          }
          else
          {
            respBuffer[0] = OPCODE_SAY;
            SplitInt(0, &respBuffer[1]);
            sendBytes = 2;
          }
          break;
        case OPCODE_ABORT:
          state = STATUS_ABORTING;  
          respBuffer[0] = OPCODE_ABORT;    
          sendBytes = 1;
      }
    }
    else if(state == STATUS_LOADING)
    {      
      buffer[receivedBytes] = Wire.read();
      receivedBytes++;
      if(receivedBytes == BUFFER_SIZE) //prevent buffer overrun
        buffer[receivedBytes-1] = 0xFF;
      if(buffer[receivedBytes-1] == 0xFF)
      {
        state = STATUS_SPEACH_READY;        
        sendBytes = 5;
        respBuffer[0] = OPCODE_SAY;        
        SplitInt(receivedBytes, &respBuffer[1]);
        break;
      }     
    }
  }
}
	 
// callback for sending data
void sendData()
{
  byte sendBuf[sendBytes];
  int i;
  if(sendBytes > 0)
  {
    for(i=0; i < sendBytes; i++)
      sendBuf[i] = respBuffer[i];  
    Wire.write(sendBuf, i+1);
    sendBytes = 0; 
  }
}
