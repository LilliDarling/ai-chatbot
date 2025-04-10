/**
 * NeuroChef - A chatbot for neurodivergent meal planning
 * 
 * This program provides a command-line interface for the NeuroChef chatbot.
 * It handles user input and processes recipe queries in C, falling back to
 * Python for other types of queries.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "recipe_utils.h"

#define MAX_INPUT_SIZE 1024
#define MAX_OUTPUT_SIZE 4096
#define MAX_COMMAND_SIZE (MAX_INPUT_SIZE * 2 + 100)
#define JSON_PATH "C:/Users/valky/Repos/neurochef/meal_data.json"

static RecipeDB* recipe_db = NULL;

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
        int exit_code = pclose(pipe);
        if (exit_code != 0) {
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
 * Process user input and generate a response
 * 
 * @param input The user input to process
 * @return The response to the user
 */
char* process_input(const char* input) {
    if (is_recipe_query(input)) {
        printf("Processing as recipe query: %s\n", input);

        QueryResult result = process_recipe_query(recipe_db, input);
        
        if (result.success) {
            char* response = strdup(result.response);
            free_query_result(&result);
            return response;
        } else {
            char* error = strdup(result.response);
            free_query_result(&result);

            if (strstr(error, "I couldn't find a recipe") == NULL) {
                free(error);
                printf("Recipe query processing failed, falling back to Python\n");
                return get_python_response(input);
            }
            
            return error;
        }
    } else {
        printf("Not a recipe query, using Python: %s\n", input);

        char* query_lower = str_to_lower(input);
        if (query_lower) {
            if (strstr(query_lower, "what should i") || 
                strstr(query_lower, "suggest") || 
                strstr(query_lower, "recommend") ||
                (strstr(query_lower, "what are") && strstr(query_lower, "food"))) {
                
                free(query_lower);

                char response[MAX_OUTPUT_SIZE];
                snprintf(response, sizeof(response), 
                         "I can tell you about specific recipes like Berry Blast Smoothie, "
                         "Creamy Garlic Mashed Potatoes, Mild Chicken Salad, or Soft Baked Sweet Potato. "
                         "Try asking something like 'What is in Berry Blast Smoothie?' or "
                         "'How do I make Soft Baked Sweet Potato?'");
                
                return strdup(response);
            }
            free(query_lower);
        }
        
        return get_python_response(input);
    }
}

/**
 * Initialize the recipe database
 * 
 * @return 0 on success, -1 on failure
 */
int init_database() {
    recipe_db = init_recipe_db(JSON_PATH);
    
    if (!recipe_db) {
        fprintf(stderr, "Failed to initialize recipe database\n");
        return -1;
    }
    
    if (recipe_db->error_message) {
        fprintf(stderr, "Error initializing recipe database: %s\n", recipe_db->error_message);
        free_recipe_db(recipe_db);
        recipe_db = NULL;
        return -1;
    }
    
    return 0;
}

/**
 * Main function
 */
int main() {
    char input[MAX_INPUT_SIZE];

    printf("Welcome to NeuroChef!\n");

    if (init_database() != 0) {
        printf("Warning: Recipe database initialization failed. Falling back to Python only.\n");
    } else {
        printf("Recipe database loaded with %d recipes.\n", recipe_db->recipe_count);
        printf("You can now ask questions about specific recipes, like:\n");
        printf("- What is in Berry Blast Smoothie?\n");
        printf("- How do I make Creamy Garlic Mashed Potatoes?\n");
        printf("- What's the texture of Mild Chicken Salad?\n");
        printf("- How long does it take to make Soft Baked Sweet Potato?\n");
    }

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

        char* response = process_input(input);
        printf("%s\n", response);
        
        if (response != NULL && 
            strcmp(response, "Error: Failed to run Python script.") != 0 &&
            strcmp(response, "Error: Python not found. Please ensure Python is installed and in your PATH.") != 0 &&
            strcmp(response, "Error: Failed to execute command.") != 0 &&
            strcmp(response, "No output from command.") != 0) {
            free(response);
        }
    }

    if (recipe_db) {
        free_recipe_db(recipe_db);
    }
    
    return 0;
}
