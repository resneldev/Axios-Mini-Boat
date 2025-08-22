// Definition der Motorsteuer-Pins
const int M2_IN1 = 34;  // In1 - PC3 - Motor 1 Richtung A
const int M2_IN2 = 35;  // In2 - PC2 - Motor 1 Richtung B
const int M2_EN = 36;   // PC1 - Enable-Pin für Motor 1

const int M1_IN1 = 31;  // PC6 - Motor 2 Richtung A
const int M1_IN2 = 32;  // PC5 - Motor 2 Richtung B
const int M1_EN = 33;   // PC4 - Enable-Pin für Motor 2

// Definition der Fahrsteuerungs-Pins
const int vorwarts = 4;   // D4 - Vorwärts-Taster
const int ruckwarts = 5;  // D5 - Rückwärts-Taster
const int leftPin = 6;    // D6 - Linker Sensor
const int rightPin = 7;   // D7 - Rechter Sensor

// Definition der Modus-Steuerungs-Pins
const int manuellmodusPin = 8;  /* D8 - Verbunden mit Pin D4 des ESP
                                 Der ESP sendet 1, um den manuellen Modus zu aktivieren */
const int automodusPin = 9;     /* D9 - Verbunden mit Pin 12 des ESP
                                 Der ESP sendet 1, um den Automodus zu aktivieren */

const int auto_startPin = 37;  // D10 - Start-Taster für Automodus
const int auto_stoppPin = 30;  // PC7 - Stop-Taster für Automodus

void setup() {
  // Initialisierung der Eingangspins
  pinMode(vorwarts, INPUT);     // Vorwärts-Taster als Eingang
  pinMode(ruckwarts, INPUT);    // Rückwärts-Taster als Eingang

  pinMode(auto_startPin, INPUT);   // Auto-Start-Taster als Eingang
  pinMode(auto_stoppPin, INPUT);   // Auto-Stop-Taster als Eingang

  pinMode(manuellmodusPin, INPUT);  // Manuellmodus-Pin als Eingang
  pinMode(automodusPin, INPUT);     // Automodus-Pin als Eingang

  // Initialisierung der Motorausgangspins
  pinMode(M2_IN1, OUTPUT);  // Motor 2 Richtung A als Ausgang
  pinMode(M2_IN2, OUTPUT);  // Motor 2 Richtung B als Ausgang
  pinMode(M2_EN, OUTPUT);   // Motor 2 Enable als Ausgang
  pinMode(leftPin, INPUT);  // Linker Sensor als Eingang
  
  pinMode(M1_IN1, OUTPUT);  // Motor 1 Richtung A als Ausgang
  pinMode(M1_IN2, OUTPUT);  // Motor 1 Richtung B als Ausgang
  pinMode(M1_EN, OUTPUT);   // Motor 1 Enable als Ausgang
  pinMode(rightPin, INPUT); // Rechter Sensor als Eingang

  // Setze Motor-Geschwindigkeit (PWM, maximal 255)
  analogWrite(M1_EN, 255);  // Volle Geschwindigkeit für Motor 1
  analogWrite(M2_EN, 255);  // Volle Geschwindigkeit für Motor 2

  digitalWrite(M2_IN1, LOW);
  digitalWrite(M2_IN2, LOW);

  digitalWrite(M1_IN1, LOW);
  digitalWrite(M1_IN2, LOW);
}

void loop() {
  // === MANUELLER MODUS ===
  if (digitalRead(manuellmodusPin) == HIGH) {
    // Vorwärts
    if (digitalRead(vorwarts) == HIGH) {
      digitalWrite(M1_IN1, LOW);
      digitalWrite(M1_IN2, HIGH);
      digitalWrite(M2_IN1, HIGH);
      digitalWrite(M2_IN2, LOW);

      if (digitalRead(leftPin) == HIGH) {
        // Nach rechts lenken (Motor 2 seul)
        digitalWrite(M1_IN1, LOW);
        digitalWrite(M1_IN2, LOW);
      } else if (digitalRead(rightPin) == HIGH) {
        // Nach links lenken (Motor 1 seul)
        digitalWrite(M2_IN1, LOW);
        digitalWrite(M2_IN2, LOW);
      }
    }
    // Rückwärts
    else if (digitalRead(ruckwarts) == HIGH) {
      digitalWrite(M1_IN1, HIGH);
      digitalWrite(M1_IN2, LOW);
      digitalWrite(M2_IN1, LOW);
      digitalWrite(M2_IN2, HIGH);

      if (digitalRead(leftPin) == HIGH) {
        digitalWrite(M2_IN1, LOW);
        digitalWrite(M2_IN2, LOW);
      } else if (digitalRead(rightPin) == HIGH) {
        digitalWrite(M1_IN1, LOW);
        digitalWrite(M1_IN2, LOW);
      }
    }
    // Keine Taste gedrückt – Stop
    else {
      digitalWrite(M1_IN1, LOW);
      digitalWrite(M1_IN2, LOW);
      digitalWrite(M2_IN1, LOW);
      digitalWrite(M2_IN2, LOW);
    }
  }


  // === AUTOMATISCHER MODUS ===
else if (digitalRead(automodusPin) == HIGH) {
  // Solange der Automatikmodus aktiviert ist
  while (digitalRead(automodusPin) == HIGH) {
    
    // Wenn der Not-Stopp-Knopf gedrückt wird → Sofortiger Stopp aller Motoren
    if (digitalRead(auto_stoppPin) == HIGH) {
      digitalWrite(M1_IN1, LOW);
      digitalWrite(M1_IN2, LOW);
      digitalWrite(M2_IN1, LOW);
      digitalWrite(M2_IN2, LOW);
      break; // Beende den Automatikmodus
    }

    delay(50); // Kurze Wartezeit zur Entprellung und Stabilisierung

    // Wenn der Startknopf für den Automatikmodus gedrückt wird
    if (digitalRead(auto_startPin) == HIGH){
      
    }
    if (digitalRead(auto_startPin) == HIGH) {
      // Bewegungsdauer für jede Etappe (in Millisekunden)
      int stepDurations[] = {10000, 5000, 6000, 10000, 5000};
      int step = 0;
      // === Etappe 1: Vorwärts fahren ===
      digitalWrite(M2_IN1, HIGH);
      digitalWrite(M2_IN2, LOW);
      digitalWrite(M1_IN1, LOW);
      digitalWrite(M1_IN2, HIGH);
      delay(stepDurations[0]);
      step += 1;
      if (digitalRead(auto_stoppPin) == HIGH) break;

      // === Etappe 2: Stoppen ===
      digitalWrite(M1_IN1, LOW);
      digitalWrite(M1_IN2, LOW);
      digitalWrite(M2_IN1, LOW);
      digitalWrite(M2_IN2, LOW);
      delay(stepDurations[1]);
      step += 1;
      if (digitalRead(auto_stoppPin) == HIGH) break;

      // === Etappe 3: Rechtsdrehung (nur ein Rad bewegt sich) ===
      digitalWrite(M2_IN1, HIGH);
      digitalWrite(M2_IN2, LOW);
      digitalWrite(M1_IN1, LOW);
      digitalWrite(M1_IN2, LOW);
      delay(stepDurations[2]);
      step += 1;
      if (digitalRead(auto_stoppPin) == HIGH) break;

      // === Etappe 4: Wieder vorwärts fahren ===
      digitalWrite(M2_IN1, HIGH);
      digitalWrite(M2_IN2, LOW);
      digitalWrite(M1_IN1, LOW);
      digitalWrite(M1_IN2, HIGH);
      delay(stepDurations[3]);
      step += 1;
      if (digitalRead(auto_stoppPin) == HIGH) break;

      // === Etappe 5: Endgültiger Stopp ===
      digitalWrite(M1_IN1, LOW);
      digitalWrite(M1_IN2, LOW);
      digitalWrite(M2_IN1, LOW);
      digitalWrite(M2_IN2, LOW);
      delay(stepDurations[4]);
      step += 1;
      if (digitalRead(auto_stoppPin) == HIGH) break;
    }
  }
}

  // Kein aktiver Modus → alles beenden
  else {
    digitalWrite(M1_IN1, LOW);
    digitalWrite(M1_IN2, LOW);
    digitalWrite(M2_IN1, LOW);
    digitalWrite(M2_IN2, LOW);
  }

  delay(50);  // Anti-rebond
}
