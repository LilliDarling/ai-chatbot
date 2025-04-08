/**
 * NeuroChef - Recipe Utilities
 * 
 * This header file declares functions for recipe data management and query processing.
 */

#ifndef RECIPE_UTILS_H
#define RECIPE_UTILS_H

#include <stdbool.h>

typedef struct {
    char* id;
    char* name;
    char** meal_type;
    int meal_type_count;
    char** ingredients;
    int ingredients_count;
    char** preparation_steps;
    int preparation_steps_count;
    int prep_time_duration;
    char* prep_time_unit;
    int cook_time_duration;
    char* cook_time_unit;
    char* description;
    char** sensory_texture;
    int sensory_texture_count;
    char** sensory_temperature;
    int sensory_temperature_count;
    char** sensory_taste;
    int sensory_taste_count;
    char** sensory_smell;
    int sensory_smell_count;
} Recipe;

typedef struct {
    Recipe* recipes;
    int recipe_count;
    char* error_message;
} RecipeDB;

typedef enum {
    QUERY_INGREDIENTS,
    QUERY_PREPARATION,
    QUERY_SENSORY,
    QUERY_TIME,
    QUERY_GENERAL,
    QUERY_UNKNOWN
} QueryType;

typedef struct {
    bool success;
    char* recipe_name;
    QueryType query_type;
    char* response;
} QueryResult;

/**
 * Initialize the recipe database by loading and parsing the JSON file
 * 
 * @return A pointer to the initialized RecipeDB structure
 */
RecipeDB* init_recipe_db(const char* json_path);

/**
 * Free the memory allocated for the recipe database
 * 
 * @param db The recipe database to free
 */
void free_recipe_db(RecipeDB* db);

/**
 * Process a recipe query and generate a response
 * 
 * @param db The recipe database
 * @param query The user query string
 * @return A QueryResult structure containing the response
 */
QueryResult process_recipe_query(RecipeDB* db, const char* query);

/**
 * Free the memory allocated for a query result
 * 
 * @param result The query result to free
 */
void free_query_result(QueryResult* result);

/**
 * Check if a query is a recipe-specific query
 * 
 * @param query The user query string
 * @return true if the query is recipe-specific, false otherwise
 */
bool is_recipe_query(const char* query);

/**
 * Get the last error message from the recipe database
 * 
 * @param db The recipe database
 * @return The error message or NULL if no error
 */
const char* get_recipe_db_error(RecipeDB* db);

/**
 * Convert a string to lowercase
 * 
 * @param str The string to convert
 * @return A new lowercase string (caller must free)
 */
char* str_to_lower(const char* str);

#endif /* RECIPE_UTILS_H */
