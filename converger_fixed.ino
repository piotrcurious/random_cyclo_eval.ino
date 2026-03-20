
/*
 * converger_fixed.ino
 * Corrected version of the signal converger.
 * Uses an 8-bit LFSR and a Cyclotomic FIR filter (Phi_16(z) = z^8 + 1).
 * Searches for the LFSR state that best approximates the input signal.
 */

#define ANALOG_IN A0
#define PWM_OUT 9
#define SAMPLING_FREQ 1000
#define BUFFER_SIZE 128

// Cyclotomic Polynomial Phi_16(z) = z^8 + 1
// Coefficients: [1, 0, 0, 0, 0, 0, 0, 0, 1]
#define CYCLO_DEGREE 8
const int8_t cyclo_coeffs[] = {1, 0, 0, 0, 0, 0, 0, 0, 1};

uint8_t input_buffer[BUFFER_SIZE];
uint8_t best_state = 1;
uint32_t min_error = 0xFFFFFFFF;
uint8_t current_lfsr_state = 1;

// Standard 8-bit LFSR polynomial: x^8 + x^6 + x^5 + x^4 + 1
#define LFSR_POLY 0xB8

uint8_t step_lfsr(uint8_t &state) {
    uint8_t bit = state & 1;
    state >>= 1;
    if (bit) {
        state ^= LFSR_POLY;
    }
    return bit;
}

uint8_t generate_filtered_output(uint8_t seed, int index) {
    // To generate the output at time 'index', we need the last 8 bits of the LFSR sequence
    // This is inefficient but clear for this implementation.
    // In a real system, we'd use a sliding window.
    uint8_t state = seed;
    uint8_t history[9];

    // Fast-forward LFSR to index
    for (int i = 0; i < index; i++) {
        step_lfsr(state);
    }

    // Get bits for filter
    for (int i = 0; i <= CYCLO_DEGREE; i++) {
        history[i] = step_lfsr(state);
    }

    // Apply Phi_16(z) = 1*h[0] + 0*h[1...7] + 1*h[8]
    int val = (int)history[0] + (int)history[8];
    // val is in [0, 2]. Map to [0, 255]
    return (uint8_t)(val * 127);
}

void setup() {
    pinMode(PWM_OUT, OUTPUT);
    Serial.begin(9600);
    randomSeed(analogRead(A1)); // Use an unconnected pin for entropy
}

void loop() {
    static int buf_idx = 0;
    static unsigned long last_sample = 0;

    if (micros() - last_sample >= (1000000 / SAMPLING_FREQ)) {
        last_sample = micros();

        // 1. Read input
        input_buffer[buf_idx] = analogRead(ANALOG_IN) >> 2; // 10-bit to 8-bit

        // 2. Output the best approximation
        uint8_t out_val = generate_filtered_output(best_state, buf_idx);
        analogWrite(PWM_OUT, out_val);

        buf_idx = (buf_idx + 1) % BUFFER_SIZE;

        // 3. Once buffer is full, search for a better state
        if (buf_idx == 0) {
            uint8_t candidate_state = (uint8_t)random(1, 256);
            uint32_t current_candidate_error = 0;

            for (int i = 0; i < BUFFER_SIZE; i++) {
                uint8_t target = input_buffer[i];
                uint8_t output = generate_filtered_output(candidate_state, i);
                int diff = (int)target - (int)output;
                current_candidate_error += (uint32_t)(diff * diff);
            }

            if (current_candidate_error < min_error) {
                min_error = current_candidate_error;
                best_state = candidate_state;
                Serial.print("New best state: ");
                Serial.print(best_state);
                Serial.print(" Error: ");
                Serial.println(min_error);
            }
        }
    }
}
