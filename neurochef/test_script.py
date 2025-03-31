#!/usr/bin/env python3
"""
Simple test script for the NeuroChef logic module.
"""

from neurochef.logic import find_matches, load_data

# Test with a simple input
test_input = "I need meals with smooth texture"
print(f"Testing input: '{test_input}'")

try:
    # Try to load the data
    print("Loading data...")
    data = load_data()
    print("Data loaded successfully!")
    
    # Try to find matches
    print("Finding matches...")
    response = find_matches(test_input, data)
    print(f"Response: {response}")
    
    print("Test completed successfully!")
except Exception as e:
    print(f"Error: {e}")
