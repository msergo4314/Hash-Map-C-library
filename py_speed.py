import time
import random

NUMBER_OF_KEYS = int(1e7)
RANGE = 1000

def main():
    print(f"Attempting to batch insert {NUMBER_OF_KEYS:.2e} keys into Python dictionary")
    
    # Generate keys and values
    start = time.time()
    keys = list(range(NUMBER_OF_KEYS))
    values = [random.randint(1, RANGE - 1) for _ in range(NUMBER_OF_KEYS)]

    # Create dictionary
    dictionary = {}

    # Start timer
    start_time = time.time()

    # Insert keys and values in batch
    for i in range(NUMBER_OF_KEYS):
        dictionary[keys[i]] = values[i]

    # End timer
    end_time = time.time()

    # Calculate time spent
    time_spent = end_time - start_time
    print(f"Operation took {time_spent:.4f} seconds")

    # Optionally print dictionary info
    print(f"Dictionary size: {len(dictionary)}")
    
    print(f"total time taken: {time.time() - start:.4f} seconds")

if __name__ == "__main__":
    main()
