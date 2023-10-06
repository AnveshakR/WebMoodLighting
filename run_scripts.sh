#!/bin/bash

virtual_env_name="webled_env"

tmux new-session -d -s web_led

tmux split-window -h

tmux send-keys -t web_led:0.0 "pyenv activate $virtual_env_name" C-m

tmux select-pane -t web_led:0.1

tmux send-keys -t web_led:0.1 "pyenv activate $virtual_env_name" C-m

tmux send-keys -t web_led:0.0 "sudo python display_loop.py" C-m

tmux select-pane -t web_led:0.1

tmux send-keys -t web_led:0.1 "python webapp.py" C-m

tmux attach-session -t web_led
