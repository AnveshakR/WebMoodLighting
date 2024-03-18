from flask import Flask, render_template, request
import socket

app = Flask(__name__)

current_state = {
    'color_hex': '#010101',
    'display_type': 'solid'
}

def update_and_trigger():
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client_socket.connect(('localhost', 5555))
    client_socket.send(f'{current_state["color_hex"]} {current_state["display_type"]}'.encode('utf-8'))
    client_socket.close()


@app.route('/')
def index():
    return render_template('main.html')

@app.route('/update', methods=['POST'])
def led():
    if request.method == 'POST':
        if request.form.get('picker_input') != "#010101":
            current_state['color_hex'] = request.form.get('picker_input')

        if 'display_input' in request.form:
            current_state['display_type'] = request.form.get('display_input')
    
    update_and_trigger()    
    return render_template('main.html')

if __name__ == '__main__':
    app.run(host="0.0.0.0")
