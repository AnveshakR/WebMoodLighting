# LED_WEB

Same project, but adapted to Python to run on a Raspberry Pi.
Webserver written with Flask.

Recommended GPIO pins would be any of the PCM pins.

Run `sh run_scripts.sh` to run the tmux environment. Obviously you'll need tmux installed.
You'll also need to create a pyenv-virtualenv with the name webled_env and install the libraries in the requirements.txt. I'll look into automating this process.

raspi_ws281x needs sudo access to run, so you'll need to enter your password in the first tmux pane. 