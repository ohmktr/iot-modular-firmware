import re
from collections import defaultdict

# Log file path (replace with your actual log file path)
log_file_path = "C:/Users/exito/OneDrive/เดสก์ท็อป/iot-modular/test_lora_commu_LORA_LAST.log"


# Function to parse and validate the log file
def parse_and_validate_log(file_path):
    # Define validation rules
    validation_rules = {
        "920.000": {"Length": 6, "Message": "4E4F44453153"},
        "920.600": {"Length": 7, "Message": "4E4F444532534E"},
        "922.400": {"Length": 8, "Message": "4E4F444533534E52"}
    }

    # Regular expressions to capture data
    freq_pattern = re.compile(r"Start to Receiver at Frequency: ([\d.]+) MHz")
    length_pattern = re.compile(r"Receiver is \+TEST: LEN:(\d+), RSSI:([-]?\d+), SNR:(\d+)")
    message_pattern = re.compile(r"Receiver is \+TEST: RX \"([A-F0-9]+)\"")

    frequency_data = defaultdict(list)
    current_freq = None

    with open(file_path, 'r') as file:
        for line in file:
            # Detect frequency
            freq_match = freq_pattern.search(line)
            if freq_match:
                current_freq = freq_match.group(1)

            # Detect length, RSSI, and SNR
            length_match = length_pattern.search(line)
            if length_match and current_freq:
                frequency_data[current_freq].append({
                    "Length": int(length_match.group(1)),
                    "RSSI": int(length_match.group(2)),
                    "SNR": int(length_match.group(3)),
                    "Message": None  # Placeholder for message
                })

            # Detect message
            message_match = message_pattern.search(line)
            if message_match and current_freq and frequency_data[current_freq]:
                frequency_data[current_freq][-1]["Message"] = message_match.group(1)

    # Validation results by frequency
    results_by_frequency = {}

    for freq, entries in frequency_data.items():
        results_by_frequency[freq] = {"Correct": 0, "Incorrect": 0, "Errors": []}
        for entry in entries:
            if freq in validation_rules:
                expected_length = validation_rules[freq]["Length"]
                expected_message = validation_rules[freq]["Message"]

                # Check conditions
                if entry["Length"] == expected_length and entry["Message"] == expected_message:
                    results_by_frequency[freq]["Correct"] += 1
                else:
                    results_by_frequency[freq]["Incorrect"] += 1
                    results_by_frequency[freq]["Errors"].append({
                        "Received": entry,
                        "Expected": validation_rules[freq]
                    })

    return results_by_frequency

# Function to display results
def display_results_by_frequency(results_by_frequency):
    for freq, result in results_by_frequency.items():
        print(f"Frequency {freq} MHz:")
        print(f"  Total Packages: {result['Correct'] + result['Incorrect']}")
        print(f"  Correct Packages: {result['Correct']}")
        print(f"  Incorrect Packages: {result['Incorrect']}")
        # if result["Errors"]:
        #     print(f"  Errors:")
        #     for error in result["Errors"]:
        #         print(f"    Received: {error['Received']}")
        #         print(f"    Expected: {error['Expected']}")
        #     print("-" * 50)
        # else:
        #     print("  No Errors")
        # print("=" * 50)

# Parse, validate, and display the log data
results_by_frequency = parse_and_validate_log(log_file_path)
display_results_by_frequency(results_by_frequency)
