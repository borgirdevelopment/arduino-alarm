// inputPins[0] --> Start Button
const byte inputPins[] = {2, 9, 10, 11, 12};
const byte inputCount = sizeof(inputPins) / sizeof(inputPins[0]);
const byte alarmSignalPin = 3;
const byte alarmLedPin = 4;
const byte warningLedPin = 13;
const unsigned int securityDelay = 15000;
const unsigned int ledOffTime = 1500;
const unsigned int ledOnTime = 300;
const byte debounceTime = 30;

// state = 0 --> Security off; 
// state = 1 --> Security start timeout; 
// state = 2 --> Security on; 
// state = 3 --> Alarm on; 
volatile byte state = 0; 

unsigned long debounceTimer[] = {0, 0, 0, 0, 0};
bool lastInputReading[] = {HIGH, HIGH, HIGH, HIGH, HIGH};
bool stableState[] = {HIGH, HIGH, HIGH, HIGH, HIGH};
bool warningLedOn = false;
bool alarmLedOn = false;
volatile unsigned long alarmLedCounter = 0;
volatile unsigned long warningLedCounter = 0;
volatile unsigned long securityDelayCounter = 0;

void setup() {
  // put your setup code here, to run once:
  for (byte i = 0; i < inputCount; i++) {
     pinMode(inputPins[i], INPUT_PULLUP);
  }

  pinMode(alarmSignalPin, OUTPUT);
  pinMode(alarmLedPin, OUTPUT);
  pinMode(warningLedPin, OUTPUT);

  digitalWrite(alarmSignalPin, LOW);
  digitalWrite(alarmLedPin, LOW);
  digitalWrite(warningLedPin, LOW);

  cli();
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;

  OCR1A = 249;  // Прерывание каждую 1 мс

  TCCR1B |= (1 << WGM12);              // Сброс при совпадении
  TCCR1B |= (1 << CS11) | (1 << CS10); // Предделитель 64

  TIMSK1 |= (1 << OCIE1A);             // Разрешаем прерывание
  sei();

}

ISR(TIMER1_COMPA_vect) {
if (state == 1) {
  warningLedOn = true;
  digitalWrite(warningLedPin, HIGH);
  securityDelayCounter++;
  if (securityDelayCounter >= securityDelay) {
    state = 2;
    securityDelayCounter = 0;
    warningLedOn = false;
    warningLedCounter = 0;
    digitalWrite(warningLedPin, LOW);
  }
}

if (state == 2 || state == 3) {
    warningLedCounter++;
     if (warningLedOn && warningLedCounter >= ledOnTime) {
    warningLedOn = false;
    warningLedCounter = 0;
    digitalWrite(warningLedPin, LOW);
  }
  else if (!warningLedOn && warningLedCounter >= ledOffTime) {
    warningLedOn = true;
    warningLedCounter = 0;
    digitalWrite(warningLedPin, HIGH);
  }
}

if (state == 3) {
    digitalWrite(alarmSignalPin, HIGH);
    alarmLedCounter++;

     if (alarmLedOn && alarmLedCounter >= ledOnTime) {
    alarmLedOn = false;
    alarmLedCounter = 0;
    digitalWrite(alarmLedPin, LOW);
  }
  else if (!alarmLedOn && alarmLedCounter >= ledOffTime) {
    alarmLedOn = true;
    alarmLedCounter = 0;
    digitalWrite(alarmLedPin, HIGH);
  }
}

if (state == 0) {
  digitalWrite(alarmLedPin, LOW);
  digitalWrite(warningLedPin, LOW);
  digitalWrite(alarmSignalPin, LOW);
  alarmLedCounter = 0;
  warningLedCounter = 0;
  warningLedOn = false;
  alarmLedOn = false;
}
}

void loop() {
  // put your main code here, to run repeatedly:
  for (byte i = 0; i < inputCount; i++) {
    handleInput(inputPins[i], i);
  }
}

void handleInput(byte inputPin, byte inputId) {
  bool newState = digitalRead(inputPin);

  if (newState != lastInputReading[inputId]) {
    debounceTimer[inputId] = millis();
    lastInputReading[inputId] = newState;
  }

  if (millis() - debounceTimer[inputId] >= debounceTime) {
    if (newState != stableState[inputId]) {
      stableState[inputId] = newState;

      if (stableState[inputId] == LOW) {
        if (inputId == 0) {  // Button click handler
          if (state == 0) {
            state = 1;
            securityDelayCounter = 0;
            warningLedCounter = 0;
            warningLedOn = true;
            digitalWrite(warningLedPin, HIGH);
            } else { 
            state = 0;
            securityDelayCounter = 0;
            }
        } else if (state == 2) {  // Alarm on
          state = 3;
        }
      }
  }
}
}