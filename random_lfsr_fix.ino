
/*
 * Corrected version of random_lfsr.ino.
 * Searches for a value 'x' such that its cyclotomic filter output
 * matches the current state of an LFSR.
 */

// Standard 8-bit LFSR polynomial: x^8 + x^6 + x^5 + x^4 + 1
#define LFSR_POLY 0xB8
uint8_t lfsr_state = 0x01;

// Function to perform one step of the LFSR and return the new state
uint8_t lfsr_step() {
  uint8_t bit = lfsr_state & 1;
  lfsr_state >>= 1;
  if (bit) {
    lfsr_state ^= LFSR_POLY;
  }
  return lfsr_state;
}

// Cyclotomic Polynomial evaluation Phi_16(x) = x^8 + 1
// For simplicity on Arduino, we'll evaluate it modulo 257 (a prime).
// Then map it to uint8_t.
uint8_t cyclo_eval(uint16_t x) {
    // x^8 + 1 mod 257
    uint32_t val = 1;
    for (int i = 0; i < 8; i++) {
        val = (val * x) % 257;
    }
    val = (val + 1) % 257;
    return (uint8_t)(val & 0xFF);
}

// Check if x is a "solution" - i.e., cyclo_eval(x) matches the current LFSR bits
bool is_solution(uint16_t x) {
    uint8_t y = cyclo_eval(x);
    // Let's see if the value y matches the current LFSR bits.
    // In this "pseudorandom" function, a solution is when the
    // filtered value of x equals the state of our target LFSR.
    return (y == lfsr_state);
}

void setup() {
  Serial.begin(9600);
  randomSeed(analogRead(0));
  Serial.println("Searching for a solution to: cyclo_eval(x) == lfsr_state");
}

void loop() {
  uint16_t x = (uint16_t)random(1, 256); // Possible x values mod 257

  if (is_solution(x)) {
    Serial.print("Found a solution! x = ");
    Serial.print(x);
    Serial.print(", cyclo_eval(x) = ");
    Serial.print(cyclo_eval(x), HEX);
    Serial.print(", LFSR_state = ");
    Serial.println(lfsr_state, HEX);

    // Step the LFSR for the next challenge
    lfsr_step();
    Serial.print("Next challenge state: ");
    Serial.println(lfsr_state, HEX);

    // Continue searching for more solutions
    delay(2000);
  }
}
