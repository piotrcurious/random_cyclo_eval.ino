
import random
import math

LFSR_POLY = 0xB8
BUFFER_SIZE = 128
CYCLO_DEGREE = 8

def step_lfsr(state):
    bit = state & 1
    state >>= 1
    if bit:
        state ^= LFSR_POLY
    return state, bit

def generate_filtered_output(seed, length):
    state = seed
    bits = []
    s = state
    for _ in range(length + CYCLO_DEGREE + 1):
        s, bit = step_lfsr(s)
        bits.append(bit)

    output = []
    for i in range(length):
        val = bits[i] + bits[i + 8]
        output.append(val * 127)
    return output

def calculate_error(sig1, sig2):
    return sum((a - b) ** 2 for a, b in zip(sig1, sig2))

def generate_svg(signals, filename="comparison_plot.svg"):
    width = 800
    height = 400
    padding = 50

    def scale_x(x):
        return padding + (x / (BUFFER_SIZE - 1)) * (width - 2 * padding)

    def scale_y(y):
        return height - padding - (y / 255.0) * (height - 2 * padding)

    svg = [f'<svg width="{width}" height="{height}" xmlns="http://www.w3.org/2000/svg">']
    svg.append(f'<rect width="100%" height="100%" fill="white" />')
    svg.append(f'<line x1="{padding}" y1="{height-padding}" x2="{width-padding}" y2="{height-padding}" stroke="black" />')
    svg.append(f'<line x1="{padding}" y1="{padding}" x2="{padding}" y2="{height-padding}" stroke="black" />')

    colors = ["blue", "red", "green"]
    for idx, (label, signal) in enumerate(signals.items()):
        color = colors[idx % len(colors)]
        points = " ".join([f"{scale_x(i)},{scale_y(v)}" for i, v in enumerate(signal)])
        svg.append(f'<polyline points="{points}" fill="none" stroke="{color}" stroke-width="2" opacity="0.7" />')
        svg.append(f'<text x="{width-150}" y="{30 + idx*20}" fill="{color}">{label}</text>')

    svg.append('</svg>')

    with open(filename, "w") as f:
        f.write("\n".join(svg))
    print(f"Generated {filename}")

def main():
    # Target 1: A sine wave
    sine_target = [int(127 + 127 * math.sin(2 * math.pi * i / 32)) for i in range(BUFFER_SIZE)]

    # Target 2: An LFSR signal with noise
    true_seed = 42
    lfsr_target = generate_filtered_output(true_seed, BUFFER_SIZE)
    lfsr_target_noised = [max(0, min(255, s + random.randint(-15, 15))) for s in lfsr_target]

    def find_best(target):
        best_state = 1
        min_err = float('inf')
        for s in range(1, 256):
            out = generate_filtered_output(s, BUFFER_SIZE)
            err = calculate_error(target, out)
            if err < min_err:
                min_err = err
                best_state = s
        return best_state, generate_filtered_output(best_state, BUFFER_SIZE)

    print("Finding best match for Sine target...")
    sine_best_seed, sine_approx = find_best(sine_target)

    print("Finding best match for LFSR target...")
    lfsr_best_seed, lfsr_approx = find_best(lfsr_target_noised)

    # Generate SVG for LFSR comparison
    generate_svg({
        "Target (Noised)": lfsr_target_noised,
        "Best LFSR Match": lfsr_approx
    }, "lfsr_comparison.svg")

    # Generate SVG for Sine approximation
    generate_svg({
        "Sine Target": sine_target,
        "Best LFSR Match": sine_approx
    }, "sine_approximation.svg")

if __name__ == "__main__":
    main()
