/*
  - PROTOCOL - 
  First byte received is the opcode:
    0x00: STATUS
      OPERAND: 
        none
      RESPONSE: 
        0x01 - READY
        0x02 - SPEAKING
        0x03 - FAULT (board is considered dead)
    0x01: SAY
      OPERAND:
        BYTE ARRAY TERMINATED BY 0xFF
        2 high order bits used for pitch. Remaining 6 for phoneme
      RESPONSE:
        4 bytes (big-endian - high -> low)
    0x02: ABORT
      OPERAND:
        none
      RESPONSE:
        none
*/        
      
#include <Wire.h>
	 
#define SLAVE_ADDRESS 0x04
#define BUFFER_SIZE 100

byte buffer[BUFFER_SIZE];

volatile int recvBytes;
volatile int sendBytes;
volatile boolean sendRqIRQ;

void setup() 
{
  recvBytes = 0;
  sendRqIRQ = 0;
  sendBytes = 0;
  Serial.begin(9600);
  Serial.println("Loading...");
  
  Wire.begin(SLAVE_ADDRESS);
	 
  // define callbacks for i2c communication
  Wire.onReceive(receiveData);
  Wire.onRequest(sendData);
  
  Serial.println("Board Initialized!");
}
	 
void loop() 
{
  byte OPCODE;
  if(recvBytes != 0) //flagged that there is data
  {
    recvBytes = 0;
    Serial.println("IRQ STRT");
    while(Wire.available()) 
    {
      buffer[sendBytes] = Wire.read();
      Serial.print(buffer[sendBytes++]);
      Serial.print(' ');
    }       
    Serial.println(' ');
    Serial.println("IRQ END");
  }
  
  if(sendRqIRQ != 0)
  {
    Serial.print("SENDING ");
    Serial.println(sendBytes);
    Wire.write(sendBytes);
    sendRqIRQ = 0;
    sendBytes = 0;
    Serial.println("SENT");
  }
}
  
// callback for received data
void receiveData(int byteCount)
{
  if(sendBytes == 0) 
    recvBytes = byteCount;
}
	 
// callback for sending data
void sendData()
{
  sendRqIRQ = 1;
}
