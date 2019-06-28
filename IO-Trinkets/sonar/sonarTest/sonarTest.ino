volatile long gSent, gRecvd;
long lastRecv = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(A0, OUTPUT); // Sonar drive power
  pinMode(2, INPUT);
  pinMode(3, INPUT);
  pinMode(13, OUTPUT);
  digitalWrite(A0, HIGH); // Default to on

  attachInterrupt(digitalPinToInterrupt(2), sonar_receive, RISING );
  attachInterrupt(digitalPinToInterrupt(3), sonar_transmit, RISING ); 
  
}

void loop() {
  if(lastRecv == gRecvd) return;
  lastRecv = gRecvd;
  if(gRecvd > gSent)
    Serial.println(gRecvd - gSent);
}

void sonar_transmit()
{
  gSent = micros();
}

void sonar_receive()
{
  gRecvd = micros();
}

