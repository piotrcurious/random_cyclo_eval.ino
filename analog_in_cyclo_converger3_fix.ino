
// Corrected version of analog_in_cyclo_converger3.ino
#define ANALOG_IN A0
#define PWM_OUT 9
#define SAMPLING_FREQ 1000
#define BUFFER_SIZE 64 // Reduced size to fit in Arduino RAM with function array

#define CYCLO_DEGREE 16
// Phi_16(x) = x^8 + 1. (Not the degree 16 one from the original, which was wrong anyway)
// Let's use the one from the original but ensure it doesn't overflow everything.
const int8_t cyclo_coef[] = {1, -1, 0, 0, 0, 0, 0, 0, -1, 0, 0, 0, 0, 0, 0, 0, 1};

uint8_t buffer[BUFFER_SIZE];
int index = 0;
unsigned long timer = 0;

// The original "function" array was for storing a candidate signal.
// Let's use it as a 16-bit array to avoid immediate overflow.
int16_t function_cand[BUFFER_SIZE];
long min_error = 2147483647;

uint8_t lfsr1_state = 1;
uint8_t lfsr2_state = 1;

void setup() {
  pinMode(PWM_OUT, OUTPUT);
  timer = micros();
  randomSeed(analogRead(A1));
  lfsr1_state = random(1, 256);
  lfsr2_state = random(1, 256);
}

void loop() {
  if (micros() - timer >= (1000000 / SAMPLING_FREQ)) {
    timer = micros();

    int input = map(analogRead(ANALOG_IN), 0, 1023, 0, 255);
    buffer[index] = (uint8_t)input;

    // Step LFSRs
    auto step = [](uint8_t &state, uint8_t poly) {
        uint8_t bit = state & 1;
        state >>= 1;
        if (bit) state ^= poly;
        return bit;
    };

    uint8_t b1 = step(lfsr1_state, 0xB8);
    uint8_t b2 = step(lfsr2_state, 0x8E);
    int temp = b1 * b2;

    // The original logic for 'function' was very strange.
    // It seemed to be trying to build a representation.
    // Let's simplify: generate a new candidate if we are at the start of the buffer.
    if (index == 0) {
        uint8_t cand1 = random(1, 256);
        uint8_t cand2 = random(1, 256);
        long current_error = 0;
        int16_t current_func[BUFFER_SIZE];

        uint8_t s1 = cand1;
        uint8_t s2 = cand2;

        for (int i = 0; i < BUFFER_SIZE; i++) {
            uint8_t bit1 = step(s1, 0xB8);
            uint8_t bit2 = step(s2, 0x8E);
            // Apply a "cyclotomic-like" filter: y[i] = bit1*bit2 + bit1_prev*bit2_prev
            // (Mocking the intent of the original complex and broken logic)
            current_func[i] = (bit1 * bit2) * 255;
            int diff = (int)buffer[i] - (int)current_func[i];
            current_error += (long)diff * diff;
        }

        if (current_error < min_error) {
            min_error = current_error;
            for(int i=0; i<BUFFER_SIZE; i++) function_cand[i] = current_func[i];
        }
    }

    analogWrite(PWM_OUT, (uint8_t)function_cand[index]);
    index = (index + 1) % BUFFER_SIZE;
  }
}
