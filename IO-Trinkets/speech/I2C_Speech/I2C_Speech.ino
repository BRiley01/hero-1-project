#include <Wire.h>
	 
#define STB 5  // Strobe need to go high to latch datas
#define AR 3  // Acknowledge/Request goes high when ready
#define POWER 4
#define PITCH1 A1
#define PITCH2 A2
#define SLAVE_ADDRESS 0x04

byte readyMsg[] = { 0x2B, 0x02, 0x00, 0x1E, 0x29, 0x03, 0xFF };
byte receiveMsg[] = { 0x0B, 0x09, 0x0D, 0xFF };
byte buffer[1024];
boolean speaking, speechReady;
int receivedBytes, lastRecv;

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
  
  // set Port B 6 lowest bit as Output (Arduino Uno pin 8 to 13)
  DDRB = B00111111; 

  // Setup pins for speech
  pinMode(STB, OUTPUT);
  pinMode(POWER, OUTPUT);
  pinMode(AR, INPUT);
  pinMode(PITCH1, OUTPUT);
  pinMode(PITCH2, OUTPUT);
  digitalWrite(STB, LOW);   // must stay low
  digitalWrite(POWER, LOW);
  speaking = false;
  speechReady = false;
  receivedBytes = 0;
	 
  say(readyMsg);
  Serial.println("Speech Board Initialized!");
}
	 
void loop() 
{
  if(speechReady)
  {
    Serial.println("Speech is ready");
    say(buffer);
  }
  delay(100);
}

void say(byte* message)
{
  if(!speaking)
  {
    speaking = true;
    Serial.println("speaking");
    digitalWrite(POWER, HIGH);
    noInterrupts();
    for(int i = 0; message[i] != 0xFF; i++)
      pronounce(message[i]);
    interrupts();
    digitalWrite(POWER, LOW);
    speechReady = false; 
    speaking = false;
  }
}

void pronounce(byte phoneme) 
{
  int pitch = 0;
  digitalWrite(PITCH1, (pitch & B1 != 0)?HIGH:LOW);
  digitalWrite(PITCH2, (pitch>>1 & B1 != 0)?HIGH:LOW);
  PORTB =  phoneme;   // Set Stb = 1 for 2usec to tell the chip to read the Port
  digitalWrite(STB, HIGH);
  delayMicroseconds(2);
  digitalWrite(STB, LOW);
  //  Wait for AR=1 when chip is ready
  while (digitalRead(AR) == 0);
}
	 
// callback for received data
void receiveData(int byteCount)
{
  if(speaking)
  {
    
	Serial.println("receive called while speaking");
    return;
  }  
  
  while(Wire.available()) 
  {
    buffer[receivedBytes] = Wire.read();
    receivedBytes++;
    if(buffer[receivedBytes-1] == 0xFF)
    {
      speechReady = true;
      lastRecv = receivedBytes;
      receivedBytes = 0;
      break;
    }      
  } 
}
	 
// callback for sending data
void sendData(){
  int recv = lastRecv;
  Serial.print("sendData called, returning ");
  Serial.println(recv);
  if(recv == 0)
    Wire.write(0);
  else
  {
    while(recv > 0)
    {
      //should send a byte that tells how many bytes describe the number
      Wire.write((int)recv & 0xFF);
      recv >>= 8; 
    }
  }
  lastRecv = 0;
}
