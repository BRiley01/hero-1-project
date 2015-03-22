#define STB 5  // Strobe need to go high to latch datas
#define AR 3  // Acknowledge/Request goes high when ready
#define POWER 4
#define PITCH1 A1
#define PITCH2 A2

//byte message[]={0x1B, 0x02, 0x23, 0x18, 0x23, 0x16, 0x37,0x3E,0x2D,0x3A,0x2B,0x18,0x1E,0x3F};
//byte message[]={0x1B, 0x02, 0x23, 0x18, 0x23, 0x16, 0x37,0x3E,0x0E,0x0E,0x2B,0x15,0x00,0x09,0x29,0x0B,0x09,0x0D,0x3F,0x3E,0x2F,0x00,0x0D,0x1E,0x3F,0x3E,0x19,0x2B,0x09,0x1F,0x2A,0x2C,0x0D,0x15,0x3F};
byte message[]={0x2A, 0x02, 0x00, 0x1F, 0x2A, 0x0B, 0x14, 0x3F, 0x2D, 0x32, 0x31, 0x0D, 0x3F, 0x2A, 0x36, 0x37, 0x37, 0x3F, 0x39, 0x2B, 0x3C, 0x29, 0x3F};
//byte message[]={0x1B, 0x02, 0x23, 0x18, 0x23, 0x16, 0x37,0x3E,0x19,0x2B,0x09,0x1F,0x2A,0x2C,0x0D,0x15,0x3F};
int messageSize = sizeof(message);
int pitch;

void setup(){
  DDRB = B00111111; // set Port B 6 lowest bit as Output (Arduino Uno pin 8 to 13)
  pitch = 0;

  pinMode(STB, OUTPUT);
  pinMode(POWER, OUTPUT);
  pinMode(AR, INPUT);
  pinMode(PITCH1, OUTPUT);
  pinMode(PITCH2, OUTPUT);
  digitalWrite(STB, LOW);   // must stay low
  digitalWrite(POWER, HIGH);
}

void loop()  {
  int i;  
  for (i=0; i < messageSize; i++) {
    say(message[i], pitch%4);
  }  
  pitch++;
  delay(2000); // delay 2 sec between repetition
}
void say(byte phoneme, int pitch) {
  digitalWrite(PITCH1, (pitch & B1 != 0)?HIGH:LOW);
  digitalWrite(PITCH2, (pitch>>1 & B1 != 0)?HIGH:LOW);
  
  PORTB =  phoneme;   // Set Stb = 1 for 2usec to tell the chip to read the Port
  digitalWrite(STB, HIGH);
  delayMicroseconds(2);
  digitalWrite(STB, LOW);
  //  Wait for AR=1 when chip is ready
  while (digitalRead(AR) == 0);
}
