#!/bin/bash

virtual_env_name="webled_env"
. ${PYENV_ROOT}/versions/$virtual_env_name/bin/activate
tmux new-session -d -s web_led

tmux split-window -h

tmux select-pane -t web_led:0.0
# tmux send-keys -t web_led:0.0 "pyenv activate $virtual_env_name" Enter
tmux send-keys -t web_led:0.0 "sudo $(pyenv which python) display_loop.py" C-m

tmux select-pane -t web_led:0.1
# tmux send-keys -t web_led:0.1 "pyenv activate $virtual_env_name" C-m
tmux send-keys -t web_led:0.1 "$(pyenv which python) webapp.py" C-m

tmux attach-session -t web_led
