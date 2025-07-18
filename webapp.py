from flask import Flask, render_template, request, jsonify
import os
import requests
from dotenv import load_dotenv

load_dotenv()

app = Flask(__name__)

# Configuration for multiple ESPs
ESP_CONFIG = {
    'desk': {
        'name': 'Desk Lights',
        'ip_entity': 'input_text.desk_esp_ip',
        'state_entity': 'input_text.desk_esp_state'
    },
    'wall': {
        'name': 'Wall Lights',
        'ip_entity': 'input_text.wall_esp_ip',
        'state_entity': 'input_text.wall_esp_state'
    }
}

# Default state for each ESP
current_states = {
    'desk': {
        'color_hex': '#010101',
        'display_type': 'solid'
    },
    'wall': {
        'color_hex': '#010101',
        'display_type': 'solid'
    }
}

HOME_ASSISTANT_BASE_URL = "http://10.0.0.50:8123/api/states/"
HOME_ASSISTANT_TOKEN = os.getenv("HA_TOKEN")

def get_ha_entity_state(entity_id):
    """Fetch current state from Home Assistant"""
    headers = {
        "Authorization": f"Bearer {HOME_ASSISTANT_TOKEN}",
        "Content-Type": "application/json"
    }
    
    try:
        response = requests.get(f"{HOME_ASSISTANT_BASE_URL}{entity_id}", headers=headers)
        response.raise_for_status()
        return response.json().get('state', '')
    except Exception as e:
        print(f"Error fetching HA state for {entity_id}: {e}")
        return ''

def update_ha_entity(entity_id, state_value):
    """Update Home Assistant entity state"""
    headers = {
        "Authorization": f"Bearer {HOME_ASSISTANT_TOKEN}",
        "Content-Type": "application/json"
    }

    data = {
        "state": str(state_value)
    }

    try:
        response = requests.post(f"{HOME_ASSISTANT_BASE_URL}{entity_id}", headers=headers, json=data)
        response.raise_for_status()
        return True
    except Exception as e:
        print(f"Error updating HA entity {entity_id}: {e}")
        return False

@app.route('/')
def index():
    return render_template('main.html', esp_config=ESP_CONFIG, current_states=current_states)

@app.route('/get_esp_states')
def get_esp_states():
    """Fetch current states for all ESPs from Home Assistant"""
    states = {}
    
    for esp_id, config in ESP_CONFIG.items():
        # Get current state from Home Assistant
        ha_state = get_ha_entity_state(config['state_entity'])
        
        # Try to parse the state as a dictionary-like string
        try:
            # If the state is stored as a string representation of a dict
            if ha_state and ha_state.startswith('{'):
                import ast
                parsed_state = ast.literal_eval(ha_state)
                if isinstance(parsed_state, dict):
                    current_states[esp_id].update(parsed_state)
        except:
            pass
        
        states[esp_id] = {
            'current_state': current_states[esp_id],
            'ha_state': ha_state,
            'ip': get_ha_entity_state(config['ip_entity'])
        }
    
    return jsonify(states)

@app.route('/update', methods=['POST'])
def update():
    data = request.json
    esp_id = data.get('esp_id')
    
    if not esp_id or esp_id not in ESP_CONFIG:
        return jsonify(success=False, error="Invalid ESP ID"), 400
    
    # Update local state
    esp_data = {k: v for k, v in data.items() if k != 'esp_id'}
    current_states[esp_id].update(esp_data)
    
    # Update Home Assistant
    config = ESP_CONFIG[esp_id]
    success = update_ha_entity(config['state_entity'], current_states[esp_id])
    
    return jsonify(success=success)

if __name__ == '__main__':
    port = int(os.getenv("FLASK_PORT", 5000))

    url = f"http://localhost:{port}/"
    with open('curr_url', 'w') as file:
        file.write(url)

    app.run(host="0.0.0.0", port=port)
