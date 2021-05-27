int i;

void setup()
{
  pinMode(2, OUTPUT);
  pinMode(16, OUTPUT);
}

void loop()
{
  for(i=1023;i>=723;i=i-10)
  {
    analogWrite(16,i);
    delay(100);
  }
  digitalWrite(2, LOW);

  for(i=723;i<1024;i=i+10)
  {
    analogWrite(16,i);
    delay(100);
  }
  digitalWrite(2, LOW);
}
