/**
 * NeuroChef - A chatbot for neurodivergent meal planning
 * 
 * This program provides a command-line interface for the NeuroChef chatbot.
 * It handles user input and calls the Python script for processing.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_INPUT_SIZE 1024
#define MAX_OUTPUT_SIZE 4096

/**
 * Call the Python script and get the response
 * 
 * @param input The user input to process
 * @return The response from the Python script
 */
char* get_python_response(const char* input) {
    // Construct the command to call the Python script
    char command[MAX_INPUT_SIZE + 100];
    
    // Call the Python module directly
    // Escape quotes in the input to prevent command injection
    char escaped_input[MAX_INPUT_SIZE * 2];
    int j = 0;
    for (int i = 0; input[i] != '\0'; i++) {
        if (input[i] == '"' || input[i] == '\\') {
            escaped_input[j++] = '\\';
        }
        escaped_input[j++] = input[i];
    }
    escaped_input[j] = '\0';
    
    // Construct the command
    snprintf(command, sizeof(command), 
             "python -m neurochef.logic \"%s\"", 
             escaped_input);
    
    // Open a pipe to the command
    FILE* pipe = popen(command, "r");
    if (!pipe) {
        return "Error: Failed to run Python script.";
    }
    
    // Read the output
    static char output[MAX_OUTPUT_SIZE];
    fgets(output, sizeof(output), pipe);
    
    // Close the pipe
    pclose(pipe);
    
    return output;
}

/**
 * Main function
 */
int main() {
    char input[MAX_INPUT_SIZE];
    
    // Display welcome message
    printf("Welcome to NeuroChef!\n");
    
    // Main interaction loop
    while (1) {
        // Prompt for input
        printf("> ");
        fflush(stdout);
        
        // Get user input
        if (!fgets(input, sizeof(input), stdin)) {
            break;  // Exit on EOF
        }
        
        // Remove trailing newline
        input[strcspn(input, "\n")] = 0;
        
        // Check for exit command
        if (strcmp(input, "exit") == 0 || strcmp(input, "quit") == 0) {
            printf("Goodbye! Take care.\n");
            break;
        }
        
        // Get response from Python script
        char* response = get_python_response(input);
        
        // Display the response
        printf("%s\n", response);
    }
    
    return 0;
}
