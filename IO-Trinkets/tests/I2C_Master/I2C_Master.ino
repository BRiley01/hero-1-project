#include <TinyWireM.h>

#define SEND_BYTES 17
#define I2C_SLAVE 0x04

void setup()
{
  TinyWireM.begin(); // join i2c bus (address optional for master)
}

byte x = 0;
char myString[] = "x is ";
boolean canSend = true;

void loop()
{  
  /*if(canSend)
  {*/
    TinyWireM.beginTransmission(4); // transmit to device #4
    for(byte i = 0; i < SEND_BYTES; i++)
    {
      TinyWireM.send(x); 
    }
    TinyWireM.endTransmission();    // stop transmitting
    canSend = false;
    
/*  }
  else
  {*/
    //TinyWireM.requestFrom(I2C_SLAVE, 1);
    if(TinyWireM.available())
    {
      //x = TinyWireM.receive();
      /*if(TinyWireM.read() == SEND_BYTES)
        x++;*/
    }
    delay(1000);
//  }  
}
