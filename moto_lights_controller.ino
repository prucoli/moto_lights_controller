/*

   Moto Lights Controller

*/

//
// Pin config
//

// Buttons
const int BUTTON_R = 2;
const int BUTTON_L = 6;

// Relays
const int RELAY_TL_R = 9;
const int RELAY_TL_L = 10;
const int RELAY_BLINKER = 11;

//
// Event sensing
//

// Event sensing states constants
const int EVENT_SENSING_IDLE = 0;
const int EVENT_SENSING_TRIGGED = 1;
const int EVENT_SENSING_PROCESSING = 2;
const int EVENT_SENSING_COMPUTING = 3;
const int EVENT_SENSING_ACTING = 4;
const int EVENT_SENSING_DONE = 5;

const unsigned long EVENT_SENSING_INTERVAL_MILLIS = 300;

// Event sensing variables
int esState = EVENT_SENSING_IDLE;
unsigned long esLastMillis = 0;

int btRLastState = HIGH;
int btRHCount = 0;
int btRLCount = 0;
bool btRClicked = false;
bool btRDoubleClicked = false;

int btLLastState = HIGH;
int btLHCount = 0;
int btLLCount = 0;
bool btLClicked = false;
bool btLDoubleClicked = false;

bool btBClicked = false;
bool btBDoubleClicked = false;

int rlRNewState = LOW;
int rlLNewState = LOW;
int rlBNewState = LOW;


//
//
//
void setup() {
  Serial.begin(57600);

  pinMode(BUTTON_R, INPUT);
  pinMode(BUTTON_L, INPUT);

  pinMode(RELAY_TL_R, OUTPUT);
  pinMode(RELAY_TL_L, OUTPUT);
  pinMode(RELAY_BLINKER, OUTPUT);

  pinMode(LED_BUILTIN, OUTPUT);
  delay(1000);
}

//
//
//
void loop() {
  switch (esState) {
    case EVENT_SENSING_IDLE:
      digitalWrite(LED_BUILTIN, HIGH);

      if (lookForEvents()) {
        esLastMillis = millis();
        esState = EVENT_SENSING_TRIGGED;
        digitalWrite(LED_BUILTIN, LOW);
        Serial.print("event trigged\n");
      }
      break;

    case EVENT_SENSING_TRIGGED:
      if (captureEvents()) {
        esState = EVENT_SENSING_PROCESSING;
        Serial.print("btRHCount = ");
        Serial.print(btRHCount);
        Serial.print(", btRLCount = ");
        Serial.print(btRLCount);
        Serial.print(", btLHCount = ");
        Serial.print(btLHCount);
        Serial.print(", btLLCount = ");
        Serial.print(btLLCount);
        Serial.print("\n");
      }
      break;

    case EVENT_SENSING_PROCESSING:
      processEvents();
      esState = EVENT_SENSING_COMPUTING;
      Serial.print("btRClicked = ");
      Serial.print(btRClicked);
      Serial.print(", btRDoubleClicked = ");
      Serial.print(btRDoubleClicked);
      Serial.print(", btLClicked = ");
      Serial.print(btLClicked);
      Serial.print(", btLDoubleClicked = ");
      Serial.print(btLDoubleClicked);
      Serial.print(", btBClicked = ");
      Serial.print(btBClicked);
      Serial.print(", btBDoubleClicked = ");
      Serial.print(btBDoubleClicked);
      Serial.print("\n");
      break;

    case EVENT_SENSING_COMPUTING:
      computeNewOutputState();
      esState = EVENT_SENSING_ACTING;
      Serial.print("rlRNewState = ");
      Serial.print(rlRNewState);
      Serial.print(", rlLNewState = ");
      Serial.print(rlLNewState);
      Serial.print(", rlBNewState = ");
      Serial.print(rlBNewState);
      Serial.print("\n");
      break;

    case EVENT_SENSING_ACTING:
      performActions();
      esState = EVENT_SENSING_DONE;
      break;

    case EVENT_SENSING_DONE:
      clear();
      esState = EVENT_SENSING_IDLE;
      break;

    default:
      clear();
      esState = EVENT_SENSING_IDLE;
      break;
  }
}

//
//
//
bool lookForEvents() {
  return watchButtons();
}

//
//
//
bool captureEvents() {
  unsigned long esCurrMillis = millis();

  if ((esCurrMillis - esLastMillis) > EVENT_SENSING_INTERVAL_MILLIS) {
    return true;
  }

  watchButtons();

  return false;
}

//
//
//
void processEvents() {
  if (btRLCount >= 2 && btRHCount >= 2) {
    btRDoubleClicked = true;
  } else if (btRLCount >= 1 && btRHCount >= 1) {
    btRClicked = true;
  }

  if (btLLCount >= 2 && btLHCount >= 2) {
    btLDoubleClicked = true;
  } else if (btLLCount >= 1 && btLHCount >= 1) {
    btLClicked = true;
  }

  if (btRDoubleClicked && btLDoubleClicked) {
    btRDoubleClicked = false;
    btLDoubleClicked = false;
    btBDoubleClicked = true;
  }

  if (btRClicked && btLClicked) {
    btRClicked = false;
    btLClicked = false;
    btBClicked = true;
  }
}

//
//
//
void computeNewOutputState() {
  int rlRCurrState = digitalRead(RELAY_TL_R);
  int rlLCurrState = digitalRead(RELAY_TL_L);
  int rlBCurrState = digitalRead(RELAY_BLINKER);

  int warningState = (rlRCurrState == HIGH && rlLCurrState == HIGH);

  if (btRClicked) {
    rlRNewState = !rlRCurrState || warningState;
    rlBNewState = rlBCurrState;
  }

  if (btLClicked) {
    rlLNewState = !rlLCurrState || warningState;
    rlBNewState = rlBCurrState;
  }

  if (btBClicked) {
    rlRNewState = !warningState;
    rlLNewState = !warningState;
    rlBNewState = rlBCurrState;
  }

  if (btRDoubleClicked || btLDoubleClicked || btBDoubleClicked) {
    rlRNewState = rlRCurrState;
    rlLNewState = rlLCurrState;
    rlBNewState = !rlBCurrState;
  }
}

//
//
//
void performActions() {
  digitalWrite(RELAY_TL_R, rlRNewState);
  digitalWrite(RELAY_TL_L, rlLNewState);
  digitalWrite(RELAY_BLINKER, rlBNewState);
}

//
//
//
void clear() {
  esLastMillis = 0;

  btRHCount = 0;
  btRLCount = 0;
  btRClicked = false;
  btRDoubleClicked = false;

  btLHCount = 0;
  btLLCount = 0;
  btLClicked = false;
  btLDoubleClicked = false;

  btBClicked = false;
  btBDoubleClicked = false;

  rlRNewState = LOW;
  rlLNewState = LOW;
  rlBNewState = LOW;
}

//
//
//
bool watchButtons() {
  int btRCurrentState = digitalRead(BUTTON_R);
  int btLCurrentState = digitalRead(BUTTON_L);

  if (btRCurrentState != btRLastState) {
    btRLastState = btRCurrentState;

    if (btRCurrentState == HIGH) {
      btRHCount++;

    } else if (btRCurrentState == LOW) {
      btRLCount++;
    }
  }

  if (btLCurrentState != btLLastState) {
    btLLastState = btLCurrentState;

    if (btLCurrentState == HIGH) {
      btLHCount++;

    } else if (btLCurrentState == LOW) {
      btLLCount++;
    }
  }

  // TODO: do it in non-blocking way
  delay(3);

  return (btRHCount > 0) || (btRLCount > 0) || (btLHCount > 0) || (btLLCount > 0);
}
