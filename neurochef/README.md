# NeuroChef

A command-line chatbot designed to help neurodivergent individuals with meal planning.

## Description

NeuroChef is a hybrid C/Python chatbot that provides meal suggestions based on sensory preferences, executive function challenges, and other neurodivergent needs. The chatbot uses a simple keyword matching system to identify user intents and provide relevant meal suggestions.

## Features

- Provides meal suggestions based on sensory preferences (e.g., smooth, soft textures)
- Offers advice for executive function challenges related to meal planning
- Suggests quick meal options
- Simple command-line interface

## Requirements

- C compiler (gcc recommended)
- Python 3.8 or higher
- Poetry (for Python dependency management)

## Installation

1. Clone the repository
2. Install Python dependencies:
   ```
   cd neurochef
   poetry install
   ```
3. Compile the C program:
   ```
   make
   ```

## Usage

Run the chatbot:
```
./neurochef
```

Example interactions:
- "I need meals with smooth texture"
- "What are some quick meals?"
- "I have difficulty planning meals"
- Type "exit" or "quit" to exit the chatbot

## Project Structure

- `main.c`: C program for command-line interface
- `neurochef/logic.py`: Python script for processing user input
- `meal_data.json`: JSON data file with meal information
- `tests/`: Directory containing tests

## License

[MIT License](LICENSE)
