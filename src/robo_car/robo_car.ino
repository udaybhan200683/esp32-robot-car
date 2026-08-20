#include <WiFi.h>
#include <WebServer.h>

// ============================================================
// ESP32 ROBOT CAR
// Touch Joystick + Vertical Speed Accelerator
// ============================================================
//
// TB6612FNG
//
// LEFT MOTOR / A
// PWMA -> GPIO25
// AIN1 -> GPIO26
// AIN2 -> GPIO27
//
// RIGHT MOTOR / B
// PWMB -> GPIO14
// BIN1 -> GPIO16
// BIN2 -> GPIO17
//
// STBY -> GPIO33
//
// ============================================================


// ============================================================
// WIFI
// ============================================================

const char* AP_SSID = "RobotCar";
const char* AP_PASSWORD = "12345678";

WebServer server(80);


// ============================================================
// MOTOR GPIO
// ============================================================

const int PWMA = 25;
const int AIN1 = 26;
const int AIN2 = 27;

const int PWMB = 14;
const int BIN1 = 16;
const int BIN2 = 17;

const int STBY = 33;


// ============================================================
// SPEED
// ============================================================

int motorSpeed = 80;

const int MIN_SPEED = 40;
const int MAX_SPEED = 255;


// ============================================================
// CURRENT COMMAND
// ============================================================

String currentCommand = "STOP";


// ============================================================
// STOP
// ============================================================

void stopMotors()
{
  analogWrite(PWMA, 0);
  analogWrite(PWMB, 0);

  digitalWrite(AIN1, LOW);
  digitalWrite(AIN2, LOW);

  digitalWrite(BIN1, LOW);
  digitalWrite(BIN2, LOW);

  currentCommand = "STOP";
}


// ============================================================
// FORWARD
// ============================================================

void moveForward()
{
  digitalWrite(AIN1, HIGH);
  digitalWrite(AIN2, LOW);

  digitalWrite(BIN1, HIGH);
  digitalWrite(BIN2, LOW);

  analogWrite(PWMA, motorSpeed);
  analogWrite(PWMB, motorSpeed);

  currentCommand = "FORWARD";
}


// ============================================================
// BACKWARD
// ============================================================

void moveBackward()
{
  digitalWrite(AIN1, LOW);
  digitalWrite(AIN2, HIGH);

  digitalWrite(BIN1, LOW);
  digitalWrite(BIN2, HIGH);

  analogWrite(PWMA, motorSpeed);
  analogWrite(PWMB, motorSpeed);

  currentCommand = "BACKWARD";
}


// ============================================================
// LEFT
// ============================================================

void turnLeft()
{
  digitalWrite(AIN1, LOW);
  digitalWrite(AIN2, HIGH);

  digitalWrite(BIN1, HIGH);
  digitalWrite(BIN2, LOW);

  analogWrite(PWMA, motorSpeed);
  analogWrite(PWMB, motorSpeed);

  currentCommand = "LEFT";
}


// ============================================================
// RIGHT
// ============================================================

void turnRight()
{
  digitalWrite(AIN1, HIGH);
  digitalWrite(AIN2, LOW);

  digitalWrite(BIN1, LOW);
  digitalWrite(BIN2, HIGH);

  analogWrite(PWMA, motorSpeed);
  analogWrite(PWMB, motorSpeed);

  currentCommand = "RIGHT";
}


// ============================================================
// APPLY CURRENT MOVEMENT
// ============================================================

void applyCurrentMovement()
{
  if (currentCommand == "FORWARD")
  {
    moveForward();
  }
  else if (currentCommand == "BACKWARD")
  {
    moveBackward();
  }
  else if (currentCommand == "LEFT")
  {
    turnLeft();
  }
  else if (currentCommand == "RIGHT")
  {
    turnRight();
  }
}


// ============================================================
// HTML PAGE
// ============================================================

const char MAIN_PAGE[] PROGMEM = R"rawliteral(

<!DOCTYPE html>

<html>

<head>

<meta charset="UTF-8">

<meta
  name="viewport"
  content="width=device-width,
  initial-scale=1.0,
  maximum-scale=1.0,
  user-scalable=no"
>

<title>Robot Car</title>


<style>

/* ============================================================
   PAGE
   ============================================================ */

html,
body
{
  width: 100%;
  height: 100%;

  margin: 0;
  padding: 0;
}


body
{
  background: #101010;

  color: white;

  font-family: Arial, sans-serif;

  overflow: hidden;

  user-select: none;
  -webkit-user-select: none;

  touch-action: none;
}


/* ============================================================
   HEADER
   ============================================================ */

.header
{
  text-align: center;

  height: 65px;

  padding-top: 8px;

  box-sizing: border-box;
}


.title
{
  font-size: 25px;

  font-weight: bold;
}


#status
{
  font-size: 18px;

  font-weight: bold;

  color: #00ff88;

  margin-top: 3px;
}


/* ============================================================
   MAIN AREA
   ============================================================ */

.main
{
  position: absolute;

  left: 0;
  right: 0;

  top: 65px;
  bottom: 0;
}


/* ============================================================
   JOYSTICK
   ============================================================ */

.joystick
{
  position: absolute;

  left: 50%;

  bottom: 18px;

  transform: translateX(-50%);

  width: 230px;

  height: 230px;

  border-radius: 50%;

  background:
    radial-gradient(
      circle,
      #333 0%,
      #242424 60%,
      #171717 100%
    );

  border: 4px solid #666;

  box-sizing: border-box;

  touch-action: none;

  z-index: 5;

  box-shadow:
    0 0 20px rgba(0,0,0,0.7);
}


/* ============================================================
   JOYSTICK CROSS
   ============================================================ */

.joystick::before
{
  content: "";

  position: absolute;

  top: 10px;
  bottom: 10px;

  left: 50%;

  width: 1px;

  background: #444;
}


.joystick::after
{
  content: "";

  position: absolute;

  left: 10px;
  right: 10px;

  top: 50%;

  height: 1px;

  background: #444;
}


/* ============================================================
   CENTER
   ============================================================ */

.center
{
  position: absolute;

  left: 50%;
  top: 50%;

  width: 34px;
  height: 34px;

  transform:
    translate(-50%, -50%);

  border-radius: 50%;

  background: #444;

  border: 2px solid #777;

  pointer-events: none;
}


/* ============================================================
   DIRECTION LABELS
   ============================================================ */

.jlabel
{
  position: absolute;

  color: #aaa;

  font-size: 14px;

  font-weight: bold;

  pointer-events: none;
}


.jup
{
  top: 10px;

  left: 50%;

  transform:
    translateX(-50%);
}


.jdown
{
  bottom: 10px;

  left: 50%;

  transform:
    translateX(-50%);
}


.jleft
{
  left: 10px;

  top: 50%;

  transform:
    translateY(-50%);
}


.jright
{
  right: 10px;

  top: 50%;

  transform:
    translateY(-50%);
}


/* ============================================================
   FINGER INDICATOR
   ============================================================ */

#finger
{
  position: absolute;

  left: 50%;
  top: 50%;

  width: 48px;
  height: 48px;

  transform:
    translate(-50%, -50%);

  border-radius: 50%;

  background: #00aa66;

  border: 3px solid #00ff99;

  display: none;

  pointer-events: none;
}


/* ============================================================
   SPEED CONTROL AREA
   ============================================================ */

.speedArea
{
  position: absolute;

  right: 12px;

  top: 50%;

  transform:
    translateY(-50%);

  width: 85px;

  height: 360px;

  display: flex;

  flex-direction: column;

  align-items: center;

  justify-content: center;

  z-index: 10;

  touch-action: none;
}


/* ============================================================
   SPEED TITLE
   ============================================================ */

.speedTitle
{
  font-size: 13px;

  font-weight: bold;

  color: #00ff99;

  margin-bottom: 7px;
}


/* ============================================================
   SPEED TOUCH PAD
   ============================================================ */

#speedPad
{
  position: relative;

  width: 70px;

  height: 280px;

  border-radius: 35px;

  background: #202020;

  border: 2px solid #555;

  box-sizing: border-box;

  touch-action: none;

  /*
     IMPORTANT:
     Large touch area.
  */

  padding: 15px;
}


/* ============================================================
   INNER TRACK
   ============================================================ */

#speedTrack
{
  position: absolute;

  left: 50%;

  top: 15px;

  transform:
    translateX(-50%);

  width: 14px;

  height: 250px;

  border-radius: 8px;

  background: #444;

  pointer-events: none;
}


/* ============================================================
   ACTIVE SPEED
   ============================================================ */

#speedFill
{
  position: absolute;

  left: 0;

  bottom: 0;

  width: 100%;

  height: 40%;

  border-radius: 8px;

  background:
    linear-gradient(
      to top,
      #008844,
      #00ff99
    );

  pointer-events: none;
}


/* ============================================================
   ACCELERATOR PADDLE
   ============================================================ */

#speedPaddle
{
  position: absolute;

  left: 50%;

  bottom: 40%;

  transform:
    translate(-50%, 50%);

  width: 58px;

  height: 28px;

  border-radius: 14px;

  background: #00dd88;

  border: 3px solid white;

  box-sizing: border-box;

  box-shadow:
    0 0 12px #00ff99;

  pointer-events: none;
}


/* ============================================================
   SPEED VALUE
   ============================================================ */

#speedValue
{
  margin-top: 8px;

  font-size: 17px;

  font-weight: bold;

  color: white;
}


/* ============================================================
   MAX / MIN
   ============================================================ */

.speedMax,
.speedMin
{
  font-size: 11px;

  color: #888;
}


.speedMax
{
  margin-bottom: 2px;
}


.speedMin
{
  margin-top: 2px;
}


/* ============================================================
   SMALL SCREEN
   ============================================================ */

@media (max-height: 650px)
{

  .joystick
  {
    width: 200px;
    height: 200px;
  }


  .speedArea
  {
    height: 300px;
  }


  #speedPad
  {
    height: 230px;
  }


  #speedTrack
  {
    height: 200px;
  }


  .main
  {
    top: 60px;
  }

}


/* ============================================================
   VERY SMALL SCREEN
   ============================================================ */

@media (max-height: 520px)
{

  .joystick
  {
    width: 175px;
    height: 175px;
  }


  .speedArea
  {
    right: 5px;

    width: 70px;

    height: 250px;
  }


  #speedPad
  {
    width: 58px;

    height: 190px;
  }


  #speedTrack
  {
    height: 165px;
  }


  #speedPaddle
  {
    width: 50px;
    height: 24px;
  }

}

</style>

</head>


<body>


<!-- ========================================================
     HEADER
     ======================================================== -->

<div class="header">

  <div class="title">
    ROBOT CAR
  </div>

  <div id="status">
    STOP
  </div>

</div>


<!-- ========================================================
     MAIN
     ======================================================== -->

<div class="main">


  <!-- ======================================================
       JOYSTICK
       ====================================================== -->

  <div
    id="joystick"
    class="joystick"
  >

    <div class="jlabel jup">
      ▲
    </div>

    <div class="jlabel jdown">
      ▼
    </div>

    <div class="jlabel jleft">
      ◀
    </div>

    <div class="jlabel jright">
      ▶
    </div>

    <div class="center">
    </div>

    <div id="finger">
    </div>

  </div>


  <!-- ======================================================
       SPEED ACCELERATOR
       ====================================================== -->

  <div
    id="speedArea"
    class="speedArea"
  >

    <div class="speedTitle">
      SPEED
    </div>

    <div class="speedMax">
      MAX
    </div>


    <!-- LARGE TOUCH AREA -->

    <div id="speedPad">

      <div id="speedTrack">

        <div id="speedFill">
        </div>

      </div>


      <div id="speedPaddle">
      </div>

    </div>


    <div id="speedValue">
      31%
    </div>

    <div class="speedMin">
      MIN
    </div>

  </div>


</div>


<script>


// ============================================================
// JOYSTICK VARIABLES
// ============================================================

const joystick =
  document.getElementById(
    "joystick"
  );


const finger =
  document.getElementById(
    "finger"
  );


const statusText =
  document.getElementById(
    "status"
  );


let joystickPointer = null;

let lastCommand = "S";

const DEAD_ZONE = 25;


// ============================================================
// JOYSTICK CENTER
// ============================================================

function getJoystickCenter()
{
  const r =
    joystick.getBoundingClientRect();

  return {
    x: r.left + r.width / 2,
    y: r.top + r.height / 2
  };
}


// ============================================================
// SEND COMMAND
// ============================================================

function sendCommand(command)
{
  if (
    command ===
    lastCommand
  )
  {
    return;
  }


  lastCommand =
    command;


  fetch(
    "/cmd?c=" +
    command
  )
  .then(
    response =>
      response.text()
  )
  .then(
    text =>
    {
      statusText.innerText =
        text;
    }
  )
  .catch(
    () =>
    {
      statusText.innerText =
        "ERROR";
    }
  );
}


// ============================================================
// JOYSTICK PROCESS
// ============================================================

function processJoystick(
  x,
  y
)
{
  const center =
    getJoystickCenter();


  const dx =
    x - center.x;


  const dy =
    y - center.y;


  const distance =
    Math.sqrt(
      dx * dx +
      dy * dy
    );


  // CENTER

  if (
    distance <
    DEAD_ZONE
  )
  {
    finger.style.display =
      "block";


    finger.style.left =
      "50%";


    finger.style.top =
      "50%";


    sendCommand("S");

    return;
  }


  // Finger graphic

  const radius =
    joystick.clientWidth / 2 -
    30;


  let fx = dx;

  let fy = dy;


  if (
    distance >
    radius
  )
  {
    fx =
      dx / distance *
      radius;


    fy =
      dy / distance *
      radius;
  }


  finger.style.display =
    "block";


  finger.style.left =
    "calc(50% + " +
    fx +
    "px)";


  finger.style.top =
    "calc(50% + " +
    fy +
    "px)";


  // Direction

  if (
    Math.abs(dx) >
    Math.abs(dy)
  )
  {
    if (dx > 0)
    {
      sendCommand("R");
    }
    else
    {
      sendCommand("L");
    }
  }
  else
  {
    if (dy > 0)
    {
      sendCommand("B");
    }
    else
    {
      sendCommand("F");
    }
  }
}


// ============================================================
// JOYSTICK DOWN
// ============================================================

joystick.addEventListener(
  "pointerdown",
  function(event)
  {
    event.preventDefault();


    joystickPointer =
      event.pointerId;


    joystick.setPointerCapture(
      event.pointerId
    );


    processJoystick(
      event.clientX,
      event.clientY
    );
  }
);


// ============================================================
// JOYSTICK MOVE
// ============================================================

joystick.addEventListener(
  "pointermove",
  function(event)
  {
    event.preventDefault();


    if (
      event.pointerId !==
      joystickPointer
    )
    {
      return;
    }


    processJoystick(
      event.clientX,
      event.clientY
    );
  }
);


// ============================================================
// JOYSTICK UP
// ============================================================

joystick.addEventListener(
  "pointerup",
  function(event)
  {
    if (
      event.pointerId !==
      joystickPointer
    )
    {
      return;
    }


    joystickPointer =
      null;


    finger.style.display =
      "none";


    sendCommand("S");
  }
);


// ============================================================
// JOYSTICK CANCEL
// ============================================================

joystick.addEventListener(
  "pointercancel",
  function()
  {
    joystickPointer =
      null;


    finger.style.display =
      "none";


    sendCommand("S");
  }
);


// ============================================================
// SPEED VARIABLES
// ============================================================

const speedArea =
  document.getElementById(
    "speedArea"
  );


const speedPad =
  document.getElementById(
    "speedPad"
  );


const speedFill =
  document.getElementById(
    "speedFill"
  );


const speedPaddle =
  document.getElementById(
    "speedPaddle"
  );


const speedValue =
  document.getElementById(
    "speedValue"
  );


let speedPointer =
  null;


// ============================================================
// SET SPEED
// ============================================================

function setSpeed(
  clientY
)
{
  const rect =
    speedPad.getBoundingClientRect();


  let y =
    clientY -
    rect.top;


  // Limit

  if (y < 0)
  {
    y = 0;
  }


  if (
    y >
    rect.height
  )
  {
    y =
      rect.height;
  }


  // ----------------------------------------------------------
  // TOP = 100%
  // BOTTOM = 0%
  // ----------------------------------------------------------

  let percent =
    1 -
    y / rect.height;


  if (percent < 0)
  {
    percent = 0;
  }


  if (percent > 1)
  {
    percent = 1;
  }


  // ----------------------------------------------------------
  // SPEED VALUE
  // ----------------------------------------------------------

  const speed =
    Math.round(
      40 +
      percent *
      (255 - 40)
    );


  const displayPercent =
    Math.round(
      (speed / 255) *
      100
    );


  // ----------------------------------------------------------
  // UPDATE GRAPHICS
  // ----------------------------------------------------------

  speedFill.style.height =
    (percent * 100) +
    "%";


  speedPaddle.style.bottom =
    (percent * 100) +
    "%";


  speedValue.innerText =
    displayPercent +
    "%";


  // ----------------------------------------------------------
  // SEND TO ESP32
  // ----------------------------------------------------------

  fetch(
    "/speed?v=" +
    speed
  );
}


// ============================================================
// SPEED DOWN
// ============================================================

speedPad.addEventListener(
  "pointerdown",
  function(event)
  {
    event.preventDefault();


    speedPointer =
      event.pointerId;


    speedPad.setPointerCapture(
      event.pointerId
    );


    setSpeed(
      event.clientY
    );
  }
);


// ============================================================
// SPEED MOVE
// ============================================================

speedPad.addEventListener(
  "pointermove",
  function(event)
  {
    event.preventDefault();


    if (
      event.pointerId !==
      speedPointer
    )
    {
      return;
    }


    setSpeed(
      event.clientY
    );
  }
);


// ============================================================
// SPEED UP
// ============================================================

speedPad.addEventListener(
  "pointerup",
  function(event)
  {
    if (
      event.pointerId ===
      speedPointer
    )
    {
      speedPointer =
        null;
    }
  }
);


// ============================================================
// SPEED CANCEL
// ============================================================

speedPad.addEventListener(
  "pointercancel",
  function()
  {
    speedPointer =
      null;
  }
);


</script>

</body>

</html>

)rawliteral";


// ============================================================
// WEB ROOT
// ============================================================

void handleRoot()
{
  server.send(
    200,
    "text/html",
    MAIN_PAGE
  );
}


// ============================================================
// COMMAND HANDLER
// ============================================================

void handleCommand()
{
  if (
    !server.hasArg("c")
  )
  {
    server.send(
      400,
      "text/plain",
      "Invalid command"
    );

    return;
  }


  String command =
    server.arg("c");


  if (command == "F")
  {
    moveForward();
  }
  else if (command == "B")
  {
    moveBackward();
  }
  else if (command == "L")
  {
    turnLeft();
  }
  else if (command == "R")
  {
    turnRight();
  }
  else
  {
    stopMotors();
  }


  server.send(
    200,
    "text/plain",
    currentCommand
  );
}


// ============================================================
// SPEED HANDLER
// ============================================================

void handleSpeed()
{
  if (
    !server.hasArg("v")
  )
  {
    server.send(
      400,
      "text/plain",
      "Invalid speed"
    );

    return;
  }


  motorSpeed =
    server.arg("v").toInt();


  // Safety limits

  if (
    motorSpeed <
    MIN_SPEED
  )
  {
    motorSpeed =
      MIN_SPEED;
  }


  if (
    motorSpeed >
    MAX_SPEED
  )
  {
    motorSpeed =
      MAX_SPEED;
  }


  // Immediately apply new speed
  // if the car is moving.

  applyCurrentMovement();


  server.send(
    200,
    "text/plain",
    String(motorSpeed)
  );
}


// ============================================================
// SETUP
// ============================================================

void setup()
{
  Serial.begin(
    115200
  );


  // Motor A

  pinMode(
    PWMA,
    OUTPUT
  );

  pinMode(
    AIN1,
    OUTPUT
  );

  pinMode(
    AIN2,
    OUTPUT
  );


  // Motor B

  pinMode(
    PWMB,
    OUTPUT
  );

  pinMode(
    BIN1,
    OUTPUT
  );

  pinMode(
    BIN2,
    OUTPUT
  );


  // Standby

  pinMode(
    STBY,
    OUTPUT
  );


  // Safe startup

  digitalWrite(
    STBY,
    LOW
  );


  stopMotors();


  delay(500);


  digitalWrite(
    STBY,
    HIGH
  );


  // ==========================================================
  // WIFI ACCESS POINT
  // ==========================================================

  WiFi.mode(
    WIFI_AP
  );


  WiFi.softAP(
    AP_SSID,
    AP_PASSWORD
  );


  IPAddress IP =
    WiFi.softAPIP();


  // ==========================================================
  // SERIAL
  // ==========================================================

  Serial.println();
  Serial.println(
    "======================================"
  );
  Serial.println(
    "       ESP32 ROBOT CAR"
  );
  Serial.println(
    " TOUCH JOYSTICK + SPEED ACCELERATOR"
  );
  Serial.println(
    "======================================"
  );


  Serial.print(
    "WiFi: "
  );

  Serial.println(
    AP_SSID
  );


  Serial.print(
    "Password: "
  );

  Serial.println(
    AP_PASSWORD
  );


  Serial.print(
    "IP Address: "
  );

  Serial.println(
    IP
  );


  Serial.println(
    "Open http://192.168.4.1"
  );


  // ==========================================================
  // WEB ROUTES
  // ==========================================================

  server.on(
    "/",
    HTTP_GET,
    handleRoot
  );


  server.on(
    "/cmd",
    HTTP_GET,
    handleCommand
  );


  server.on(
    "/speed",
    HTTP_GET,
    handleSpeed
  );


  server.begin();


  Serial.println(
    "Web server started."
  );
}


// ============================================================
// LOOP
// ============================================================

void loop()
{
  server.handleClient();
}
