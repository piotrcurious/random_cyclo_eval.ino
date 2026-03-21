
#include <SD.h>
#define CS_PIN 10
#define ANALOG_IN A0
#define PWM_OUT 9
#define SAMPLING_FREQ 1000
#define BUFFER_SIZE 64 // Reduced from 1000 to fit in RAM

#define CYCLO_DEGREE 16
const int8_t cyclo_coef[CYCLO_DEGREE + 1] = {1, -1, 0, 0, 0, 0, 0, 0, -1, 0, 0, 0, 0, 0, 0, 0, 1};

#define LFSR_DEGREE 8
const uint8_t lfsr1_poly = 0xB8;
const uint8_t lfsr2_poly = 0x8E;

uint8_t buffer[BUFFER_SIZE];
int buf_index = 0;
unsigned long timer = 0;

// The original "function" array was for storing a candidate signal.
int16_t function_cand[BUFFER_SIZE];
long min_error = 2147483647;

uint8_t lfsr1_state = 1;
uint8_t lfsr2_state = 1;

#define ERROR_THRESHOLD 100000 // Realistically, sum of squares for uint8_t is large

int hashFunction(uint8_t s1, uint8_t s2) {
  return s1 ^ s2;
}

void setup() {
  pinMode(PWM_OUT, OUTPUT);
  timer = micros();
  Serial.begin(9600);
  if (!SD.begin(CS_PIN)) {
    Serial.println("SD card initialization failed");
  }
}

void loop() {
  if (micros() - timer >= (1000000 / SAMPLING_FREQ)) {
    timer = micros();
    
    int input = map(analogRead(ANALOG_IN), 0, 1023, 0, 255);
    buffer[buf_index] = (uint8_t)input;
    
    // Step LFSRs
    auto step = [](uint8_t &state, uint8_t poly) {
        uint8_t bit = state & 1;
        state >>= 1;
        if (bit) state ^= poly;
        return bit;
    };
    
    uint8_t b1 = step(lfsr1_state, lfsr1_poly);
    uint8_t b2 = step(lfsr2_state, lfsr2_poly);
    
    // Periodically search for better states
    if (buf_index == 0) {
        uint8_t cand1 = random(1, 256);
        uint8_t cand2 = random(1, 256);
        long current_error = 0;
        int16_t temp_func[BUFFER_SIZE];
        
        uint8_t s1 = cand1;
        uint8_t s2 = cand2;
        for (int i = 0; i < BUFFER_SIZE; i++) {
            uint8_t bit1 = step(s1, lfsr1_poly);
            uint8_t bit2 = step(s2, lfsr2_poly);
            temp_func[i] = (bit1 * bit2) * 255;
            int diff = (int)buffer[i] - (int)temp_func[i];
            current_error += (long)diff * diff;
        }
        
        if (current_error < min_error) {
            min_error = current_error;
            for(int i=0; i<BUFFER_SIZE; i++) function_cand[i] = temp_func[i];
            Serial.print("New error: ");
            Serial.println(min_error);
        }
    }
    
    analogWrite(PWM_OUT, (uint8_t)function_cand[buf_index]);
    buf_index = (buf_index + 1) % BUFFER_SIZE;
  }
}
