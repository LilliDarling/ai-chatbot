/**
 * NeuroChef - Recipe Utilities Implementation
 * 
 * This file implements functions for recipe data management and query processing.
 */

#include "recipe_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINE_LENGTH 4096
#define MAX_RECIPE_COUNT 100
#define MAX_INGREDIENTS 50
#define MAX_STEPS 30
#define MAX_SENSORY_ATTRS 10
#define MAX_RESPONSE_LENGTH 4096

static char* str_duplicate(const char* str) {
    if (!str) return NULL;
    size_t len = strlen(str);
    char* dup = (char*)malloc(len + 1);
    if (dup) {
        strcpy(dup, str);
    }
    return dup;
}

char* str_to_lower(const char* str) {
    if (!str) return NULL;
    char* lower = str_duplicate(str);
    if (!lower) return NULL;
    
    for (char* p = lower; *p; p++) {
        *p = tolower(*p);
    }
    return lower;
}

static char* extract_string_value(const char* json, const char* key) {
    char search_key[256];
    snprintf(search_key, sizeof(search_key), "\"%s\"", key);
    
    char* key_pos = strstr(json, search_key);
    if (!key_pos) return NULL;
    
    char* colon_pos = strchr(key_pos, ':');
    if (!colon_pos) return NULL;

    char* value_start = colon_pos + 1;
    while (*value_start && isspace(*value_start)) value_start++;
    
    if (*value_start != '"') return NULL;
    
    value_start++;

    char* value_end = value_start;
    while (*value_end && *value_end != '"') {
        if (*value_end == '\\' && *(value_end + 1)) {
            value_end += 2;
        } else {
            value_end++;
        }
    }
    
    if (*value_end != '"') return NULL;
    
    size_t value_len = value_end - value_start;
    char* value = (char*)malloc(value_len + 1);
    if (!value) return NULL;
    
    strncpy(value, value_start, value_len);
    value[value_len] = '\0';
    
    return value;
}

static int extract_int_value(const char* json, const char* key) {
    char search_key[256];
    snprintf(search_key, sizeof(search_key), "\"%s\"", key);
    
    char* key_pos = strstr(json, search_key);
    if (!key_pos) return -1;
    
    char* colon_pos = strchr(key_pos, ':');
    if (!colon_pos) return -1;

    char* value_start = colon_pos + 1;
    while (*value_start && isspace(*value_start)) value_start++;
    
    return atoi(value_start);
}

static char** extract_string_array(const char* json, const char* key, int* count) {
    *count = 0;
    
    char search_key[256];
    snprintf(search_key, sizeof(search_key), "\"%s\"", key);
    
    char* key_pos = strstr(json, search_key);
    if (!key_pos) return NULL;
    
    char* array_start = strchr(key_pos, '[');
    if (!array_start) return NULL;
    
    char* array_end = strchr(array_start, ']');
    if (!array_end) return NULL;

    int element_count = 0;
    char* p = array_start;
    while (p < array_end) {
        if (*p == '"') {
            element_count++;
            p++;
            while (p < array_end && *p != '"') {
                if (*p == '\\' && *(p + 1)) {
                    p += 2;
                } else {
                    p++;
                }
            }
        }
        p++;
    }
    
    if (element_count == 0) return NULL;

    char** array = (char**)malloc(element_count * sizeof(char*));
    if (!array) return NULL;

    p = array_start;
    int i = 0;
    while (p < array_end && i < element_count) {
        if (*p == '"') {
            char* value_start = p + 1;
            char* value_end = value_start;

            while (value_end < array_end && *value_end != '"') {
                if (*value_end == '\\' && *(value_end + 1)) {
                    value_end += 2;
                } else {
                    value_end++;
                }
            }
            
            if (*value_end == '"') {
                size_t value_len = value_end - value_start;
                array[i] = (char*)malloc(value_len + 1);
                if (array[i]) {
                    strncpy(array[i], value_start, value_len);
                    array[i][value_len] = '\0';
                    i++;
                }
            }
        }
        p++;
    }
    
    *count = i;
    return array;
}

static char** extract_ingredient_names(const char* json, int* count) {
    *count = 0;
    
    char* ingredients_start = strstr(json, "\"ingredients\"");
    if (!ingredients_start) return NULL;
    
    char* array_start = strchr(ingredients_start, '[');
    if (!array_start) return NULL;
    
    char* array_end = NULL;
    int bracket_count = 1;
    char* p = array_start + 1;

    while (*p && bracket_count > 0) {
        if (*p == '[') bracket_count++;
        else if (*p == ']') bracket_count--;
        p++;
    }
    
    if (bracket_count != 0) return NULL;
    array_end = p - 1;

    int ingredient_count = 0;
    p = array_start;
    while (p < array_end) {
        if (*p == '{') ingredient_count++;
        p++;
    }
    
    if (ingredient_count == 0) return NULL;

    char** ingredients = (char**)malloc(ingredient_count * sizeof(char*));
    if (!ingredients) return NULL;

    p = array_start;
    int i = 0;
    while (p < array_end && i < ingredient_count) {
        if (*p == '{') {
            char* obj_start = p;
            int obj_bracket_count = 1;
            p++;
            
            while (p < array_end && obj_bracket_count > 0) {
                if (*p == '{') obj_bracket_count++;
                else if (*p == '}') obj_bracket_count--;
                p++;
            }
            
            char* obj_end = p;

            char obj_str[MAX_LINE_LENGTH];
            size_t obj_len = obj_end - obj_start;
            if (obj_len < MAX_LINE_LENGTH - 1) {
                strncpy(obj_str, obj_start, obj_len);
                obj_str[obj_len] = '\0';
                
                char* name = extract_string_value(obj_str, "name");
                if (name) {
                    ingredients[i++] = name;
                }
            }
        } else {
            p++;
        }
    }
    
    *count = i;
    return ingredients;
}

RecipeDB* init_recipe_db(const char* json_path) {
    RecipeDB* db = (RecipeDB*)malloc(sizeof(RecipeDB));
    if (!db) return NULL;
    
    db->recipes = NULL;
    db->recipe_count = 0;
    db->error_message = NULL;

    FILE* file = fopen(json_path, "r");
    if (!file) {
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg), "Failed to open JSON file: %s", json_path);
        db->error_message = str_duplicate(error_msg);
        return db;
    }

    char line[MAX_LINE_LENGTH];
    char* json_buffer = NULL;
    size_t buffer_size = 0;
    size_t buffer_capacity = 0;
    
    while (fgets(line, sizeof(line), file)) {
        size_t line_len = strlen(line);

        if (buffer_size + line_len + 1 > buffer_capacity) {
            buffer_capacity = buffer_capacity == 0 ? 16384 : buffer_capacity * 2;
            char* new_buffer = (char*)realloc(json_buffer, buffer_capacity);
            if (!new_buffer) {
                free(json_buffer);
                fclose(file);
                db->error_message = str_duplicate("Failed to allocate memory for JSON buffer");
                return db;
            }
            json_buffer = new_buffer;
        }

        strcpy(json_buffer + buffer_size, line);
        buffer_size += line_len;
    }
    
    fclose(file);
    
    if (!json_buffer) {
        db->error_message = str_duplicate("Empty JSON file");
        return db;
    }

    json_buffer[buffer_size] = '\0';

    char* meals_start = strstr(json_buffer, "\"meals\"");
    if (!meals_start) {
        free(json_buffer);
        db->error_message = str_duplicate("Failed to find meals array in JSON");
        return db;
    }
    
    char* array_start = strchr(meals_start, '[');
    if (!array_start) {
        free(json_buffer);
        db->error_message = str_duplicate("Invalid meals array format in JSON");
        return db;
    }

    int recipe_count = 0;
    char* p = array_start;
    while (*p) {
        if (*p == '{') recipe_count++;
        else if (*p == ']') break;
        p++;
    }
    
    if (recipe_count == 0) {
        free(json_buffer);
        db->error_message = str_duplicate("No recipes found in JSON");
        return db;
    }

    db->recipes = (Recipe*)malloc(recipe_count * sizeof(Recipe));
    if (!db->recipes) {
        free(json_buffer);
        db->error_message = str_duplicate("Failed to allocate memory for recipes");
        return db;
    }

    p = array_start;
    int i = 0;
    while (*p && i < recipe_count) {
        if (*p == '{') {
            char* obj_start = p;
            int obj_bracket_count = 1;
            p++;
            
            while (*p && obj_bracket_count > 0) {
                if (*p == '{') obj_bracket_count++;
                else if (*p == '}') obj_bracket_count--;
                p++;
            }
            
            char* obj_end = p;

            char recipe_str[MAX_LINE_LENGTH * 10];
            size_t obj_len = obj_end - obj_start;
            if (obj_len < sizeof(recipe_str) - 1) {
                strncpy(recipe_str, obj_start, obj_len);
                recipe_str[obj_len] = '\0';
                
                Recipe* recipe = &db->recipes[i];

                recipe->id = extract_string_value(recipe_str, "id");
                recipe->name = extract_string_value(recipe_str, "name");
                recipe->description = extract_string_value(recipe_str, "description");

                recipe->meal_type = extract_string_array(recipe_str, "meal_type", &recipe->meal_type_count);

                char* prep_time_str = strstr(recipe_str, "\"prep_time\"");
                if (prep_time_str) {
                    recipe->prep_time_duration = extract_int_value(prep_time_str, "duration");
                    recipe->prep_time_unit = extract_string_value(prep_time_str, "unit");
                } else {
                    recipe->prep_time_duration = 0;
                    recipe->prep_time_unit = str_duplicate("unknown");
                }
                
                char* cook_time_str = strstr(recipe_str, "\"cook_time\"");
                if (cook_time_str) {
                    recipe->cook_time_duration = extract_int_value(cook_time_str, "duration");
                    recipe->cook_time_unit = extract_string_value(cook_time_str, "unit");
                } else {
                    recipe->cook_time_duration = 0;
                    recipe->cook_time_unit = str_duplicate("unknown");
                }

                recipe->ingredients = extract_ingredient_names(recipe_str, &recipe->ingredients_count);

                recipe->preparation_steps = extract_string_array(recipe_str, "preparation_steps", &recipe->preparation_steps_count);

                char* sensory_str = strstr(recipe_str, "\"sensory_profile\"");
                if (sensory_str) {
                    recipe->sensory_texture = extract_string_array(sensory_str, "texture", &recipe->sensory_texture_count);
                    recipe->sensory_temperature = extract_string_array(sensory_str, "temperature", &recipe->sensory_temperature_count);
                    recipe->sensory_taste = extract_string_array(sensory_str, "taste", &recipe->sensory_taste_count);
                    recipe->sensory_smell = extract_string_array(sensory_str, "smell", &recipe->sensory_smell_count);
                } else {
                    recipe->sensory_texture = NULL;
                    recipe->sensory_texture_count = 0;
                    recipe->sensory_temperature = NULL;
                    recipe->sensory_temperature_count = 0;
                    recipe->sensory_taste = NULL;
                    recipe->sensory_taste_count = 0;
                    recipe->sensory_smell = NULL;
                    recipe->sensory_smell_count = 0;
                }
                
                i++;
            }
        } else {
            p++;
        }
    }
    
    db->recipe_count = i;
    free(json_buffer);
    
    return db;
}

void free_recipe_db(RecipeDB* db) {
    if (!db) return;
    
    if (db->recipes) {
        for (int i = 0; i < db->recipe_count; i++) {
            Recipe* recipe = &db->recipes[i];
            
            free(recipe->id);
            free(recipe->name);
            free(recipe->description);

            if (recipe->meal_type) {
                for (int j = 0; j < recipe->meal_type_count; j++) {
                    free(recipe->meal_type[j]);
                }
                free(recipe->meal_type);
            }

            if (recipe->ingredients) {
                for (int j = 0; j < recipe->ingredients_count; j++) {
                    free(recipe->ingredients[j]);
                }
                free(recipe->ingredients);
            }

            if (recipe->preparation_steps) {
                for (int j = 0; j < recipe->preparation_steps_count; j++) {
                    free(recipe->preparation_steps[j]);
                }
                free(recipe->preparation_steps);
            }

            if (recipe->sensory_texture) {
                for (int j = 0; j < recipe->sensory_texture_count; j++) {
                    free(recipe->sensory_texture[j]);
                }
                free(recipe->sensory_texture);
            }
            
            if (recipe->sensory_temperature) {
                for (int j = 0; j < recipe->sensory_temperature_count; j++) {
                    free(recipe->sensory_temperature[j]);
                }
                free(recipe->sensory_temperature);
            }
            
            if (recipe->sensory_taste) {
                for (int j = 0; j < recipe->sensory_taste_count; j++) {
                    free(recipe->sensory_taste[j]);
                }
                free(recipe->sensory_taste);
            }
            
            if (recipe->sensory_smell) {
                for (int j = 0; j < recipe->sensory_smell_count; j++) {
                    free(recipe->sensory_smell[j]);
                }
                free(recipe->sensory_smell);
            }
            
            free(recipe->prep_time_unit);
            free(recipe->cook_time_unit);
        }
        
        free(db->recipes);
    }
    
    free(db->error_message);
    free(db);
}

static Recipe* find_recipe_by_name(RecipeDB* db, const char* name) {
    if (!db || !name) return NULL;
    
    char* name_lower = str_to_lower(name);
    if (!name_lower) return NULL;
    
    Recipe* found_recipe = NULL;
    int best_match_score = 0;

    char* cleaned_name = name_lower;
    if (strncmp(cleaned_name, "a ", 2) == 0) cleaned_name += 2;
    else if (strncmp(cleaned_name, "an ", 3) == 0) cleaned_name += 3;
    else if (strncmp(cleaned_name, "the ", 4) == 0) cleaned_name += 4;

    printf("Searching for recipe: '%s' (cleaned: '%s')\n", name, cleaned_name);
    printf("Database has %d recipes\n", db->recipe_count);
    
    for (int i = 0; i < db->recipe_count; i++) {
        char* recipe_name_lower = str_to_lower(db->recipes[i].name);
        if (!recipe_name_lower) continue;

        printf("Comparing with: '%s'\n", db->recipes[i].name);

        if (strcmp(recipe_name_lower, cleaned_name) == 0) {
            found_recipe = &db->recipes[i];
            free(recipe_name_lower);
            break;
        }

        if (strstr(recipe_name_lower, cleaned_name) || strstr(cleaned_name, recipe_name_lower)) {
            int score = 100 - abs((int)strlen(recipe_name_lower) - (int)strlen(cleaned_name));

            if (score > best_match_score) {
                best_match_score = score;
                found_recipe = &db->recipes[i];
            }
        }
        
        free(recipe_name_lower);
    }
    
    free(name_lower);
    return found_recipe;
}

static QueryType determine_query_type(const char* query) {
    if (!query) return QUERY_UNKNOWN;
    
    char* query_lower = str_to_lower(query);
    if (!query_lower) return QUERY_UNKNOWN;
    
    QueryType type = QUERY_UNKNOWN;
    
    if (strstr(query_lower, "what is in") || 
        strstr(query_lower, "ingredients") || 
        strstr(query_lower, "what's in")) {
        type = QUERY_INGREDIENTS;
    } else if (strstr(query_lower, "how do i make") || 
               strstr(query_lower, "how to make") || 
               strstr(query_lower, "preparation") || 
               strstr(query_lower, "instructions") || 
               strstr(query_lower, "steps")) {
        type = QUERY_PREPARATION;
    } else if (strstr(query_lower, "texture") || 
               strstr(query_lower, "taste") || 
               strstr(query_lower, "smell") || 
               strstr(query_lower, "sensory") || 
               strstr(query_lower, "feel") || 
               strstr(query_lower, "temperature")) {
        type = QUERY_SENSORY;
    } else if (strstr(query_lower, "how long") || 
               strstr(query_lower, "time") || 
               strstr(query_lower, "duration") || 
               strstr(query_lower, "minutes") || 
               strstr(query_lower, "hours")) {
        type = QUERY_TIME;
    } else {
        type = QUERY_GENERAL;
    }
    
    free(query_lower);
    return type;
}

static char* extract_recipe_name(const char* query, QueryType query_type) {
    if (!query) return NULL;
    
    char* query_lower = str_to_lower(query);
    if (!query_lower) return NULL;
    
    char* recipe_name = NULL;
    char* start_pos = NULL;
    
    switch (query_type) {
        case QUERY_INGREDIENTS:
            if ((start_pos = strstr(query_lower, "what is in "))) {
                start_pos += 11;
            } else if ((start_pos = strstr(query_lower, "what's in "))) {
                start_pos += 10;
            } else if ((start_pos = strstr(query_lower, "ingredients in "))) {
                start_pos += 14;
            } else if ((start_pos = strstr(query_lower, "ingredients for "))) {
                start_pos += 16;
            }
            break;
            
        case QUERY_PREPARATION:
            if ((start_pos = strstr(query_lower, "how do i make "))) {
                start_pos += 14;
            } else if ((start_pos = strstr(query_lower, "how to make "))) {
                start_pos += 12;
            } else if ((start_pos = strstr(query_lower, "preparation for "))) {
                start_pos += 16;
            } else if ((start_pos = strstr(query_lower, "instructions for "))) {
                start_pos += 17;
            } else if ((start_pos = strstr(query_lower, "steps for "))) {
                start_pos += 10;
            }
            break;
            
        case QUERY_SENSORY:
            if ((start_pos = strstr(query_lower, "texture of "))) {
                start_pos += 11;
            } else if ((start_pos = strstr(query_lower, "taste of "))) {
                start_pos += 9;
            } else if ((start_pos = strstr(query_lower, "smell of "))) {
                start_pos += 9;
            } else if ((start_pos = strstr(query_lower, "sensory profile of "))) {
                start_pos += 19;
            } else if ((start_pos = strstr(query_lower, "feel of "))) {
                start_pos += 8;
            } else if ((start_pos = strstr(query_lower, "temperature of "))) {
                start_pos += 15;
            }
            break;
            
        case QUERY_TIME:
            if ((start_pos = strstr(query_lower, "how long to make "))) {
                start_pos += 16;
            } else if ((start_pos = strstr(query_lower, "time to make "))) {
                start_pos += 13;
            } else if ((start_pos = strstr(query_lower, "duration of "))) {
                start_pos += 12;
            } else if ((start_pos = strstr(query_lower, "how long does "))) {
                start_pos += 14;
                char* it_take = strstr(start_pos, "it take to make ");
                if (it_take == start_pos) {
                    start_pos += 16;
                }
            }
            break;
            
        case QUERY_GENERAL:y
            start_pos = query_lower;
            break;
            
        default:
            break;
    }
    
    if (start_pos) {
        size_t len = strlen(start_pos);

        while (len > 0 && (ispunct(start_pos[len-1]) || isspace(start_pos[len-1]))) {
            len--;
        }
        
        if (len > 0) {
            size_t offset = start_pos - query_lower;
            
            recipe_name = (char*)malloc(len + 1);
            if (recipe_name) {
                strncpy(recipe_name, query + offset, len);
                recipe_name[len] = '\0';
            }
        }
    }
    
    free(query_lower);
    return recipe_name;
}

static char* generate_ingredients_response(Recipe* recipe) {
    if (!recipe || !recipe->ingredients || recipe->ingredients_count == 0) {
        return str_duplicate("I couldn't find information about the ingredients for this recipe.");
    }
    
    char* response = (char*)malloc(MAX_RESPONSE_LENGTH);
    if (!response) return NULL;
    
    snprintf(response, MAX_RESPONSE_LENGTH, "The ingredients for %s are:\n", recipe->name);
    
    size_t offset = strlen(response);
    for (int i = 0; i < recipe->ingredients_count; i++) {
        size_t remaining = MAX_RESPONSE_LENGTH - offset;
        int written = snprintf(response + offset, remaining, "- %s\n", recipe->ingredients[i]);
        
        if (written < 0 || written >= (int)remaining) {
            strncat(response, "...", MAX_RESPONSE_LENGTH - offset - 1);
            break;
        }
        
        offset += written;
    }
    
    return response;
}

static char* generate_preparation_response(Recipe* recipe) {
    if (!recipe || !recipe->preparation_steps || recipe->preparation_steps_count == 0) {
        return str_duplicate("I couldn't find preparation instructions for this recipe.");
    }
    
    char* response = (char*)malloc(MAX_RESPONSE_LENGTH);
    if (!response) return NULL;
    
    snprintf(response, MAX_RESPONSE_LENGTH, "Here's how to make %s:\n", recipe->name);
    
    size_t offset = strlen(response);
    for (int i = 0; i < recipe->preparation_steps_count; i++) {
        size_t remaining = MAX_RESPONSE_LENGTH - offset;
        int written = snprintf(response + offset, remaining, "%d. %s\n", i + 1, recipe->preparation_steps[i]);
        
        if (written < 0 || written >= (int)remaining) {
            strncat(response, "...", MAX_RESPONSE_LENGTH - offset - 1);
            break;
        }
        
        offset += written;
    }
    
    return response;
}

static char* generate_sensory_response(Recipe* recipe) {
    if (!recipe) {
        return str_duplicate("I couldn't find sensory information for this recipe.");
    }
    
    char* response = (char*)malloc(MAX_RESPONSE_LENGTH);
    if (!response) return NULL;
    
    snprintf(response, MAX_RESPONSE_LENGTH, "Sensory profile for %s:\n", recipe->name);
    
    size_t offset = strlen(response);
    size_t remaining;
    int written;

    if (recipe->sensory_texture && recipe->sensory_texture_count > 0) {
        remaining = MAX_RESPONSE_LENGTH - offset;
        written = snprintf(response + offset, remaining, "Texture: ");
        if (written > 0 && written < (int)remaining) {
            offset += written;
            
            for (int i = 0; i < recipe->sensory_texture_count; i++) {
                remaining = MAX_RESPONSE_LENGTH - offset;
                written = snprintf(response + offset, remaining, 
                                  "%s%s", 
                                  recipe->sensory_texture[i],
                                  (i < recipe->sensory_texture_count - 1) ? ", " : "\n");
                
                if (written < 0 || written >= (int)remaining) {
                    strncat(response, "...\n", MAX_RESPONSE_LENGTH - offset - 1);
                    offset = strlen(response);
                    break;
                }
                
                offset += written;
            }
        }
    }

    if (recipe->sensory_temperature && recipe->sensory_temperature_count > 0) {
        remaining = MAX_RESPONSE_LENGTH - offset;
        written = snprintf(response + offset, remaining, "Temperature: ");
        if (written > 0 && written < (int)remaining) {
            offset += written;
            
            for (int i = 0; i < recipe->sensory_temperature_count; i++) {
                remaining = MAX_RESPONSE_LENGTH - offset;
                written = snprintf(response + offset, remaining, 
                                  "%s%s", 
                                  recipe->sensory_temperature[i],
                                  (i < recipe->sensory_temperature_count - 1) ? ", " : "\n");
                
                if (written < 0 || written >= (int)remaining) {
                    strncat(response, "...\n", MAX_RESPONSE_LENGTH - offset - 1);
                    offset = strlen(response);
                    break;
                }
                
                offset += written;
            }
        }
    }

    if (recipe->sensory_taste && recipe->sensory_taste_count > 0) {
        remaining = MAX_RESPONSE_LENGTH - offset;
        written = snprintf(response + offset, remaining, "Taste: ");
        if (written > 0 && written < (int)remaining) {
            offset += written;
            
            for (int i = 0; i < recipe->sensory_taste_count; i++) {
                remaining = MAX_RESPONSE_LENGTH - offset;
                written = snprintf(response + offset, remaining, 
                                  "%s%s", 
                                  recipe->sensory_taste[i],
                                  (i < recipe->sensory_taste_count - 1) ? ", " : "\n");
                
                if (written < 0 || written >= (int)remaining) {
                    strncat(response, "...\n", MAX_RESPONSE_LENGTH - offset - 1);
                    offset = strlen(response);
                    break;
                }
                
                offset += written;
            }
        }
    }

    if (recipe->sensory_smell && recipe->sensory_smell_count > 0) {
        remaining = MAX_RESPONSE_LENGTH - offset;
        written = snprintf(response + offset, remaining, "Smell: ");
        if (written > 0 && written < (int)remaining) {
            offset += written;
            
            for (int i = 0; i < recipe->sensory_smell_count; i++) {
                remaining = MAX_RESPONSE_LENGTH - offset;
                written = snprintf(response + offset, remaining, 
                                  "%s%s", 
                                  recipe->sensory_smell[i],
                                  (i < recipe->sensory_smell_count - 1) ? ", " : "\n");
                
                if (written < 0 || written >= (int)remaining) {
                    strncat(response, "...\n", MAX_RESPONSE_LENGTH - offset - 1);
                    offset = strlen(response);
                    break;
                }
                
                offset += written;
            }
        }
    }
    
    return response;
}

static char* generate_time_response(Recipe* recipe) {
    if (!recipe) {
        return str_duplicate("I couldn't find time information for this recipe.");
    }
    
    char* response = (char*)malloc(MAX_RESPONSE_LENGTH);
    if (!response) return NULL;
    
    snprintf(response, MAX_RESPONSE_LENGTH, "Time information for %s:\n", recipe->name);
    
    size_t offset = strlen(response);
    size_t remaining = MAX_RESPONSE_LENGTH - offset;
    int written;
    
    written = snprintf(response + offset, remaining, 
                      "Preparation time: %d %s\n", 
                      recipe->prep_time_duration, 
                      recipe->prep_time_unit);
    
    if (written > 0 && written < (int)remaining) {
        offset += written;
        remaining = MAX_RESPONSE_LENGTH - offset;
        
        written = snprintf(response + offset, remaining, 
                          "Cooking time: %d %s\n", 
                          recipe->cook_time_duration, 
                          recipe->cook_time_unit);
        
        if (written > 0 && written < (int)remaining) {
            offset += written;
            remaining = MAX_RESPONSE_LENGTH - offset;
            
            written = snprintf(response + offset, remaining, 
                              "Total time: %d %s\n", 
                              recipe->prep_time_duration + recipe->cook_time_duration, 
                              recipe->prep_time_unit);
        }
    }
    
    return response;
}

static char* generate_general_response(Recipe* recipe) {
    if (!recipe) {
        return str_duplicate("I couldn't find information about this recipe.");
    }
    
    char* response = (char*)malloc(MAX_RESPONSE_LENGTH);
    if (!response) return NULL;
    
    snprintf(response, MAX_RESPONSE_LENGTH, "About %s:\n", recipe->name);
    
    size_t offset = strlen(response);
    size_t remaining = MAX_RESPONSE_LENGTH - offset;
    int written;

    if (recipe->description) {
        written = snprintf(response + offset, remaining, "%s\n\n", recipe->description);
        
        if (written > 0 && written < (int)remaining) {
            offset += written;
            remaining = MAX_RESPONSE_LENGTH - offset;
        }
    }

    if (recipe->meal_type && recipe->meal_type_count > 0) {
        written = snprintf(response + offset, remaining, "Meal type: ");
        
        if (written > 0 && written < (int)remaining) {
            offset += written;
            remaining = MAX_RESPONSE_LENGTH - offset;
            
            for (int i = 0; i < recipe->meal_type_count; i++) {
                written = snprintf(response + offset, remaining, 
                                  "%s%s", 
                                  recipe->meal_type[i],
                                  (i < recipe->meal_type_count - 1) ? ", " : "\n");
                
                if (written < 0 || written >= (int)remaining) {
                    strncat(response, "...\n", MAX_RESPONSE_LENGTH - offset - 1);
                    offset = strlen(response);
                    break;
                }
                
                offset += written;
                remaining = MAX_RESPONSE_LENGTH - offset;
            }
        }
    }

    written = snprintf(response + offset, remaining, 
                      "Preparation time: %d %s\n", 
                      recipe->prep_time_duration, 
                      recipe->prep_time_unit);
    
    if (written > 0 && written < (int)remaining) {
        offset += written;
        remaining = MAX_RESPONSE_LENGTH - offset;
    }
    
    written = snprintf(response + offset, remaining, 
                      "Cooking time: %d %s\n\n", 
                      recipe->cook_time_duration, 
                      recipe->cook_time_unit);
    
    if (written > 0 && written < (int)remaining) {
        offset += written;
        remaining = MAX_RESPONSE_LENGTH - offset;
    }

    if (recipe->ingredients && recipe->ingredients_count > 0) {
        written = snprintf(response + offset, remaining, 
                          "Contains %d ingredients including ", 
                          recipe->ingredients_count);
        
        if (written > 0 && written < (int)remaining) {
            offset += written;
            remaining = MAX_RESPONSE_LENGTH - offset;

            int max_examples = recipe->ingredients_count < 3 ? recipe->ingredients_count : 3;
            
            for (int i = 0; i < max_examples; i++) {
                written = snprintf(response + offset, remaining, 
                                  "%s%s", 
                                  recipe->ingredients[i],
                                  (i < max_examples - 1) ? ", " : "");
                
                if (written < 0 || written >= (int)remaining) {
                    strncat(response, "...\n", MAX_RESPONSE_LENGTH - offset - 1);
                    offset = strlen(response);
                    break;
                }
                
                offset += written;
                remaining = MAX_RESPONSE_LENGTH - offset;
            }
            
            if (recipe->ingredients_count > 3) {
                written = snprintf(response + offset, remaining, " and others");
                
                if (written > 0 && written < (int)remaining) {
                    offset += written;
                    remaining = MAX_RESPONSE_LENGTH - offset;
                }
            }
            
            written = snprintf(response + offset, remaining, ".\n");
            
            if (written > 0 && written < (int)remaining) {
                offset += written;
                remaining = MAX_RESPONSE_LENGTH - offset;
            }
        }
    }

    if ((recipe->sensory_texture && recipe->sensory_texture_count > 0) ||
        (recipe->sensory_taste && recipe->sensory_taste_count > 0)) {
        
        written = snprintf(response + offset, remaining, "Sensory profile: ");
        
        if (written > 0 && written < (int)remaining) {
            offset += written;
            remaining = MAX_RESPONSE_LENGTH - offset;
            
            if (recipe->sensory_texture && recipe->sensory_texture_count > 0) {
                written = snprintf(response + offset, remaining, "Texture - ");
                
                if (written > 0 && written < (int)remaining) {
                    offset += written;
                    remaining = MAX_RESPONSE_LENGTH - offset;
                    
                    for (int i = 0; i < recipe->sensory_texture_count && i < 2; i++) {
                        written = snprintf(response + offset, remaining, 
                                          "%s%s", 
                                          recipe->sensory_texture[i],
                                          (i < recipe->sensory_texture_count - 1 && i < 1) ? ", " : "");
                        
                        if (written < 0 || written >= (int)remaining) {
                            strncat(response, "...", MAX_RESPONSE_LENGTH - offset - 1);
                            offset = strlen(response);
                            break;
                        }
                        
                        offset += written;
                        remaining = MAX_RESPONSE_LENGTH - offset;
                    }
                    
                    if (recipe->sensory_texture_count > 2) {
                        written = snprintf(response + offset, remaining, "...");
                        
                        if (written > 0 && written < (int)remaining) {
                            offset += written;
                            remaining = MAX_RESPONSE_LENGTH - offset;
                        }
                    }
                }
            }
            
            if (recipe->sensory_taste && recipe->sensory_taste_count > 0) {
                if (recipe->sensory_texture && recipe->sensory_texture_count > 0) {
                    written = snprintf(response + offset, remaining, ", Taste - ");
                } else {
                    written = snprintf(response + offset, remaining, "Taste - ");
                }
                
                if (written > 0 && written < (int)remaining) {
                    offset += written;
                    remaining = MAX_RESPONSE_LENGTH - offset;
                    
                    for (int i = 0; i < recipe->sensory_taste_count && i < 2; i++) {
                        written = snprintf(response + offset, remaining, 
                                          "%s%s", 
                                          recipe->sensory_taste[i],
                                          (i < recipe->sensory_taste_count - 1 && i < 1) ? ", " : "");
                        
                        if (written < 0 || written >= (int)remaining) {
                            strncat(response, "...", MAX_RESPONSE_LENGTH - offset - 1);
                            offset = strlen(response);
                            break;
                        }
                        
                        offset += written;
                        remaining = MAX_RESPONSE_LENGTH - offset;
                    }
                    
                    if (recipe->sensory_taste_count > 2) {
                        written = snprintf(response + offset, remaining, "...");
                        
                        if (written > 0 && written < (int)remaining) {
                            offset += written;
                            remaining = MAX_RESPONSE_LENGTH - offset;
                        }
                    }
                }
            }
            
            written = snprintf(response + offset, remaining, "\n");
            
            if (written > 0 && written < (int)remaining) {
                offset += written;
                remaining = MAX_RESPONSE_LENGTH - offset;
            }
        }
    }
    
    return response;
}

bool is_recipe_query(const char* query) {
    if (!query) return false;
    
    char* query_lower = str_to_lower(query);
    if (!query_lower) return false;

    if (strstr(query_lower, "what should i") || 
        strstr(query_lower, "what can i") ||
        strstr(query_lower, "what are") ||
        strstr(query_lower, "suggest") ||
        strstr(query_lower, "recommendation") ||
        strstr(query_lower, "recommend") ||
        strstr(query_lower, "ideas") ||
        strstr(query_lower, "options")) {
        free(query_lower);
        return false;
    }
    
    free(query_lower);

    QueryType type = determine_query_type(query);
    char* recipe_name = extract_recipe_name(query, type);
    
    if (!recipe_name) {
        return false;
    }

    char* name_lower = str_to_lower(recipe_name);
    bool is_generic = false;
    
    if (strcmp(name_lower, "dinner") == 0 ||
        strcmp(name_lower, "lunch") == 0 ||
        strcmp(name_lower, "breakfast") == 0 ||
        strcmp(name_lower, "meal") == 0 ||
        strcmp(name_lower, "food") == 0 ||
        strcmp(name_lower, "foods") == 0 ||
        strcmp(name_lower, "recipe") == 0 ||
        strcmp(name_lower, "recipes") == 0 ||
        strcmp(name_lower, "dish") == 0 ||
        strcmp(name_lower, "dishes") == 0) {
        is_generic = true;
    }
    
    free(name_lower);
    free(recipe_name);
    
    return !is_generic;
}

QueryResult process_recipe_query(RecipeDB* db, const char* query) {
    QueryResult result = {
        .success = false,
        .recipe_name = NULL,
        .query_type = QUERY_UNKNOWN,
        .response = NULL
    };
    
    if (!db || !query) {
        result.response = str_duplicate("Error: Invalid database or query.");
        return result;
    }

    QueryType type = determine_query_type(query);
    result.query_type = type;

    char* recipe_name = extract_recipe_name(query, type);
    if (!recipe_name) {
        result.response = str_duplicate("I couldn't identify a recipe in your query. Try asking about a specific recipe, like 'What is in Berry Blast Smoothie?'");
        return result;
    }
    
    result.recipe_name = recipe_name;

    Recipe* recipe = find_recipe_by_name(db, recipe_name);
    if (!recipe) {
        char* response = (char*)malloc(MAX_RESPONSE_LENGTH);
        if (response) {
            snprintf(response, MAX_RESPONSE_LENGTH, 
                    "I couldn't find a recipe for '%s'. Please try another recipe name.", 
                    recipe_name);
            result.response = response;
        } else {
            result.response = str_duplicate("I couldn't find that recipe.");
        }
        return result;
    }

    switch (type) {
        case QUERY_INGREDIENTS:
            result.response = generate_ingredients_response(recipe);
            break;
            
        case QUERY_PREPARATION:
            result.response = generate_preparation_response(recipe);
            break;
            
        case QUERY_SENSORY:
            result.response = generate_sensory_response(recipe);
            break;
            
        case QUERY_TIME:
            result.response = generate_time_response(recipe);
            break;
            
        case QUERY_GENERAL:
            result.response = generate_general_response(recipe);
            break;
            
        default:
            result.response = generate_general_response(recipe);
            break;
    }
    
    if (result.response) {
        result.success = true;
    } else {
        result.response = str_duplicate("Error generating response.");
        result.success = false;
    }
    
    return result;
}

void free_query_result(QueryResult* result) {
    if (!result) return;
    
    free(result->recipe_name);
    free(result->response);
    
    result->recipe_name = NULL;
    result->response = NULL;
}

const char* get_recipe_db_error(RecipeDB* db) {
    if (!db) return "Invalid database";
    return db->error_message;
}
