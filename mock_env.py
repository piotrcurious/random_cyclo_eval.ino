
import random

LFSR_POLY = 0xB8
BUFFER_SIZE = 128
CYCLO_DEGREE = 8

def step_lfsr(state):
    # Standard 8-bit LFSR (Galois)
    # x^8 + x^6 + x^5 + x^4 + 1
    # Actually, let's use a simpler one or ensure consistency with the Arduino code
    bit = state & 1
    state >>= 1
    if bit:
        state ^= LFSR_POLY
    return state, bit

def generate_filtered_output(seed, length):
    state = seed
    bits = []
    # Pre-generate bits
    s = state
    for _ in range(length + CYCLO_DEGREE + 1):
        s, bit = step_lfsr(s)
        bits.append(bit)

    output = []
    for i in range(length):
        # Phi_16(z) = 1*h[i] + 1*h[i+8]
        val = bits[i] + bits[i + 8]
        output.append(val * 127)
    return output

def calculate_error(sig1, sig2):
    return sum((a - b) ** 2 for a, b in zip(sig1, sig2))

def main():
    # 1. Generate a "target" signal using a known seed
    true_seed = 42
    target_signal = generate_filtered_output(true_seed, BUFFER_SIZE)
    print(f"Target signal generated with seed {true_seed}")

    # 2. Add some deterministic "noise" to simulate analogRead variations
    # (Just to make it more realistic, though not strictly necessary)
    target_signal = [max(0, min(255, s + random.randint(-10, 10))) for s in target_signal]

    # 3. Simulate the randomized search
    best_state = 1
    min_error = float('inf')

    print("Starting search...")
    # There are only 255 possible seeds for an 8-bit LFSR.
    # An exhaustive search is actually feasible.
    for candidate_state in range(1, 256):
        candidate_output = generate_filtered_output(candidate_state, BUFFER_SIZE)

        error = calculate_error(target_signal, candidate_output)

        if error < min_error:
            min_error = error
            best_state = candidate_state
            print(f"Found state {best_state}, Error {min_error}")

    print(f"\nSearch finished.")
    print(f"True seed: {true_seed}")
    print(f"Found seed: {best_state}")

    if best_state == true_seed:
        print("SUCCESS: Converged to the correct seed!")
    else:
        print(f"Final error: {min_error}")
        # Sometimes multiple seeds might lead to the same sequence if they're in the same cycle.
        # But in a cycle of 255, each state is unique.

if __name__ == "__main__":
    main()
