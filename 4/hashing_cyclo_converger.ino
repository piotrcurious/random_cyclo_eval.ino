
#include <SD.h>
#define CS_PIN 10

// define the analog input pin
#define ANALOG_IN A0

// define the PWM output pin
#define PWM_OUT 9

// define the sampling frequency in Hz
#define SAMPLING_FREQ 1000

// define the buffer size proportional to the sampling frequency
#define BUFFER_SIZE SAMPLING_FREQ

// define the cyclotomic polynomial coefficients
#define CYCLO_DEGREE 16
const int cyclo_coef[CYCLO_DEGREE + 1] = {1, -1, 0, 0, 0, 0, 0, 0, -1, 0, 0, 0, 0, 0, 0, 0, 1};

// define the LFSR polynomials coefficients
#define LFSR_DEGREE 8
const int lfsr1_coef[LFSR_DEGREE + 1] = {1, 1, 0, 1, 1, 0, 0, 0, 1}; // x^8 + x^7 + x^4 + x^3 + x + 1
const int lfsr2_coef[LFSR_DEGREE + 1] = {1, 0, 1, 1, 0, 1, 0, 0, 1}; // x^8 + x^6 + x^5 + x^3 + x + 1

// declare the buffer array to store the input samples
int buffer[BUFFER_SIZE];

// declare the index variable to keep track of the buffer position
int index = 0;

// declare the timer variable to keep track of the sampling interval
unsigned long timer = 0;

// declare the function variable to store the function candidate
int function[LFSR_DEGREE * CYCLO_DEGREE];

// declare the error variable to store the error between the input and the function
long error = LONG_MAX;

// declare the lfsr variables to store the current state and output of the LFSRs
int lfsr1_state = random(256); // initialize with a random value
int lfsr2_state = random(256); // initialize with a random value
int lfsr1_out = lfsr1_state & 1; // get the least significant bit as output
int lfsr2_out = lfsr2_state & 1; // get the least significant bit as output

// define the error threshold for convergence
#define ERROR_THRESHOLD 0.01

// define the hash function that takes the LFSR states and outputs as inputs and returns a hash value
int hashFunction(int lfsr1_state, int lfsr2_state, int lfsr1_out,int lfsr2_out) {
  // use a simple XOR-based hash function
  return lfsr1_state ^ lfsr2_state ^ lfsr1_out ^ lfsr2_out;
}

// define the file name for the database file
const char* fileName = "database.txt";

void setup() {
  // set the PWM output pin as an output
  pinMode(PWM_OUT, OUTPUT);
  
  // initialize the timer variable with the current time in microseconds
  timer = micros();
  
  // initialize the SD card and check if it is ready
  if (!SD.begin(CS_PIN)) {
    Serial.println("SD card initialization failed");
    return;
  }
  Serial.println("SD card initialized");
}

void loop() {
  
  // check if the timer variable has reached the sampling interval in microseconds
  if (micros() - timer >= (1000000 / SAMPLING_FREQ)) {
    
    // read the analog input value and map it to a range of 0 to 255
    int input = map(analogRead(ANALOG_IN), 0, 1023, 0, 255);
    
    // store the input value in the buffer array at the current index position
    buffer[index] = input;
    
    // increment the index variable and wrap it around if it reaches the buffer size
    index = (index + 1) % BUFFER_SIZE;
    
    // reset the timer variable with the current time in microseconds
    timer = micros();
    
    // check if there is an existing function in the database file that matches the current LFSR states and outputs
    
    // calculate the hash value of the current LFSR states and outputs using the hash function
    int hashValue = hashFunction(lfsr1_state, lfsr2_state, lfsr1_out, lfsr2_out);
    
    // open the database file for reading and check if it exists
    File dbFile = SD.open(fileName);
    if (!dbFile) {
      Serial.println("Database file not found");
      return;
    }
    
    // loop through each line of the database file and split it by commas
    bool found = false;
    while (dbFile.available()) {
      String line = dbFile.readStringUntil('\n');
      int commaIndex = line.indexOf(',');
      int lineHashValue = line.substring(0, commaIndex).toInt();
      String lineFunctionValues = line.substring(commaIndex + 1);
      
      // compare the hash value of the current LFSR states and outputs with the hash value of each line
      if (hashValue == lineHashValue) {
        Serial.println("Found matching function in database");
        found = true;
        
        // copy the function values from the line to the function array
        int i = 0;
        int j = 0;
        while (j < lineFunctionValues.length()) {
          int k = lineFunctionValues.indexOf(',', j);
          if (k == -1) {
            k = lineFunctionValues.length();
          }
          function[i] = lineFunctionValues.substring(j, k).toInt();
          i++;
          j = k + 1;
        }
        
        // close the file and break out of the loop
        dbFile.close();
        break;
      }
    }
    
    // if no matching function is found in the database file, close the file and proceed to generate a new function candidate as before
    if (!found) {
      Serial.println("No matching function in database");
      dbFile.close();
      
      // generate a new function candidate by multiplying two LFSR polynomials and evaluating them at the cyclotomic polynomial roots
      
      // shift both LFSR states to the right by one bit and calculate the new least significant bit by XORing the previous state with the polynomial coefficients
      
      int new_bit1 = ((lfsr1_state >> 7) & 1) ^ ((lfsr1_state >> 6) & 1) ^ ((lfsr1_state >> 3) & 1) ^ ((lfsr1_state >> 2) & 1) ^ (lfsr1_state & 1);
      int new_bit2 = ((lfsr2_state >> 7) & 1) ^ ((lfsr2_state >> 6) & 1) ^ ((lfsr2_state >> 5) & 1) ^ ((lfsr2_state >> 3) & 1) ^ (lfsr2_state & 1);

      // update the LFSR states with the new bits
      lfsr1_state = (lfsr1_state >> 1) | (new_bit1 << 7);
      lfsr2_state = (lfsr2_state >> 1) | (new_bit2 << 7);
      
      // update the LFSR outputs by getting the new least significant bits
      lfsr1_out = lfsr1_state & 1;
      lfsr2_out = lfsr2_state & 1;
      
      // multiply the two LFSR outputs and store them in a temporary variable
      int temp = lfsr1_out * lfsr2_out;
      
      // loop through the function array and update each element by multiplying it with the temporary variable and adding it to the previous element
      for (int i = LFSR_DEGREE * CYCLO_DEGREE - 1; i > 0; i--) {
        function[i] = function[i] * temp + function[i - 1];
      }
      function[0] = function[0] * temp;
      
      // loop through the function array again and evaluate each element at the cyclotomic polynomial roots by substituting x with 2^i mod 256
      for (int i = 0; i < LFSR_DEGREE * CYCLO_DEGREE; i++) {
        int x = pow(2, i) % 256;
        int y = 0;
        for (int j = CYCLO_DEGREE; j >= 0; j--) {
          y = y * x + cyclo_coef[j];
        }
        function[i] = function[i] * y;
      }
    }
    
    // calculate the error between the input and the function by looping through the buffer array and subtracting each input value from the corresponding function value and squaring the difference
    long new_error = 0;
    for (int i = 0; i < BUFFER_SIZE; i++) {
      int diff = buffer[i] - function[i];
      new_error += diff * diff;
    }
    
    // if the error is smaller than the previous error and smaller than the error threshold, do the following
    if (new_error < error && new_error < ERROR_THRESHOLD) {
      
      // update the error variable with the new error and output the corresponding function value to the PWM pin
      error = new_error;
      analogWrite(PWM_OUT, function[index]);
      
      // check if there is already a function in the database file that matches the current LFSR states and outputs
      
      // calculate the hash value of the current LFSR states and outputs using the hash function
      int hashValue = hashFunction(lfsr1_state, lfsr2_state, lfsr1_out, lfsr2_out);
      
      // open the database file for reading and check if it exists
      File dbFile = SD.open(fileName);
      if (!dbFile) {
        Serial.println("Database file not found");
        return;
      }
      
      // loop through each line of the database file and split it by commas
      bool found = false;
      while (dbFile.available()) {
        String line = dbFile.readStringUntil('\n');
        int commaIndex = line.indexOf(',');
        int lineHashValue = line.substring(0, commaIndex).toInt();
        
        // compare the hash value of the current LFSR states and outputs with the hash value of each line
        if (hashValue == lineHashValue) {
          Serial.println("Found matching function in database");
          found = true;
          
          // close the file and break out of the loop
          dbFile.close();
          break;
        }
      }
      
      // if no matching function is found in the database file, append a new line to the file with the hash value and the function values separated by commas
      
      if (!found) {
        Serial.println("Appending new function to database");
        
        // open the database file for writing and check if it exists
        File dbFile = SD.open(fileName, FILE_WRITE);
        if (!dbFile) {
          Serial.println("Database file not found");
          return;
        }
        
        // write the hash value and a comma to the file
        dbFile.print(hashValue);
        dbFile.print(',');
        
        // loop through the function array and write each value and a comma to the file
        for (int i = 0; i < LFSR_DEGREE * CYCLO_DEGREE; i++) {
          dbFile.print(function[i]);
          if (i < LFSR_DEGREE * CYCLO_DEGREE - 1) {
            dbFile.print(',');
          }
        }
        
        // write a new line to the file
        dbFile.println();
        
        // close the file
        dbFile.close();
      }
      
    }
    
  }
  
}
