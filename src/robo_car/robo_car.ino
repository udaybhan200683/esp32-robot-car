#include <WiFi.h>
#include <WebServer.h>
#include <math.h>

// ============================================================
// ESP32 ROBOT CAR
// PROPORTIONAL TOUCH JOYSTICK
// + SMALL-ANGLE STEERING
// + MOTOR CALIBRATION TRIM
// + VERTICAL SPEED CONTROL
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

// Actual PWM used by the motors. UI speed is 0-100%.
// 69% of 255 ~= 176 is the experimentally stable maximum.
const int SAFE_MAX_PWM = 176;
int motorSpeed = 88; // 35% UI speed mapped to safe PWM

const int MIN_SPEED = 40;
const int MAX_SPEED = SAFE_MAX_PWM;

// Default UI speed = 35%.
// 35% maps to about 88 PWM.
const int DEFAULT_UI_SPEED = 35;


// ============================================================
// MOTOR CALIBRATION
// ============================================================
//
// IMPORTANT:
//
// Start with:
//
// LEFT  = 1.00
// RIGHT = 1.00
//
// If the car naturally moves LEFT during FORWARD,
// reduce LEFT_MOTOR_TRIM.
//
// Example:
//
// LEFT  = 0.94
// RIGHT = 1.00
//
// If the car naturally moves RIGHT,
// reduce RIGHT_MOTOR_TRIM.
//
// Example:
//
// LEFT  = 1.00
// RIGHT = 0.94
//
// Change in small steps:
// 1.00 -> 0.98 -> 0.96 -> 0.94
//
// ============================================================

float LEFT_MOTOR_TRIM  = 1.00;
float RIGHT_MOTOR_TRIM = 1.00;


// ============================================================
// JOYSTICK
// ============================================================

float joystickX = 0.0;
float joystickY = 0.0;

bool joystickActive = false;

String currentCommand = "STOP";


// ============================================================
// STOP MOTORS
// ============================================================

void stopMotors()
{
  analogWrite(PWMA, 0);
  analogWrite(PWMB, 0);

  digitalWrite(AIN1, LOW);
  digitalWrite(AIN2, LOW);

  digitalWrite(BIN1, LOW);
  digitalWrite(BIN2, LOW);

  joystickX = 0.0;
  joystickY = 0.0;

  joystickActive = false;

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

  int leftPWM =
    (int)(motorSpeed * LEFT_MOTOR_TRIM);

  int rightPWM =
    (int)(motorSpeed * RIGHT_MOTOR_TRIM);

  leftPWM =
    constrain(leftPWM, 0, 255);

  rightPWM =
    constrain(rightPWM, 0, 255);

  analogWrite(PWMA, leftPWM);
  analogWrite(PWMB, rightPWM);

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

  int leftPWM =
    (int)(motorSpeed * LEFT_MOTOR_TRIM);

  int rightPWM =
    (int)(motorSpeed * RIGHT_MOTOR_TRIM);

  leftPWM =
    constrain(leftPWM, 0, 255);

  rightPWM =
    constrain(rightPWM, 0, 255);

  analogWrite(PWMA, leftPWM);
  analogWrite(PWMB, rightPWM);

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

  int leftPWM =
    (int)(motorSpeed * LEFT_MOTOR_TRIM);

  int rightPWM =
    (int)(motorSpeed * RIGHT_MOTOR_TRIM);

  leftPWM =
    constrain(leftPWM, 0, 255);

  rightPWM =
    constrain(rightPWM, 0, 255);

  analogWrite(PWMA, leftPWM);
  analogWrite(PWMB, rightPWM);

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

  int leftPWM =
    (int)(motorSpeed * LEFT_MOTOR_TRIM);

  int rightPWM =
    (int)(motorSpeed * RIGHT_MOTOR_TRIM);

  leftPWM =
    constrain(leftPWM, 0, 255);

  rightPWM =
    constrain(rightPWM, 0, 255);

  analogWrite(PWMA, leftPWM);
  analogWrite(PWMB, rightPWM);

  currentCommand = "RIGHT";
}


// ============================================================
// PROPORTIONAL DIFFERENTIAL DRIVE
// ============================================================
//
// X = steering
// Y = throttle
//
// X:
// -1 = full left
// +1 = full right
//
// Y:
// -1 = reverse
// +1 = forward
//
// ============================================================

void setJoystick(
  float x,
  float y
)
{
  x = constrain(
    x,
    -1.0,
    1.0
  );

  y = constrain(
    y,
    -1.0,
    1.0
  );


  joystickX = x;
  joystickY = y;


  // ==========================================================
  // VERY SMALL CENTER DEAD ZONE
  //
  // Previous version used 0.10.
  //
  // That was too large and caused small steering movements
  // to be ignored.
  //
  // Now only about 3.5% around center is ignored.
  // ==========================================================

  const float DEAD_ZONE = 0.035;


  float magnitude =
    sqrt(
      x * x +
      y * y
    );


  if (
    magnitude <
    DEAD_ZONE
  )
  {
    stopMotors();
    return;
  }


  joystickActive = true;


  // ==========================================================
  // REMOVE DEAD-ZONE WITHOUT LOSING SMALL MOVEMENTS
  // ==========================================================

  float scale =
    (magnitude - DEAD_ZONE) /
    (1.0 - DEAD_ZONE);

  scale =
    constrain(
      scale,
      0.0,
      1.0
    );


  if (
    magnitude > 1.0
  )
  {
    x /= magnitude;
    y /= magnitude;
  }


  x *= scale;
  y *= scale;


  // ==========================================================
  // NON-LINEAR STEERING RESPONSE
  //
  // Small joystick movements remain very fine.
  // Large movements become stronger.
  // ==========================================================

  float steering =
    copysign(
      pow(
        fabs(x),
        1.20
      ),
      x
    );


  float left;
  float right;


  // ==========================================================
  // FORWARD / REVERSE
  // ==========================================================

  if (
    fabs(y) > 0.035
  )
  {
    // --------------------------------------------------------
    // DIFFERENTIAL CURVE
    //
    // Example:
    //
    // Forward:
    // left  = 1.0
    // right = 1.0
    //
    // Forward-right:
    // left  = 1.0
    // right = 0.7
    //
    // Small forward-right:
    // left  = 1.0
    // right = 0.95
    //
    // --------------------------------------------------------

    left =
      y * (1.0 + steering);

    right =
      y * (1.0 - steering);


    // --------------------------------------------------------
    // Normalize
    // --------------------------------------------------------

    float maxValue =
      max(
        fabs(left),
        fabs(right)
      );


    if (
      maxValue > 1.0
    )
    {
      left /= maxValue;
      right /= maxValue;
    }
  }
  else
  {
    // ========================================================
    // PURE LEFT / RIGHT
    //
    // Differential pivot steering.
    // ========================================================

    left =
      steering;

    right =
      -steering;
  }


  // ==========================================================
  // APPLY MOTOR CALIBRATION
  // ==========================================================

  left *= LEFT_MOTOR_TRIM;

  right *= RIGHT_MOTOR_TRIM;


  // ==========================================================
  // PWM
  // ==========================================================

  int leftPWM =
    (int)(
      fabs(left) *
      motorSpeed
    );

  int rightPWM =
    (int)(
      fabs(right) *
      motorSpeed
    );


  leftPWM =
    constrain(
      leftPWM,
      0,
      255
    );

  rightPWM =
    constrain(
      rightPWM,
      0,
      255
    );


  // ==========================================================
  // LEFT MOTOR DIRECTION
  // ==========================================================

  if (
    left > 0.01
  )
  {
    digitalWrite(
      AIN1,
      HIGH
    );

    digitalWrite(
      AIN2,
      LOW
    );
  }
  else if (
    left < -0.01
  )
  {
    digitalWrite(
      AIN1,
      LOW
    );

    digitalWrite(
      AIN2,
      HIGH
    );
  }
  else
  {
    digitalWrite(
      AIN1,
      LOW
    );

    digitalWrite(
      AIN2,
      LOW
    );

    leftPWM = 0;
  }


  // ==========================================================
  // RIGHT MOTOR DIRECTION
  // ==========================================================

  if (
    right > 0.01
  )
  {
    digitalWrite(
      BIN1,
      HIGH
    );

    digitalWrite(
      BIN2,
      LOW
    );
  }
  else if (
    right < -0.01
  )
  {
    digitalWrite(
      BIN1,
      LOW
    );

    digitalWrite(
      BIN2,
      HIGH
    );
  }
  else
  {
    digitalWrite(
      BIN1,
      LOW
    );

    digitalWrite(
      BIN2,
      LOW
    );

    rightPWM = 0;
  }


  // ==========================================================
  // APPLY PWM
  // ==========================================================

  analogWrite(
    PWMA,
    leftPWM
  );

  analogWrite(
    PWMB,
    rightPWM
  );


  // ==========================================================
  // STATUS
  // ==========================================================

  if (
    y > 0.08
  )
  {
    if (
      fabs(x) < 0.06
    )
    {
      currentCommand =
        "FORWARD";
    }
    else if (
      x > 0
    )
    {
      currentCommand =
        "FORWARD RIGHT";
    }
    else
    {
      currentCommand =
        "FORWARD LEFT";
    }
  }
  else if (
    y < -0.08
  )
  {
    if (
      fabs(x) < 0.06
    )
    {
      currentCommand =
        "BACKWARD";
    }
    else if (
      x > 0
    )
    {
      currentCommand =
        "BACKWARD RIGHT";
    }
    else
    {
      currentCommand =
        "BACKWARD LEFT";
    }
  }
  else
  {
    if (
      x > 0
    )
    {
      currentCommand =
        "RIGHT";
    }
    else
    {
      currentCommand =
        "LEFT";
    }
  }
}


// ============================================================
// HTML
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

.main
{
  position: absolute;
  left: 0;
  right: 0;
  top: 65px;
  bottom: 0;
}

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

.center
{
  position: absolute;
  left: 50%;
  top: 50%;
  width: 34px;
  height: 34px;
  transform: translate(-50%, -50%);
  border-radius: 50%;
  background: #444;
  border: 2px solid #777;
  pointer-events: none;
}

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
  transform: translateX(-50%);
}

.jdown
{
  bottom: 10px;
  left: 50%;
  transform: translateX(-50%);
}

.jleft
{
  left: 10px;
  top: 50%;
  transform: translateY(-50%);
}

.jright
{
  right: 10px;
  top: 50%;
  transform: translateY(-50%);
}

#finger
{
  position: absolute;
  left: 50%;
  top: 50%;
  width: 48px;
  height: 48px;
  transform: translate(-50%, -50%);
  border-radius: 50%;
  background: #00aa66;
  border: 3px solid #00ff99;
  display: none;
  pointer-events: none;
}

.speedArea
{
  position: absolute;
  right: 12px;
  top: 50%;
  transform: translateY(-50%);
  width: 85px;
  height: 360px;

  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;

  z-index: 10;
  touch-action: none;
}

.speedTitle
{
  font-size: 13px;
  font-weight: bold;
  color: #00ff99;
  margin-bottom: 7px;
}

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
  padding: 15px;
}

#speedTrack
{
  position: absolute;
  left: 50%;
  top: 15px;
  transform: translateX(-50%);
  width: 14px;
  height: 250px;
  border-radius: 8px;
  background: #444;
  pointer-events: none;
}

#speedFill
{
  position: absolute;
  left: 0;
  bottom: 0;
  width: 100%;
  height: 35%;
  border-radius: 8px;

  background:
    linear-gradient(
      to top,
      #008844,
      #00ff99
    );

  pointer-events: none;
}

#speedPaddle
{
  position: absolute;
  left: 50%;
  bottom: 35%;
  transform: translate(-50%, 50%);
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

#speedValue
{
  margin-top: 8px;
  font-size: 17px;
  font-weight: bold;
  color: white;
}

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

<div class="header">

  <div class="title">
    ROBOT CAR
  </div>

  <div id="status">
    STOP
  </div>

</div>

<div class="main">

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

    <div id="speedPad">

      <div id="speedTrack">

        <div id="speedFill">
        </div>

      </div>

      <div id="speedPaddle">
      </div>

    </div>

    <div id="speedValue">
      35%
    </div>

    <div class="speedMin">
      MIN
    </div>

  </div>

</div>


<script>

// ============================================================
// JOYSTICK
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

let joystickPointer =
  null;

let lastSendTime =
  0;


// Faster updates for smoother steering
const SEND_INTERVAL =
  35;


// ============================================================
// IMPORTANT
//
// Previous value was 25 pixels.
//
// That made small steering movements disappear.
//
// Now only 7 pixels around the exact center are ignored.
// ============================================================

const DEAD_ZONE =
  7;


// ============================================================
// JOYSTICK CENTER
// ============================================================

function getJoystickCenter()
{
  const r =
    joystick.getBoundingClientRect();

  return {
    x:
      r.left +
      r.width / 2,

    y:
      r.top +
      r.height / 2
  };
}


// ============================================================
// SEND JOYSTICK
// ============================================================

function sendJoystick(
  x,
  y,
  force = false
)
{
  const now =
    Date.now();


  if (
    !force &&
    now - lastSendTime <
    SEND_INTERVAL
  )
  {
    return;
  }


  lastSendTime =
    now;


  fetch(
    "/joy?x=" +
    x.toFixed(4) +
    "&y=" +
    y.toFixed(4),
    {
      cache: "no-store"
    }
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
// PROCESS JOYSTICK
// ============================================================

function processJoystick(
  clientX,
  clientY
)
{
  const center =
    getJoystickCenter();


  let dx =
    clientX -
    center.x;

  let dy =
    clientY -
    center.y;


  let distance =
    Math.sqrt(
      dx * dx +
      dy * dy
    );


  const radius =
    joystick.clientWidth / 2 -
    30;


  // ==========================================================
  // VERY SMALL CENTER AREA ONLY
  // ==========================================================

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


    sendJoystick(
      0,
      0
    );

    statusText.innerText =
      "STOP";

    return;
  }


  // ==========================================================
  // LIMIT TO CIRCLE
  // ==========================================================

  let fx =
    dx;

  let fy =
    dy;


  if (
    distance >
    radius
  )
  {
    fx =
      dx /
      distance *
      radius;

    fy =
      dy /
      distance *
      radius;

    dx =
      fx;

    dy =
      fy;

    distance =
      radius;
  }


  // ==========================================================
  // SHOW FINGER
  // ==========================================================

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


  // ==========================================================
  // NORMALIZE
  // ==========================================================

  let x =
    dx / radius;

  let y =
    -dy / radius;


  x =
    Math.max(
      -1,
      Math.min(
        1,
        x
      )
    );


  y =
    Math.max(
      -1,
      Math.min(
        1,
        y
      )
    );


  // ==========================================================
  // STATUS
  // ==========================================================

  const m =
    Math.sqrt(
      x*x +
      y*y
    );


  if (
    m < 0.04
  )
  {
    statusText.innerText =
      "STOP";
  }
  else if (
    y > 0.06
  )
  {
    if (
      Math.abs(x) < 0.04
    )
    {
      statusText.innerText =
        "FORWARD";
    }
    else if (
      x > 0
    )
    {
      statusText.innerText =
        "FORWARD RIGHT";
    }
    else
    {
      statusText.innerText =
        "FORWARD LEFT";
    }
  }
  else if (
    y < -0.06
  )
  {
    if (
      Math.abs(x) < 0.04
    )
    {
      statusText.innerText =
        "BACKWARD";
    }
    else if (
      x > 0
    )
    {
      statusText.innerText =
        "BACKWARD RIGHT";
    }
    else
    {
      statusText.innerText =
        "BACKWARD LEFT";
    }
  }
  else
  {
    if (
      x > 0
    )
    {
      statusText.innerText =
        "RIGHT";
    }
    else
    {
      statusText.innerText =
        "LEFT";
    }
  }


  // ==========================================================
  // SEND CONTINUOUS POSITION
  // ==========================================================

  sendJoystick(
    x,
    y
  );
}


// ============================================================
// POINTER DOWN
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
// POINTER MOVE
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
// POINTER UP
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


    sendJoystick(
      0,
      0,
      true
    );


    statusText.innerText =
      "STOP";
  }
);


// ============================================================
// POINTER CANCEL
// ============================================================

joystick.addEventListener(
  "pointercancel",
  function()
  {
    joystickPointer =
      null;


    finger.style.display =
      "none";


    sendJoystick(
      0,
      0,
      true
    );


    statusText.innerText =
      "STOP";
  }
);


// ============================================================
// SPEED CONTROL
// ============================================================

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


  if (
    y < 0
  )
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


  let percent =
    1 -
    y /
    rect.height;


  percent =
    Math.max(
      0,
      Math.min(
        1,
        percent
      )
    );


  // UI is now a true 0-100% control.
  // ESP32 maps 100% UI to the safe 69% PWM limit.
  const displayPercent =
    Math.round(
      percent * 100
    );


  speedFill.style.height =
    (percent * 100) +
    "%";


  speedPaddle.style.bottom =
    (percent * 100) +
    "%";


  speedValue.innerText =
    displayPercent +
    "%";


  fetch(
    "/speed?v=" +
    displayPercent,
    {
      cache: "no-store"
    }
  )
  .catch(
    () => {}
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
// ROOT
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
// JOYSTICK
// ============================================================

void handleJoystick()
{
  if (
    !server.hasArg("x") ||
    !server.hasArg("y")
  )
  {
    server.send(
      400,
      "text/plain",
      "Invalid joystick"
    );

    return;
  }


  float x =
    server.arg("x").toFloat();

  float y =
    server.arg("y").toFloat();


  setJoystick(
    x,
    y
  );


  server.send(
    200,
    "text/plain",
    currentCommand
  );
}


// ============================================================
// OLD COMMAND INTERFACE
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


  if (
    command == "F"
  )
  {
    moveForward();
  }
  else if (
    command == "B"
  )
  {
    moveBackward();
  }
  else if (
    command == "L"
  )
  {
    turnLeft();
  }
  else if (
    command == "R"
  )
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
// SPEED
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


  int uiSpeed =
    server.arg("v").toInt();

  uiSpeed =
    constrain(
      uiSpeed,
      0,
      100
    );

  // Map UI 0-100% to actual PWM.
  // 100% UI = 176 PWM (~69% of 255).
  motorSpeed =
    (uiSpeed == 0) ? 0 :
    (int)(MIN_SPEED +
      ((float)uiSpeed / 100.0f) *
      (SAFE_MAX_PWM - MIN_SPEED));


  // Immediately apply new speed
  // to current joystick position.

  if (
    joystickActive
  )
  {
    setJoystick(
      joystickX,
      joystickY
    );
  }


  Serial.print(
    "Speed UI: "
  );

  Serial.print(
    uiSpeed
  );

  Serial.print(
    "% -> PWM "
  );

  Serial.println(
    motorSpeed
  );


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


  // LEFT MOTOR

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


  // RIGHT MOTOR

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


  // STANDBY

  pinMode(
    STBY,
    OUTPUT
  );


  // SAFE START

  digitalWrite(
    STBY,
    LOW
  );

  stopMotors();

  delay(
    500
  );

  digitalWrite(
    STBY,
    HIGH
  );


  // ==========================================================
  // WIFI AP
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
    "          ESP32 ROBOT CAR"
  );

  Serial.println(
    " PROPORTIONAL TOUCH JOYSTICK"
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


  Serial.print(
    "Left motor trim: "
  );

  Serial.println(
    LEFT_MOTOR_TRIM
  );


  Serial.print(
    "Right motor trim: "
  );

  Serial.println(
    RIGHT_MOTOR_TRIM
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
    "/joy",
    HTTP_GET,
    handleJoystick
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
