#define TRIGGER 100
#define TRIGGER1 10

#define EVENTTHRESH 4

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  for (int i = 0; i < 4; i++) {
    pinMode(A0 + i, INPUT);
    digitalWrite(A0 + i, 1);
  }
}
uint16_t sample[4];
char pattern[4];
char lastpattern[4];
char countpos = 0;
char countneg = 0;
int autocorrect = 0;
const char sequence[4][4] = {
  {0, 1, 0, 1},
  {0, 1, 1, 0},
  {1, 0, 1, 0},
  {1, 0, 0, 1}
};
char lastseq;
char deltaseq;
void loop() {
  // put your main code here, to run repeatedly:
  bool event = false;
  for (int i = 0; i < 4; i++) {
    sample[i] = analogRead(A0 + i);
  }

  for (int i = 0; i < 4; i++) {
    sample[i] = 1023 - sample[i];
    if (!event && sample[i] > TRIGGER) {
      event = true;
    }
  }
  if (!event) {
    return;
  }
  for (int i = 0; i < 4; i++) {
    if (sample[i] < TRIGGER1) {
      pattern[i] = 1;
    }
    else {
      pattern[i] = 0;
    }
  }

  event = false;
  int patterncounter = 0;
  for (int i = 0; i < 4; i++) {
    if (pattern[i] == 1) {
      patterncounter ++;
    }
    if (pattern[i] != lastpattern[i]) {
      event = true;
    }
  }
  if (patterncounter != 2) {
    event = false;
  }
  if (!event) {
    return;
  }
  char seq = -1;
  for (int i = 0; i < 4; i++) {
    bool match = true;
    for (int j = 0; j < 4; j++) {
      if (pattern[j] != sequence[i][j]) {
        match = false;
        break;
      }
    }
    if (match) {
      seq = i;
      break;
    }
  }
  if (seq == -1) {
    event = false;
  }
  else {
    if (seq == 0 && lastseq == 3) {
      deltaseq = 1;
    }
    else if (seq == 3 && lastseq == 0) {
      deltaseq = -1;
    }
    else {
      deltaseq = seq - lastseq;
      if (abs(deltaseq) != 1) {
        if (autocorrect != 0) {
          deltaseq = autocorrect;
        }
        else {
          event = false;
        }
      }
      else if (autocorrect != 0 && autocorrect != deltaseq) {
        if (deltaseq == 1) {
          countpos ++;
          if (countpos >= EVENTTHRESH) {
            countpos = 0;
            countneg = 0;
            autocorrect = 1;
          }
        }
        else {
          countneg ++;
          if (countneg >= EVENTTHRESH) {
            countpos = 0;
            countneg = 0;
            autocorrect = -1;
          }
        }
        event = false;
      }
    }
  }
  if (abs(deltaseq) == 1 && (abs(countpos) < EVENTTHRESH || abs(countneg) < EVENTTHRESH) && autocorrect == 0) {
    if (deltaseq == 1) {
      countpos ++;
      if (countpos >= EVENTTHRESH) {
        countpos = 0;
        countneg = 0;
        autocorrect = 1;
      }
    }
    else {
      countneg ++;
      if (countneg >= EVENTTHRESH) {
        countpos = 0;
        countneg = 0;
        autocorrect = -1;
      }
    }
    event = false;
  }
  if (!event) {
    return;
  }
  memcpy(lastpattern, pattern, 4 * sizeof(lastpattern));
  lastseq = seq;
  // one and zero represent rotation direction
  Serial.print((deltaseq > 0) ? 1 : 0);
}
