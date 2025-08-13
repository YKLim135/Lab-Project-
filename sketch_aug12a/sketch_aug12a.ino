#define TM_CLK 10
#define TM_DIO 11

#define RED_LED 4
#define GREEN_LED 5
#define BLUE_LED 6
#define BUZZER 3
#define KNOB_PIN A0

const int TEMP_MIN = 22;
const int TEMP_MAX = 30;

int temperature = TEMP_MIN;

// Convert digit to segment bits
uint8_t digitToSegmentValue(int digit) {
  switch (digit) {
    case 0: return 0b00111111;
    case 1: return 0b00000110;
    case 2: return 0b01011011;
    case 3: return 0b01001111;
    case 4: return 0b01100110;
    case 5: return 0b01101101;
    case 6: return 0b01111101;
    case 7: return 0b00000111;
    case 8: return 0b01111111;
    case 9: return 0b01101111;
    default: return 0;
  }
}

// TM1637 display functions
void startSignal() {
  pinMode(TM_DIO, OUTPUT);
  digitalWrite(TM_DIO, HIGH);
  digitalWrite(TM_CLK, HIGH);
  delayMicroseconds(2);
  digitalWrite(TM_DIO, LOW);
  delayMicroseconds(2);
  digitalWrite(TM_CLK, LOW);
}

void stopSignal() {
  pinMode(TM_DIO, OUTPUT);
  digitalWrite(TM_CLK, LOW);
  digitalWrite(TM_DIO, LOW);
  delayMicroseconds(2);
  digitalWrite(TM_CLK, HIGH);
  delayMicroseconds(2);
  digitalWrite(TM_DIO, HIGH);
}

void writeByte(uint8_t b) {
  for (int i = 0; i < 8; i++) {
    digitalWrite(TM_CLK, LOW);
    digitalWrite(TM_DIO, (b & 0x01) ? HIGH : LOW);
    delayMicroseconds(3);
    digitalWrite(TM_CLK, HIGH);
    delayMicroseconds(3);
    b >>= 1;
  }
  digitalWrite(TM_CLK, LOW);
  pinMode(TM_DIO, INPUT);
  delayMicroseconds(5);
  while (digitalRead(TM_DIO));
  pinMode(TM_DIO, OUTPUT);
  digitalWrite(TM_CLK, HIGH);
  delayMicroseconds(2);
  digitalWrite(TM_CLK, LOW);
}

void displayNumber(int num) {
  uint8_t segData[4];
  for (int i = 0; i < 4; i++) {
    int digit = num % 10;
    segData[i] = digitToSegmentValue(digit);
    num /= 10;
  }
  startSignal();
  writeByte(0x40);
  stopSignal();

  startSignal();
  writeByte(0xC0);
  for (int i = 0; i < 4; i++) {
    writeByte(segData[3 - i]);
  }
  stopSignal();

  startSignal();
  writeByte(0x8f);
  stopSignal();
}

// Normal beep (moderate volume/frequency)
void normalBeep() {
  tone(BUZZER, 1000); // 1kHz
  delay(200);
  noTone(BUZZER);
  delay(100);
}

// Loud beep (higher frequency/faster)
void loudBeep() {
  tone(BUZZER, 2000); // 2kHz
  delay(100);
  noTone(BUZZER);
  delay(50);
}

// London Bridge melody
void playLondonBridge() {
  int notes[] = {
    262, 294, 330, 262, 330, 349, 330, 294,
    262, 294, 330, 262, 330, 349, 330, 294
  };
  int durations[] = {
    400, 400, 400, 400, 400, 400, 400, 400,
    400, 400, 400, 400, 400, 400, 400, 400
  };
  for (int i = 0; i < 16; i++) {
    tone(BUZZER, notes[i], durations[i]);
    delay(durations[i] * 1.3);
  }
  noTone(BUZZER);
}

void setup() {
  pinMode(TM_CLK, OUTPUT);
  pinMode(TM_DIO, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  int knobValue = analogRead(KNOB_PIN);
  int newTemp = map(knobValue, 0, 1023, TEMP_MIN, TEMP_MAX);

  if (newTemp != temperature) {
    temperature = newTemp;
    displayNumber(temperature);
    Serial.println(temperature); // For Serial Plotter graph
  }

  if (temperature >= 28) {
    // Loud beep, Red LED blink, Blue ON
    digitalWrite(BLUE_LED, HIGH);
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(RED_LED, HIGH);
    delay(200);
    digitalWrite(RED_LED, LOW);
    loudBeep();
  } 
  else if (temperature >= 25 && temperature <= 27) {
    // Normal beep, Red LED blink, Blue ON
    digitalWrite(BLUE_LED, HIGH);
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(RED_LED, HIGH);
    delay(200);
    digitalWrite(RED_LED, LOW);
    normalBeep();
  }
  else if (temperature <= 24) {
    // Green LED ON, London Bridge melody
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(RED_LED, LOW);
    digitalWrite(BLUE_LED, LOW);
    playLondonBridge();
  }
}
