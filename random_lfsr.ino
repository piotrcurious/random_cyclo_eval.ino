
// Arduino sketch to find solution to a pseudorandom function
// using cyclotomic polynomial and LFSR polynomials

// Define the feedback polynomial for the LFSR as x^16 + x^14 + x^13 + x^11 + 1
#define LFSR_POLY 0xB400

// Define the cyclotomic polynomial as x^16 - 1
#define CYCLO_POLY 0x0001

// Define the initial state of the LFSR as 1
uint16_t lfsr_state = 0x0001;

// Define a function to perform one step of the LFSR
uint16_t lfsr_step() {
  // Shift the state right by one bit
  uint16_t new_bit = lfsr_state >> 15;
  lfsr_state = lfsr_state << 1;
  
  // If the new bit is 1, XOR the state with the feedback polynomial
  if (new_bit == 1) {
    lfsr_state = lfsr_state ^ LFSR_POLY;
  }
  
  // Return the new bit
  return new_bit;
}

// Define a function to evaluate the cyclotomic polynomial at a given value
int16_t cyclo_eval(uint16_t x) {
  // Initialize the result as -1
  int16_t result = -1;
  
  // Loop over each bit of x
  for (int i = 0; i < 16; i++) {
    // If the bit is 1, add it to the result
    if (x & (1 << i)) {
      result = result + 1;
    }
    
    // Multiply the result by x modulo CYCLO_POLY
    result = (result * x) % CYCLO_POLY;
  }
  
  // Return the result
  return result;
}

// Define a function to check if a given value is a solution to the pseudorandom function
bool is_solution(uint16_t x) {
  // Evaluate the cyclotomic polynomial at x
  int16_t y = cyclo_eval(x);
  
  // If y is zero, return false
  if (y == 0) {
    return false;
  }
  
  // Loop over each bit of y
  for (int i = 0; i < 16; i++) {
    // If the bit is not equal to the next bit from the LFSR, return false
    if ((y & (1 << i)) != (lfsr_step() << i)) {
      return false;
    }
  }
  
  // If all bits match, return true
  return true;
}

void setup() {
  // Initialize serial communication at 9600 baud rate
  Serial.begin(9600);
}

void loop() {
  // Generate a random value between 0 and 65535
  uint16_t x = random(0,65536);
  
  // Check if it is a solution to the pseudorandom function
  if (is_solution(x)) {
    // Print it to the serial monitor
    Serial.print("Found a solution: ");
    Serial.println(x, HEX);
    
    // Stop the loop
    while (true);
    
    // Alternatively, you can continue searching for more solutions by removing this line
    // and resetting the LFSR state to its initial value: lfsr_state = 0x0001;
    
    // Note that there may be no more solutions or they may be very rare
    // depending on the choice of polynomials and initial state
    
    // You can also change the polynomials and initial state to explore different cases
    // See https://en.wikipedia.org/wiki/Linear-feedback_shift_register for more information
    
    // You can also use different methods to generate random values, such as analogRead()
    // or another LFSR with a different polynomial and state
    
    // Have fun experimenting!

//Source: Conversation with Bing, 4/14/2023(1) Sketch | Arduino. https://www.arduino.cc/en/Tutorial/Sketch/ Accessed 4/14/2023.
//(2) Arduino Sketches | Arduino Documentation. https://docs.arduino.cc/learn/programming/sketches/ Accessed 4/14/2023.
//(3) Getting Started with Arduino | Arduino Documentation. https://docs.arduino.cc/learn/starting-guide/getting-started-arduino/ Accessed 4/14/2023.
//(4) Pseudorandom function family - Wikipedia. https://en.wikipedia.org/wiki/Pseudorandom_function_family Accessed 4/14/2023.
//(5) Pseudorandom function (PRF) - Glossary | CSRC - NIST. https://csrc.nist.gov/glossary/term/Pseudorandom_function Accessed 4/14/2023.
//(6) Cryptography - Pseudo-Random Functions - Stanford University. https://crypto.stanford.edu/pbc/notes/crypto/prf.html Accessed 4/14/2023.
//(7) Cyclotomic polynomial - Wikipedia. https://en.wikipedia.org/wiki/Cyclotomic_polynomial Accessed 4/14/2023.
//(8) Cyclotomic Polynomial -- from Wolfram MathWorld. https://mathworld.wolfram.com/CyclotomicPolynomial.html Accessed 4/14/2023.
//(9) Cyclotomic Polynomials - Whitman College. https://www.whitman.edu/Documents/Academics/Mathematics/2015/Final%20Project%20-%20Porter,%20Brett.pdf Accessed 4/14/2023.
//(10) Linear-feedback shift register - Wikipedia. https://en.wikipedia.org/wiki/Linear-feedback_shift_register Accessed 4/14/2023.
//(11) Pseudo Random Number Generation Using Linear Feedback Shift Registers. https://www.analog.com/en/design-notes/random-number-generation-using-lfsr.html Accessed 4/14/2023.
//(12) Polynomial notation of LFSR - Cryptography Stack Exchange. https://crypto.stackexchange.com/questions/95412/polynomial-notation-of-lfsr Accessed 4/14/2023.
