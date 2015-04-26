#include <Wire.h>
#include <Stepper.h>

#define SLAVE_ADDRESS 0x05

// change this to the number of steps on your motor
#define STEPS 300

// create an instance of the stepper class, specifying
// the number of steps of the motor and the pins it's
// attached to
Stepper stepper(STEPS, 3, 4, 5, 6);

// the previous reading from the analog input
int previous = 0;
int i = 0;
int newSpeed = 0;

void setup()
{
  Serial.begin(9600);
  // set the speed of the motor to 30 RPMs
  stepper.setSpeed(30);
  
  //stepper.step(1200);
  pinMode(A0, OUTPUT); 
  DDRB = B00111111;
  PORTB = 0;
  //digitalWrite(A0, HIGH);
  PORTB = B00111111;
   
  Wire.begin(SLAVE_ADDRESS);
  /*Wire.onReceive(receiveData);
  Wire.onRequest(sendData);*/
}

void loop()
{
  /*PORTB = newSpeed;
  newSpeed++;
  newSpeed %= 64;
  delay(1000);*/
  /*stepper.step(600);
  delay(1000);
  digitalWrite(A0, HIGH);
  stepper.step(-600);  
  delay(1000);
  digitalWrite(A0, LOW);*/
}
