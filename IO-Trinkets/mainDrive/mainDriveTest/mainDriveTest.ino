#include <Stepper.h>

// change this to the number of steps on your motor
#define STEPS 300

// create an instance of the stepper class, specifying
// the number of steps of the motor and the pins it's
// attached to
Stepper stepper(STEPS, 8, 9, 10, 11);

// the previous reading from the analog input
int previous = 0;
int i = 0;

void setup()
{
  Serial.begin(9600);
  // set the speed of the motor to 30 RPMs
  stepper.setSpeed(30);
  
  stepper.step(1200);
  delay(1000);
  stepper.step(-600);
  delay(1000);
  stepper.step(300);
  delay(1000);
  stepper.step(-600);
  
  delay(1000);
  stepper.step(300);
  delay(1000);
  stepper.step(-600);
  
  //stepper.step(24);
  
  //stepper.step(1000);
}

void loop()
{
  // get the sensor value
  //int val = analogRead(0);
//stepper.step(-1);
//Serial.println(i);
//i++;
  // move a number of steps equal to the change in the
  // sensor reading
  //stepper.step(val - previous);

  // remember the previous value of the sensor
  //previous = val;
}
