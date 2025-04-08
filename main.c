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
#define MAX_COMMAND_SIZE (MAX_INPUT_SIZE * 2 + 100) // Allow for escaped input and command prefix

/**
 * Call the Python script and get the response
 * 
 * @param input The user input to process
 * @return The response from the Python script
 */
char* get_python_response(const char* input) {
    char command[MAX_COMMAND_SIZE];
    
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

    snprintf(command, sizeof(command), 
             "python -m neurochef.logic \"%s\"", 
             escaped_input);

    FILE* pipe = popen(command, "r");
    if (!pipe) {
        return "Error: Failed to run Python script.";
    }

    static char output[MAX_OUTPUT_SIZE];
    if (!fgets(output, sizeof(output), pipe)) {
        // If fgets returns NULL, there was an error or no output
        int exit_code = pclose(pipe);
        if (exit_code != 0) {
            // Check if the error might be due to Python not being found
            if (strstr(command, "python") != NULL) {
                return "Error: Python not found. Please ensure Python is installed and in your PATH.";
            }
            return "Error: Failed to execute command.";
        }
        return "No output from command.";
    }

    pclose(pipe);
    
    return output;
}

/**
 * Main function
 */
int main() {
    char input[MAX_INPUT_SIZE];

    printf("Welcome to NeuroChef!\n");

    while (1) {
        printf("> ");
        fflush(stdout);

        if (!fgets(input, sizeof(input), stdin)) {
            break;
        }

        input[strcspn(input, "\n")] = 0;

        if (strcmp(input, "exit") == 0 || strcmp(input, "quit") == 0) {
            printf("Goodbye! Take care.\n");
            break;
        }

        char* response = get_python_response(input);

        printf("%s\n", response);
    }
    
    return 0;
}
