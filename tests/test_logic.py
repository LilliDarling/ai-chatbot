"""
Tests for the NeuroChef logic module.
"""

import sys
import os

# Add the parent directory to the path so we can import the module
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))

from neurochef.logic import find_matches

# Mock data for testing
mock_data = {
    "meals": [
        {
            "name": "Smoothie",
            "sensory_profile": {
                "texture": ["smooth"],
                "temperature": ["cold"],
                "taste": ["sweet"]
            },
            "prep_time": {
                "duration": 5,
                "unit": "minutes"
            },
            "description": "A quick and easy meal."
        }
    ],
    "sensory_considerations": {
        "avoidance_triggers": {
            "texture": ["crunchy", "slimy", "chewy"]
        },
        "preferred_sensory_profiles": {
            "texture": ["smooth", "soft", "creamy"]
        },
        "texture_mapping": {
            "crunchy": ["consider finely chopped or pureed alternatives", "offer a soft accompaniment"]
        }
    },
    "executive_function_support_strategies": {
        "difficulty_planning": ["meal prepping", "theme days"],
        "memory_challenges": ["meal reminders", "written recipes"]
    }
}

def test_smooth_texture():
    """Test response for smooth texture query."""
    response = find_matches("I need smooth texture", mock_data)
    assert "Smoothie" in response

def test_quick_meal():
    """Test response for quick meal query."""
    response = find_matches("I need a quick meal", mock_data)
    assert "Smoothie" in response
    assert "5 minutes" in response

def test_planning():
    """Test response for planning difficulty query."""
    response = find_matches("I have difficulty planning", mock_data)
    assert "meal prepping" in response

def test_default_response():
    """Test default response for unrecognized query."""
    response = find_matches("hello", mock_data)
    assert "NeuroChef" in response
