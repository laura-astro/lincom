'''
Written 22Feb95 by Scot Kleinman.
6Feb96 -added -s switch - sjk
Needs to be linked with ncolread.o (my generic read data routines).

Python version written in 2025 by Laura C. Sultan
'''

import argparse
import math

# Constants
FREQS = 200  # Max freqs. allowed in input file
CONV = 0.4   # Default equality criteria in microHz
ALIAS = 11.57  # Default alias check in microHz
NC = 1  # Default column number of freq. in input file
SKIP = 0  # Default number of header lines to skip

data = []  # List to store the frequencies
conv = CONV  # Convergence criteria
alias = ALIAS  # Alias check in microHz
skip = SKIP  # Number of header lines to skip


def read_data(file_path, col, skip):
    """Reads the data from the file and returns it as a list of floats."""
    with open(file_path, 'r') as file:
        lines = file.readlines()
    
    # Skip the header lines
    data_lines = lines[skip:]

    freqs = []
    for line in data_lines:
        line = line.strip()  # Remove leading/trailing spaces and newlines
        if not line:  # Skip empty lines
            continue
        
        # Split by whitespace and get the frequency in the specified column
        fields = line.split()
        
        # Debug: print the split fields to verify what's being processed
        print(f"Split line: {fields}")
        
        if len(fields) > col - 1:  # Check if the column exists
            try:
                freq = float(fields[col - 1])  # Convert to float (Python index is 0-based)
                freqs.append(freq)
            except ValueError:
                print(f"Warning: Unable to convert '{fields[col - 1]}' to float in line: {line.strip()}")
    
    # Debug print to check if we have valid frequencies
    print(f"Read frequencies: {freqs}")
    
    # If frequencies are in Hz, convert them to microHz
    if freqs and freqs[0] < 1:
        print("Converting input frequencies in Hz to microHz.")
        freqs = [f * 1e6 for f in freqs]
    
    return freqs



def find_combinations(freqs, conv, alias):
    """Finds combinations of frequencies based on the given criteria."""
    result = []

    for k, freq_k in enumerate(freqs):
        result.append(f"{freq_k:.3f}")

        for i in range(len(freqs)):
            for j in range(i, len(freqs)):
                sum1 = freqs[i] + freqs[j]
                sum2 = 2 * freqs[i] + freqs[j]
                sum3 = freqs[i] + 2 * freqs[j]
                
                # Check for A+B=C+/-Alias
                diff = sum1 - freq_k
                if abs(diff) < conv:
                    result.append(f" {freqs[i]:8.3f}+{freqs[j]:8.3f} diff={diff:5.3f}")
                elif abs(diff - alias) < conv:
                    result.append(f" {freqs[i]:8.3f}+{freqs[j]:8.3f}-alias diff={diff - alias:5.3f}")
                elif abs(diff + alias) < conv:
                    result.append(f" {freqs[i]:8.3f}+{freqs[j]:8.3f}+alias diff={diff + alias:5.3f}")

                # Check for 2A+B=C+/-Alias
                diff2 = sum2 - freq_k
                if abs(diff2) < conv:
                    result.append(f" 2*{freqs[i]:8.3f}+{freqs[j]:8.3f} diff={diff2:5.3f}")
                elif abs(diff2 - alias) < conv:
                    result.append(f" 2*{freqs[i]:8.3f}+{freqs[j]:8.3f}-alias diff={diff2 - alias:5.3f}")
                elif abs(diff2 + alias) < conv:
                    result.append(f" 2*{freqs[i]:8.3f}+{freqs[j]:8.3f}+alias diff={diff2 + alias:5.3f}")

                # Check for A+2B=C+/-Alias
                diff3 = sum3 - freq_k
                if abs(diff3) < conv:
                    result.append(f" {freqs[i]:8.3f}+2*{freqs[j]:8.3f} diff={diff3:5.3f}")
                elif abs(diff3 - alias) < conv:
                    result.append(f" {freqs[i]:8.3f}+2*{freqs[j]:8.3f}-alias diff={diff3 - alias:5.3f}")
                elif abs(diff3 + alias) < conv:
                    result.append(f" {freqs[i]:8.3f}+2*{freqs[j]:8.3f}+alias diff={diff3 + alias:5.3f}")

    return result


def main():
    # Argument parser to handle command line options
    parser = argparse.ArgumentParser(description="Find combination frequencies in input data.")
    parser.add_argument("-f", "--file", required=True, help="Input data file")
    parser.add_argument("-a", "--alias", type=float, default=ALIAS, help="Alias value in microHz")
    parser.add_argument("-c", "--column", type=int, default=NC, help="Column number of frequency in input file")
    parser.add_argument("-e", "--conv", type=float, default=CONV, help="Equality criteria in microHz")
    parser.add_argument("-s", "--skip", type=int, default=SKIP, help="Number of header lines to skip")
    
    args = parser.parse_args()

    global alias, conv, skip
    alias = args.alias
    conv = args.conv
    skip = args.skip

    # Read data from the file
    freqs = read_data(args.file, args.column, skip)
    print(f"There are {len(freqs)} points in the data file.")
    
    # Find combinations
    results = find_combinations(freqs, conv, alias)
    
    # Output results
    output_file = f"{args.file}.sdv"
    with open(output_file, "w") as ofp:
        ofp.write(f"Input file={args.file}\n")
        if skip != 0:
            ofp.write(f"Skipped {skip} lines at start of input file.\n")
        ofp.write(f"Equality criteria={conv:.2f} microHz\n")
        ofp.write(f"Alias={alias:.3f} microHz\n\n")

        for line in results:
            ofp.write(f"{line}\n")
    
    print(f"Output written to {output_file}")


if __name__ == "__main__":
    main()
