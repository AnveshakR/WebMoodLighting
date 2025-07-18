#!/bin/bash

set -e 

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

print_status() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

print_status "Checking for pyenv installation..."
if ! command -v pyenv &> /dev/null; then
    print_error "pyenv is not installed. Please install pyenv first."
    exit 1
fi
print_status "pyenv found!"

if [[ ! -f ".python-version" ]]; then
    print_warning ".python-version file not found."

    print_status "Creating virtual environment 'webled_env'..."
    python -m venv webled_env
 
    if [[ -f "requirements.txt" ]]; then
        print_status "Installing packages from requirements.txt..."
        ./webled_env/bin/pip install -r requirements.txt
    else
        print_warning "requirements.txt not found. Skipping package installation."
    fi
    
    PYTHON_PATH="./webled_env/bin/python"
else
    print_status ".python-version file found. Using pyenv python."
    PYTHON_PATH="python"
fi

if ! command -v tmux &> /dev/null; then
    print_error "tmux is not installed. Please install tmux first."
    exit 1
fi

print_status "Starting tmux session 'webled_env'..."
tmux new-session -d -s webled_env

if [[ ! -f ".python-version" ]]; then
    print_status "Activating virtual environment and running webapp.py..."
    tmux send-keys -t webled_env "source webled_env/bin/activate" Enter
else
    print_status "Running webapp.py with pyenv python..."
fi

if [[ ! -f "webapp.py" ]]; then
    print_error "webapp.py not found in current directory."
    tmux kill-session -t webled_env
    exit 1
fi

tmux send-keys -t webled_env "$PYTHON_PATH webapp.py" Enter

print_status "webapp.py is now running in tmux session 'webled_env'"
print_status "To attach to the session, run: tmux attach-session -t webled_env"
print_status "To detach from the session, press Ctrl+B then D"

SERVICE_NAME="webledctl"
print_status "Checking for systemctl service '$SERVICE_NAME'..."

if systemctl list-unit-files | grep -q "$SERVICE_NAME.service"; then
    print_status "Service '$SERVICE_NAME' already exists."
else
    print_warning "Service '$SERVICE_NAME' not found."
    read -p "Would you like to create a startup service? (y/n): " -n 1 -r
    echo
    
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        print_status "Creating systemctl service..."
  
        SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
        SCRIPT_PATH="$SCRIPT_DIR/$(basename "${BASH_SOURCE[0]}")"
 
        SERVICE_FILE="/etc/systemd/system/$SERVICE_NAME.service"
        
        sudo tee "$SERVICE_FILE" > /dev/null <<EOF
[Unit]
Description=WebLed Application Service
After=network.target

[Service]
Type=forking
User=$USER
WorkingDirectory=$SCRIPT_DIR
ExecStart=$SCRIPT_PATH
Restart=on-failure
RestartSec=10

[Install]
WantedBy=multi-user.target
EOF
        sudo systemctl daemon-reload
        sudo systemctl enable "$SERVICE_NAME"
        
        print_status "Service created and enabled!"
        print_status "To start the service: sudo systemctl start $SERVICE_NAME"
        print_status "To check status: sudo systemctl status $SERVICE_NAME"
        print_status "To stop the service: sudo systemctl stop $SERVICE_NAME"
    else
        print_status "Service creation skipped."
    fi
fi

print_status "Script completed successfully!"