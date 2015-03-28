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
        STATUS_SPEECH_READY 0x03
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
#define AR_IRQ 1
#define POWER 4
#define PITCH1 A1
#define PITCH2 A2

#define SLAVE_ADDRESS 0x04

#define RESP_BUFFER_SIZE 5
#define BUFFER_SIZE 1024

#define OPCODE_STATUS 0x00
#define OPCODE_SAY 0x01
#define OPCODE_SPEECH 0x02
#define OPCODE_ABORT 0xFE

#define STATUS_READY 0x01
#define STATUS_LOADING 0x02
#define STATUS_SPEECH_READY 0x03
#define STATUS_SPEAKING 0x04
#define STATUS_WAIT_AR 0x05
#define STATUS_ABORTING 0x06
#define STATUS_FAULT 0xFF


volatile byte buffer[BUFFER_SIZE];
volatile byte respBuffer[RESP_BUFFER_SIZE];
volatile int receivedBytes, sendBytes;
volatile int state;
volatile byte* phoneme_ptr;
volatile byte speechType;
int waitCnt;

const int kSpeechBase = 0xFA4B;

//Historic Hero1 speeches
byte speeches[] = { /*0xFA4B*/  0X16,0X01,0X18,0X26,0X2D,0X3E,0X0C,0X15,0X00,0X3C,0X0D,0X20,0X09,0X0C,0X03,0X0B,0X12,0X03,0X1B,0X2C,0X2B,0X26,0X2D,0X3F,0XFF,
                    /*0xFA64*/  0X16,0X01,0X18,0X26,0X2D,0X3E,0X15,0X00,0X09,0X29,0X2F,0X00,0X0C,0X96,0XBC,0XBC,0XAB,0XB5,0XB7,0X38,0X32,0X23,0X1B,0X3C,0X29,0X39,0X02,0X1E,0X1A,0X36,0X19,0X06,0X29,0X11,0X23,0X0D,0X23,0X18,0X6B,0X75,0X77,0X0E,0X15,0X23,0X2A,0X3F,0XFF,
                    /*0xFA93*/  0X15,0X00,0x09,0X29,0X19,0X2F,0X00,0X0D,0X6A,0X7D,0X59,0X3E,0X18,0X23,0x08,0X29,0X19,0XB8,0X89,0X8A,0X9F,0X3F,0XFF,
                    /*0xFAAA*/  0X15,0X00,0x09,0X29,0X19,0X2F,0x00,0X0D,0XC,0X37,0X37,0XF,0XC,0X15,0x00,0x09,0X29,0X55,0X6B,0X4C,0X3F,0XFF,
                    /*0xFAC0*/  0X15,0x00,0x09,0X29,0X19,0X2F,0x00,0X0D,0X22,0X36,0X37,0X37,0X12,0XC,0X15,0x00,0x09,0X29,0X9C,0XAB,0X8B,0X89,0X65,0X7A,0X3F,0XFF,
                    /*0xFADA*/  0X15,0x00,0x09,0X29,0X19,0X2F,0x00,0XD,0X2A,0X3A,0X2B,0XD,0XC,0X15,0x00,0x09,0X29,0X5B,0X42,0X40,0X5E,0X3F,0XFF,
                    /*0xFAF1*/  0X2F,0x00,0XD,0X1E,0X15,0x00,0x09,0X29,0X19,0X2F,0x00,0XD,0X4C,0X77,0X77,0X4F,0X62,0XE,0X31,0x08,0X37,0X2A,0X3F,0XFF,
                    /*0xFB09*/  0X7D,0X58,0X1F,0X35,0X37,0X3E,0X15,0x00,0x09,0X29,0X19,0X2F,0x00,0XD,0X1F,0x02,0x00,0XD,0X1F,0XD8,0XE3,0XC8,0XE9,0XEA,0X3E,0X9F,0X95,0XA3,0XAD,0X8D,0X9E,0X3E,0X2F,0x00,0X0D,0X1E,0X4C,0X77,0X77,0X4F,0X4C,0X40,0X4D,0X6A,0X3F,0XFF,
                    /*0xFB37*/  0X55,0X40,0X49,0X69,0X1F,0X3C,0X29,0X98,0XA3,0X88,0X89,0XAA,0X3F,0XFF,
                    /*0xFB45*/  0XED,0XD5,0XE3,0XF7,0X4B,0X49,0X6A,0X7F,0X4B,0X49,0X52,0X9E,0X95,0XAB,0X99,0X3F,0XFF,
                    /*0xFB56*/  0XAD,0X86,0XA1,0XA9,0XAA,0X3E,0X1F,0X32,0X23,0X0C,0X39,0X0B,0x09,0X14,0X4C,0X77,0X77,0X4F,0X5E,0X3F,0XFF,
                    /*0xFB6B*/  0X3E,0X3E,0X15,0x00,0x09,0X29,0X9B,0XBA,0XAB,0X9E,0X78,0X6F,0X40,0X6A,0X3F,0XFF,
                    /*0xFB7B*/  0X3E,0X38,0x00,0x05,0x00,0X2B,0XB,0x09,0X12,0X1F,0X32,0X23,0X0C,0X39,0XB,0x09,0X14,0XB,0x09,0XD,0XC,0X15,0x00,0x09,0X29,0X6D,0X46,0X49,0X69,0X3F,0XFF,
                    /*0xFB9A*/  0X1B,0x02,0x00,0X18,0X25,0XC2,0XC0,0XD8,0XE5,0X3E,0X5B,0X42,0X40,0X58,0X65,0X3E,0X3E,0X3E,0X3E,0X32,0X18,0X15,0X2B,0XC,0x03,0XC2,0XCC,0XFA,0XDE,0XDA,0X82,0X80,0X8D,0X9F,0XA9,0x03,0X3E,0X3F,0XFF,
                    /*0xFBC1*/  0X15,0x00,0x09,0X29,0X1B,0X2F,0X0F,0x06,0X21,0X29,0XE,0X2B,0x06,0X21,0X29,0XD,0X3E,0X1E,0X1A,0X32,0X31,0X1F,0X2A,0X18,0X23,0x08,0X29,0X19,0X62,0X76,0X77,0X77,0X1E,0X36,0X37,0X37,0X3E,0X3E,0X3E,0XE,0X32,0X31,0X2A,0X4C,0X55,0X40,0X49,0X69,0X0E,0X2B,0x06,0X21,0X29,0XD,0XB,0x09,0X12,0x06,0X21,0X29,0X19,0X32,0X23,0XC,0X65,0X62,0X76,0X77,0X77,0X6A,0X3A,0X3E,0X3E,0X3E,0X0C,0X15,0x00,0x09,0X29,0X35,0X37,0XD,0X3A,0X25,0X2B,0X35,0X1C,0X2B,0X2F,0x00,0XC,0X12,0XC,0X15,0x00,0x09,0X29,0X19,0X32,0X23,0XC,0X25,0X22,0X36,0X37,0X37,0X2A,0X3A,0X5D,0X74,0X74,0X6B,0XC,0X3C,0X29,0X2F,0x00,0XD,0X1E,0X15,0x00,0x09,0X29,0XBD,0X98,0XAD,0X86,0X89,0XA9,0X92,0X1E,0X36,0X37,0X37,0X2F,0x00,0X12,0X15,0x00,0x09,0X29,0XC,0X25,0X2B,0X35,0X1C,0X2B,0X2F,0x00,0XC,0X1E,0X3F,0XFF,
                    /*0xFC5B*/  0X38,0x00,0x05,0x00,0X2B,0XB,0x09,0X12,0XD,0X35,0X37,0X1F,0X32,0X31,0X2A,0X10,0X39,0XB,0x09,0X14,0X2F,0x00,0X12,0x06,0X21,0X29,0XCE,0XEF,0XC0,0XDE,0X2B,0X35,0X37,0X0E,0X15,0X23,0X2A,0X3E,0X1E,0X1A,0X32,0X31,0X1F,0X2A,0x06,0X21,0X29,0XC,0XB,0x09,0X1F,0XA5,0XAB,0XB5,0X9C,0X2B,0X2F,0x00,0XC,0X1E,0X2D,0X32,0X31,0XD,0X3F,0XFF,
                    /*0xFC9D*/  0X9C,0X95,0XA3,0X91,0X3E,0X15,0x00,0x09,0X29,0X39,0XB,0x09,0X14,0X19,0X15,0x00,0x09,0X29,0XC,0X1E,0X1A,0X32,0X31,0X1F,0X2A,0X32,0XE,0X31,0x08,0X37,0X2A,0XA5,0XBA,0X5D,0X42,0X40,0X59,0X6A,0X3F,0XFF,
                    /*0xFCC5*/  0X55,0X40,0X49,0X69,0X39,0XB,0x09,0X14,0X19,0X22,0X36,0X37,0X37,0X15,0X23,0X2B,0X99,0XA2,0XB6,0XB7,0XB7,0XEA,0X3E,0X3E,0X3E,0X5C,0X4B,0X49,0X4F,0X0C,0X3C,0X29,0x06,0X21,0X29,0X5B,0X72,0X71,0X5C,0X3E,0X6B,0X75,0X77,0X0E,0X15,0X23,0X2A,0X1F,0XD,0X3C,0X29,0X1E,0X18,0X32,0X23,0XF,0X6A,0X77,0X77,0X77,0X7E,0X3F,0XFF,
                    /*0xFD04*/  0X22,0X36,0X37,0X37,0X15,0X23,0X2B,0XF,0x02,0X2B,0X29,0X72,0X6A,0X6B,0X6F,0X40,0X59,0X6A,0X4B,0X4F,0X3E,0X3E,0X3E,0X1D,0X34,0X34,0X2B,0x06,0X21,0X29,0X5B,0X62,0X76,0X77,0X77,0XC,0x01,0XD,0X3F,0XFF,
                    /*0xFD2C*/  0X22,0x00,0x02,0X1F,0X9F,0XBA,0XAB,0X3E,0X22,0X36,0X37,0X37,0X95,0XA3,0XAB,0X5B,0X6F,0X40,0X4D,0X5E,0X1F,0X32,0X23,0XC,0X3F,0XFF,
                    /*0xFD46*/  0X29,0X34,0X34,0X2B,0X6D,0X4B,0X49,0X51,0XB,0x09,0X12,0XC,0X15,0x00,0x09,0X29,0X19,0X32,0X23,0X4C,0X6F,0X40,0X4D,0X5E,0X3F,0XFF,
                    /*0xFD60*/  0XB5,0XB7,0XB7,0X3F,0XCC,0XD5,0XC0,0XC9,0XE9,0XFE,0XFE,0XE5,0XD8,0XFC,0XE1,0XD2,0X5E,0X76,0X77,0X77,0X4D,0X55,0X63,0X6A,0X5E,0X76,0X77,0X77,0x01,0x01,0XD,0X29,0X39,0XB,0x09,0X14,0X2A,0X36,0X37,0X9B,0XBA,0XAB,0XAA,0X3F,0X4C,0X7C,0X69,0X3F,0XFF,
                    /*0xFD91*/  0X25,0X18,0X3C,0X21,0X12,0XE,0XC3,0X29,0X19,0X2D,0X15,0x00,0X21,0XA,0X2A,0X3E,0X3E,0X15,0x00,0x09,0X29,0XC,0X2A,0X2B,0X15,0x00,0x09,0X29,0XB,0x09,0X14,0X2A,0X36,0X37,0X1F,0X18,0X3C,0X29,0X25,0X3F,0XFF,
                    /*0xFDBA*/  0X15,0x00,0x09,0X29,0X39,0XB,0x09,0X14,0X19,0X15,0x00,0x09,0X29,0XC,0x06,0X21,0X29,0X19,0X2F,0x00,0XD,0X42,0X40,0X59,0X1F,0x02,0x00,0X18,0x02,0x00,0XD,0X2A,0X25,0x02,0x00,0X2A,0X3E,0X3E,0X55,0X40,0X49,0X69,0X4C,0X29,0XF,0X4A,0X4D,0X5B,0X63,0X48,0X77,0X5F,0X2A,0X2B,0x06,0X21,0X29,0XD,0X1E,0X3F,0XFF,
                    /*0xFDF7*/  0X25,0X3C,0X21,0X25,0X23,0X18,0X1F,0X2A,0x00,0x00,0X3A,0X2F,0x00,0X2A,0XC,0X3C,0X29,0X32,0X31,0X18,0X15,0X23,0X2A,0X3E,0X3E,0X15,0x00,0x09,0X29,0X1F,0X32,0X31,0X25,0X35,0X37,0X12,0XB,0x09,0X2A,0X1F,0XE,0X29,0X19,0X3D,0X12,0X15,0x00,0x09,0X29,0XC,0X1F,0X35,0X37,0X11,0X34,0X34,0X2B,0X2A,0X3F,0XFF,
                    /*0xFE33*/  0X2D,0X35,0X34,0X2B,0XD,0XB,0x09,0X14,0X3E,0X3E,0X3E,0X2D,0X35,0X34,0X2B,0XD,0XB,0x09,0X14,0XB,0x09,0XD,0X2A,0X2B,0X36,0X37,0X37,0X1E,0X3A,0X3E,0X3E,0X3E,0X15,0x00,0x09,0X29,0X1B,0X2F,0x00,0XF,0X1F,0X32,0X31,0XC,0X32,0XD,0X1E,0X38,0X32,0X23,0X25,0X32,0X18,0X21,0X29,0X1F,0X3F,0XFF,
                    /*0xFE6D*/  0X35,0X37,0XD,0X35,0X37,0X3E,0X15,0x00,0x09,0X29,0X1E,0X36,0X37,0X37,0XD,0X15,0X23,0X2A,0X1E,0X36,0X37,0X37,0X2D,0XB,0XD,0X1E,0X35,0X37,0X12,0X3F,0XFF,
                    /*0xFE8C*/  0X15,0x00,0x09,0X29,0X2F,0x00,0XC,0XB,0x09,0XD,0X19,0x06,0x09,0X29,0X25,0X23,0XE,0X23,0X18,0X32,0X23,0XF,0XC,0x06,0X21,0X29,0X19,0XB,0X14,0x06,0X21,0X29,0XC,0XB,0x09,0X1F,0X2A,0x06,0X21,0X29,0X19,0X3E,0X3E,0X3E,0X38,0x00,0x05,0x00,0X2B,0X1D,0X34,0X34,0X2B,0X3E,0X22,0X36,0X37,0X37,0XC,0X32,0X31,0X1F,0X2A,0XE,0X3C,0X29,0X2B,0X3D,0X14,0X3F,0XFF,
                    /*0xF4A1*/  0X2B,0X3B,0X1E,0X2C,0x03,0XFF,
                    /*0xF2CC*/  0X18,0X26,0X2D,0X3F,0XF,0X26,0X18,0X2A,0X27,0X2A,0X10,0X3E,0XFF };
                      
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
  
  //Attach interrupt for A/R
  attachInterrupt(AR_IRQ, AR_HIGH, RISING);
  
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
  {
    Serial.print(state);
    Serial.print("-");
    Serial.println(speechType);
    /*Serial.print("-");
    Serial.print(waitCnt);
    Serial.print("-");
    Serial.println(*phoneme_ptr);*/
  }
  switch(state)
  {
    case STATUS_ABORTING:
      digitalWrite(POWER, LOW);
      state = STATUS_READY;
      break;
    case STATUS_READY:
    case STATUS_LOADING:
      //delay(100);
      break;
    case STATUS_FAULT:
      digitalWrite(POWER, LOW);
      break;
    case STATUS_SPEECH_READY:
      digitalWrite(POWER, HIGH);
      state = STATUS_SPEAKING;
      break;
    case STATUS_WAIT_AR:
      //  Wait for AR=1 when chip is ready
      if(digitalRead(AR) != 0)
        AR_HIGH();
      else
      {
        if(waitCnt <= 100)
          waitCnt++;  
        if(waitCnt >= 100) 
          state = STATUS_FAULT;     
      }
      break;
    case STATUS_SPEAKING: 
      if(*phoneme_ptr == 0xFF)
      {
        state = STATUS_READY; 
        digitalWrite(POWER, LOW);
      }
      else
      {
        pronounce(*phoneme_ptr);
        phoneme_ptr++;
      } 
  }
  //delay(100);
  delay(20);
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

void AR_HIGH()
{
  if(state == STATUS_WAIT_AR)
    state = STATUS_SPEAKING;
}
	 
// callback for received data
void receiveData(int byteCount)
{
  while(Wire.available())
  {
    if(state == STATUS_LOADING)
    {      
      buffer[receivedBytes] = Wire.read();
      receivedBytes++;
      if(speechType == OPCODE_SAY)
      {      
        if(receivedBytes == BUFFER_SIZE) //prevent buffer overrun
          buffer[receivedBytes-1] = 0xFF;
        if(buffer[receivedBytes-1] == 0xFF)
        {
          phoneme_ptr = buffer;
          state = STATUS_SPEECH_READY;        
          sendBytes = 5;
          respBuffer[0] = OPCODE_SAY;        
          SplitInt(receivedBytes, &respBuffer[1]);
          break;
        }     
      }
      else if(speechType == OPCODE_SPEECH)
      {
        if(receivedBytes == 2) //got speech offset, now load preprogrammed speech
        {
          int offset = kSpeechBase - ((buffer[0] << 8) | (buffer[1]));
          phoneme_ptr = &(speeches[offset]);
          state = STATUS_SPEECH_READY;    
        }
      }        
    }
    else
    {
      //not in process of getting bytes to speak, so waiting on next opcode      
      byte data = Wire.read();
      switch(data)
      {
        case OPCODE_STATUS:
          respBuffer[0] = OPCODE_STATUS;
          respBuffer[1] = state;
          sendBytes = 2;
          break;        
        case OPCODE_SPEECH:
        case OPCODE_SAY:
          speechType = data;
          if(state != STATUS_SPEECH_READY)
          {
            state = STATUS_LOADING;
            receivedBytes = 0; 
          }
          else
          {
            respBuffer[0] = speechType;
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
    
    if(state == STATUS_WAIT_AR)
    {
      //  Wait for AR=1 when chip is ready
      if(digitalRead(AR) != 0)
        AR_HIGH();
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
