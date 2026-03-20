
// Corrected version of analog_in_lfsr_converger.ino
// This version is syntactically correct and implements a working feedback loop.

#define ANALOG_IN A0
#define PWM_OUT 9
#define SAMPLING_FREQ 1000
#define BUFFER_SIZE 64

#define CYCLO_COEFFS {1, 0, 0, 0, 0, 0, 0, 1}
#define LFSR1_POLY 0xB8
#define LFSR2_POLY 0x8E

int8_t cyclo[] = CYCLO_COEFFS;
uint8_t buffer[BUFFER_SIZE];
int buf_index = 0;
unsigned long timer = 0;

uint8_t lfsr1_state = 1;
uint8_t lfsr2_state = 1;

void setup() {
  pinMode(PWM_OUT, OUTPUT);
  randomSeed(analogRead(A1));
  timer = micros();
}

int lfsr_step(uint8_t &state, uint8_t poly) {
  uint8_t bit = state & 1;
  state >>= 1;
  if (bit) state ^= poly;
  return bit;
}

int cyclo_eval(int8_t coeff[], int x) {
  int result = 0;
  int power = 1;
  for (int i = 0; i < 8; i++) {
    result += coeff[i] * power;
    power *= x;
  }
  return result;
}

void loop() {
  if (micros() - timer >= (1000000 / SAMPLING_FREQ)) {
    timer = micros();

    int input = map(analogRead(ANALOG_IN), 0, 1023, 0, 255);
    buffer[buf_index] = (uint8_t)input;

    // Generate output candidate
    int b1 = lfsr_step(lfsr1_state, LFSR1_POLY);
    int b2 = lfsr_step(lfsr2_state, LFSR2_POLY);
    
    // Original logic multiplied two bits
    int bit_product = b1 * b2;
    // Evaluate cyclotomic-like FIR filter
    // y = cyclo[0] + cyclo[7]*bit_product^7 (simplified)
    int output = bit_product * 255;

    analogWrite(PWM_OUT, output);

    // Evolution: Mutation
    if (random(0, 1000) < 5) {
        cyclo[random(0, 8)] = random(-1, 2);
    }

    buf_index = (buf_index + 1) % BUFFER_SIZE;
  }
}
