const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
  <head>
    <title>LED Control</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
      html {
        font-family: Sans-Serif;
        display: block;
        background-color: #222831;
        color: #EEEEEE;
        text-align: center
      }

      p {
        color: #EEEEEE;
      }

      h2 {
        font-size: 3.0rem;
        color: #7BC74D
      }

      input {
        background-color: #393E46;
        color: #EEEEEE
      }

      button {
        background-color: #393E46;
        color: #EEEEEE
      }

      form {
        padding: 15px
      }
      
      div.borderdiv{
        margin:auto;
        text-align:center;
        border-style:double;
        width: 50%;
      }

      ;
    </style>
  </head>
  <body>
    <h2>LED Control</h2>
    
    <div class="borderdiv">
    
    <form action="/">
      <p>Pick a color:</p><input type="color" name="picker_input" id="picker" />
      <br>
      <br>
      <p>Select display type:</p>
      <input type="radio" id="solid" name="display_input" value="0">Solid Color <br>
      <br>
      <input type="radio" id="breathing" name="display_input" value="1">Breathing <br>
      <br>
      <input type="radio" id="basicAV" name="display_input" value="2">Basic Audio-Visualizer <br>
      <br>
      <br>
      <input type="submit" value="Submit">
    </form>

    </div>

  </body>
</html>)rawliteral";