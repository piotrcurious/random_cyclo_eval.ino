
// Arduino sketch to find representation of input signal by using cyclotomic polynomial and LFSR polynomials

// Define the analog input pin
#define ANALOG_IN A0

// Define the PWM output pin
#define PWM_OUT 9

// Define the sampling frequency in Hz
#define SAMPLING_FREQ 1000

// Define the buffer size proportional to sampling frequency
#define BUFFER_SIZE 256

// Define the cyclotomic polynomial coefficients
#define CYCLO_COEFFS {1, -1, 1, -1, 1, -1, 1, -1}

// Define the LFSR polynomials coefficients
#define LFSR1_COEFFS {1, 0, 0, 0, 0, 0, 0, 1}
#define LFSR2_COEFFS {1, 0, 0, 0, 0, 0, 1, 1}

// Declare the buffer array to store the input samples
int buffer[BUFFER_SIZE];

// Declare the index variable to keep track of the buffer position
int index = 0;

// Declare the timer variable to keep track of the sampling interval
unsigned long timer = 0;

// Declare the output variable to store the output value
int output = 0;

// Declare the cyclotomic polynomial variable
int cyclo = CYCLO_COEFFS;

// Declare the LFSR polynomials variables
int lfsr1 = LFSR1_COEFFS;
int lfsr2 = LFSR2_COEFFS;

// Declare the function candidates variables
int func1 = 0;
int func2 = 0;

// Declare the error variables to measure the difference between input and output
int error1 = 0;
int error2 = 0;

void setup() {
  // Set the analog input pin as input
  pinMode(ANALOG_IN, INPUT);

  // Set the PWM output pin as output
  pinMode(PWM_OUT, OUTPUT);

  // Set the PWM frequency to match the sampling frequency
  analogWriteFrequency(PWM_OUT, SAMPLING_FREQ);
}

void loop() {
  // Check if it is time to sample the input
  if (micros() - timer >= (1000000 / SAMPLING_FREQ)) {
    // Update the timer variable
    timer = micros();

    // Read the analog input value and map it to [0,255]
    int input = map(analogRead(ANALOG_IN), 0, 1023, 0, 255);

    // Store the input value in the buffer array
    buffer[index] = input;

    // Increment the index variable and wrap it around if it reaches the buffer size
    index++;
    if (index >= BUFFER_SIZE) {
      index = 0;
    }

    // Calculate the function candidates by multiplying two LFSR polynomials of 8bit size
    func1 = lfsr(lfsr1) * lfsr(lfsr2);
    func2 = lfsr(lfsr2) * lfsr(lfsr1);

    // Calculate the output value by evaluating the cyclotomic polynomial with the function candidates
    output = cyclo_eval(cyclo, func1);

    // Calculate the error values by subtracting the output value from the input value in the buffer
    error1 = abs(buffer[index] - output);
    error2 = abs(buffer[index] - cyclo_eval(cyclo, func2));

    // Compare the error values and choose the function candidate with lower error as output
    if (error2 < error1) {
      output = cyclo_eval(cyclo, func2);
    }

    // Write the output value to the PWM pin
    analogWrite(PWM_OUT, output);
    
    // Make the system converge infinitely by updating the cyclotomic polynomial coefficients randomly
    cyclo[random(8)] += random(-1,2);
    
    // Make sure that cyclotomic polynomial coefficients are either -1 or +1 
    for (int i=0; i<8; i++) {
      if (cyclo[i] < -1) {
        cyclo[i] = -1;
      }
      else if (cyclo[i] > +1) {
        cyclo[i] = +1;
      }
    }
}


// Function to implement a linear feedback shift register (LFSR) with a given polynomial
int lfsr(int poly) {
  // Declare a static variable to store the current state of the LFSR
  static int state = 1;

  // Declare a variable to store the output bit
  int output = 0;

  // Declare a variable to store the feedback bit
  int feedback = 0;

  // Loop through the polynomial coefficients and XOR them with the corresponding state bits
  for (int i = 0; i < 8; i++) {
    if (poly & (1 << i)) {
      feedback ^= state & (1 << i);
    }
  }

  // Shift the feedback bit to the rightmost position
  feedback >>= 7;

  // Shift the state to the left by one bit and append the feedback bit
  state = (state << 1) | feedback;

  // Extract the output bit from the state
  output = state & 1;

  // Return the output bit
  return output;
}

// Function to evaluate a cyclotomic polynomial with a given value
int cyclo_eval(int coeff[], int x) {
  // Declare a variable to store the result
  int result = 0;

  // Declare a variable to store the power of x
  int power = 1;

  // Loop through the coefficients and multiply them with the corresponding power of x
  for (int i = 0; i < 8; i++) {
    result += coeff[i] * power;
    power *= x;
  }

  // Return the result
  return result;
}
