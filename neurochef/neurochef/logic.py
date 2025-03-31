#!/usr/bin/env python3
"""
NeuroChef Logic Module

This module handles the core logic of the NeuroChef chatbot,
including keyword matching and response generation.
"""

import json
import sys
import os

def load_data():
    """Load meal data from JSON file."""
    script_dir = os.path.dirname(os.path.abspath(__file__))
    json_path = os.path.join(os.path.dirname(script_dir), "meal_data.json")
    
    with open(json_path, 'r') as file:
        return json.load(file)

def find_matches(user_input, data):
    """Find matches in the data based on user input."""
    user_input = user_input.lower()
    response = ""
    
    # Check for sensory-related keywords
    if any(word in user_input for word in ["texture", "sensory", "smooth", "soft", "crunchy"]):
        if "smooth" in user_input:
            smooth_meals = [meal["name"] for meal in data["meals"] if "smooth" in meal["sensory"]]
            if smooth_meals:
                response += f"For smooth textures, you might enjoy: {', '.join(smooth_meals)}.\n"
        
        if "soft" in user_input:
            soft_meals = [meal["name"] for meal in data["meals"] if "soft" in meal["sensory"]]
            if soft_meals:
                response += f"For soft textures, you might enjoy: {', '.join(soft_meals)}.\n"
        
        if "crunchy" in user_input:
            response += "I notice you mentioned crunchy textures. Some neurodivergent individuals avoid: "
            response += f"{', '.join(data['sensory_avoidance']['crunchy'])}.\n"
            response += f"Instead, you might prefer: {', '.join(data['sensory_suggestions']['crunchy'])}.\n"
        
        # General sensory response if no specific texture mentioned
        if not response:
            response = "I can help with meal suggestions based on sensory preferences. "
            response += "Try asking about specific textures like 'smooth', 'soft', or 'crunchy'."
    
    # Check for time-related keywords
    elif any(word in user_input for word in ["quick", "fast", "time", "minutes"]):
        quick_meals = [f"{meal['name']} ({meal['prep_time']})" for meal in data["meals"] if "minutes" in meal["prep_time"] and int(meal["prep_time"].split()[0]) <= 15]
        if quick_meals:
            response = f"Here are some quick meals: {', '.join(quick_meals)}."
        else:
            response = "I don't have any quick meals in my database yet."
    
    # Check for executive function related keywords
    elif any(word in user_input for word in ["planning", "remember", "executive", "function"]):
        if "planning" in user_input:
            suggestions = data["executive_function_challenges"]["difficulty_planning"]
            response = f"For difficulty with planning, consider: {', '.join(suggestions)}."
        elif "remember" in user_input:
            suggestions = data["executive_function_challenges"]["difficulty_remembering"]
            response = f"For difficulty with remembering, consider: {', '.join(suggestions)}."
        else:
            response = "I can help with executive function challenges. Try asking about 'planning' or 'remembering'."
    
    # Default response
    else:
        response = "I'm NeuroChef, here to help with meal planning for neurodivergent needs. "
        response += "You can ask me about sensory preferences (textures), quick meals, or executive function challenges."
    
    return response

def main():
    """Main function to process input and return a response."""
    if len(sys.argv) < 2:
        return "Please provide some input."
    
    user_input = " ".join(sys.argv[1:])
    
    # Handle exit command
    if user_input.lower() in ["exit", "quit"]:
        return "Goodbye! Take care."
    
    # Process normal input
    data = load_data()
    return find_matches(user_input, data)

if __name__ == "__main__":
    result = main()
    print(result)
