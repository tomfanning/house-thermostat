void setup() {
  
  attachInterrupt(digitalPinToInterrupt(2), isr, mode);
  attachInterrupt(digitalPinToInterrupt(3), button_isr, mode);

}

float setPoint = 22.0;
float tempNow = 22.5;
float hysteresis = 0.5;
int demand = 0;
int outputState = 0;
unsigned long lastTransition = 0;

void isr(){
  
}

void processDemand() {
  if (outputState != demand) {
    return;
  }

  unsigned long now = millis();
  unsigned long since_last_transition = now - lastTransition;

  // need to consider rollover
  // https://arduino.stackexchange.com/questions/12587/how-can-i-handle-the-millis-rollover
  
  if (since_last_transition < 60000)
    return;
  }

  digitalWrite(CONTROL_PIN, demand);
  outputState = demand;
  lastTransition = now;
}

void loop() {
  
  tempNow = ReadTemperature();
  
  if (tempNow < setPoint - hysteresis){
    demand = 1;
  } else {
    demand = 0;
  }

  processDemand();
}
