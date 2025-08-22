

/*Dieses Programm implementiert die Steuerung eines mobilen Roboters (basierend auf dem ESP8266),
  der sowohl manuell als auch automatisch gesteuert werden kann. 
  Zusätzlich misst der ESP8266 kontinuierlich die elektrische Leitfähigkeit des Wassers über einen Sensor, 
  der an Pin A0 angeschlossen ist. 
  Das System stellt eine Webschnittstelle zur Verfügung, über die Steuerbefehle gesendet werden können.*/


#include <ESP8266WiFi.h>               // Bibliothek für die WiFi-Verbindung des ESP8266
#include <WiFiClient.h>                // TCP/IP-Client für Netzwerkkommunikation
#include <ESP8266WebServer.h>          // HTTP-Server zur Verwaltung von Webanfragen

// WiFi-Verbindungsdaten
const char* ssid = "Pixel_resnel";     // Name des WiFi-Netzwerks
const char* password = "1234567890";   // Passwort des WiFi-Netzwerks

// Definition der Pins für Bewegungssteuerung
const int MOVE_FORWARD_PIN = 16;       // Pin für Vorwärtsbewegung (D0)
const int MOVE_BACKWARD_PIN = 5;       // Pin für Rückwärtsbewegung (D1)
const int TURN_LEFT_PIN = 4;           // Pin für Linkskurve (D2)
const int TURN_RIGHT_PIN  = 0;          // Pin für Rechtskurve (D3)

// Pin für Leitfähigkeitssensor
const int LeitwertPin = A0;            // Analoge Eingang A0

// Pins für Betriebsmodi
const int manuellmodusPin = 2;         // Pin für manuellen Modus (D4)
const int automodusPin = 14;           // Pin für automatischen Modus (D5)

const int auto_startPin = 12;          // Pin zum Starten des Automodus (D6)
const int auto_stoppPin = 13;          // Pin zum Stoppen des Automodus (D7)

// Erstellung des HTTP-Servers auf Port 80
ESP8266WebServer server(80);

// Statusvariablen für Betriebsmodi
bool autoModus = false;                // Variable für Automatikmodus
bool manuellModus = false;             // Variable für manuellen Modus

void setup() { 
  Serial.begin(9600);                  // Initialisierung der seriellen Kommunikation
  delay(1000);                         // Pause für Stabilität

  Serial.println("\n=== ESP8266 Initialisierung ===");  // Startnachricht

  // Konfiguration der Motorausgangspins
  pinMode(MOVE_FORWARD_PIN, OUTPUT);
  pinMode(MOVE_BACKWARD_PIN, OUTPUT);
  pinMode(TURN_LEFT_PIN, OUTPUT);
  pinMode(TURN_RIGHT_PIN, OUTPUT);

  pinMode(LeitwertPin, INPUT);         // Konfiguration des analogen Sensorinputs

  // Konfiguration der Modus-Pins (Ausgang oder Eingang bei echten Tastern)
  pinMode(manuellmodusPin, OUTPUT);    
  pinMode(automodusPin, OUTPUT);
  pinMode(auto_startPin, OUTPUT);
  pinMode(auto_stoppPin, OUTPUT);

  // Initialisierung der Motoren auf LOW
  digitalWrite(MOVE_FORWARD_PIN, LOW);
  digitalWrite(MOVE_BACKWARD_PIN, LOW);
  digitalWrite(TURN_LEFT_PIN, LOW);
  digitalWrite(TURN_RIGHT_PIN, LOW);

  digitalWrite(auto_startPin, LOW);
  digitalWrite(auto_stoppPin, LOW);

  // Manueller Modus standardmäßig aktiviert
  digitalWrite(manuellmodusPin, HIGH);
  digitalWrite(automodusPin, LOW);

  // WiFi-Verbindung
  Serial.print("Verbinde mit ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);          // Start der WiFi-Verbindung
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);                        // Warte auf Verbindung
    Serial.print(".");
  }

  // WiFi verbunden
  Serial.println("\nWiFi verbunden");
  Serial.print("IP-Adresse: ");
  Serial.println(WiFi.localIP());      // Anzeige der lokalen IP-Adresse

  // Hauptroute: Funktionsprüfung + A0-Lesung
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/plain", "ESP8266 Dual Mode Server\n"
                "IP-Adresse: " + WiFi.localIP().toString() + "\n"
                "Pin A0: " + String(analogRead(LeitwertPin)));
  });

  // Steuerungsroute (API)
  server.on("/control", HTTP_GET, handleControl);  // Route für Bewegungs- und Modusbefehle

  server.begin();                        // Start des Webservers
}

void loop() {
  server.handleClient();                 // Bearbeitung eingehender Anfragen
}

// Funktion zur Bearbeitung der per GET-Anfrage erhaltenen Aktionen
void handleControl() {
  if (!server.hasArg("action")) {       // Prüft ob 'action'-Parameter vorhanden ist
    server.send(400, "text/plain", "Parameter 'action' fehlt");
    return;
  }

  String action = server.arg("action"); // Holt den Aktionswert
  String response = "";                 // Antwortstring

  // Bewegungsbefehle:
  if (action == "GO_FORWARD_ON") {
    digitalWrite(MOVE_FORWARD_PIN, HIGH);   // Aktiviert Vorwärtsfahrt
    
  } 
  else if (action == "GO_FORWARD_OFF") {
    digitalWrite(MOVE_FORWARD_PIN, LOW);    // Deaktiviert Vorwärtsfahrt
    
  }
  else if (action == "GO_BACKWARD_ON") {
    digitalWrite(MOVE_BACKWARD_PIN, HIGH);  // Aktiviert Rückwärtsfahrt
    
  } 
  else if (action == "GO_BACKWARD_OFF") {
    digitalWrite(MOVE_BACKWARD_PIN, LOW);   // Deaktiviert Rückwärtsfahrt
    
  }
  else if (action == "GO_LEFT_ON") {
    digitalWrite(TURN_LEFT_PIN, HIGH);      // Aktiviert Linkskurve
    
  } 
  else if (action == "GO_LEFT_OFF") {
    digitalWrite(TURN_LEFT_PIN, LOW);       // Deaktiviert Linkskurve
    
  }
  else if (action == "GO_RIGHT_ON") {
    digitalWrite(TURN_RIGHT_PIN, HIGH);     // Aktiviert Rechtskurve
    
  } 
  else if (action == "GO_RIGHT_OFF") {
    digitalWrite(TURN_RIGHT_PIN, LOW);      // Deaktiviert Rechtskurve
    
  }

  // Moduswechsel
  else if (action == "auto_modus") {
    autoModus = true;                       // Aktiviert Automatikmodus
    manuellModus = false;
    digitalWrite(automodusPin, HIGH);       // Aktiviert zugehörige LED/Ausgang
    digitalWrite(manuellmodusPin, LOW);
    //response = "Auto-Modus aktiviert";      // Antwort an Client
  }
  else if (action == "manuell_modus") {
    manuellModus = true;                    // Aktiviert manuellen Modus
    autoModus = false;
    digitalWrite(manuellmodusPin, HIGH);
    digitalWrite(automodusPin, LOW);
    
  }

  // Leitfähigkeitssensor auslesen
  else if (action == "read_Leitwert") {
    float conductance = measureConductance(); // Ruft Messfunktion auf
    response = String(conductance, 4);        // Konvertiert zu Text mit 4 Dezimalen
  }
  
  else if (action == "auto_start"){

    digitalWrite(auto_startPin, HIGH);         //Start automatische Funktionnierung 
    
    digitalWrite(auto_stoppPin, LOW);
  }
  else if (action == "auto_stopp"){

    digitalWrite(auto_startPin, LOW);           //Sptoppt automatische Funktionnierung
    digitalWrite(auto_stoppPin, HIGH);
  }

  // Unbekannte Aktion
  else {
    server.send(400, "text/plain", "Befehl nicht erkannt");  // Fehler: Unbekannter Befehl
    return;
  }
  
  server.send(200, "text/plain", response);  // Sendet HTTP-Antwort mit Ergebnis
}

// Funktion zur Berechnung der Leitfähigkeit basierend auf gemessener Spannung
float measureConductance() {
  int sensorWert = analogRead(LeitwertPin);  // Liest analogen Wert
  float sensorSpannung = sensorWert * (3.3 / 1023.0); // Umwandlung in Spannung (3.3V max)

  if (sensorSpannung <= 0.01) return 0;      // Schutz gegen Division durch 0

  long messWiderstand = 100000;              // Wert des Messwiderstands (100 kOhms)
  float conductance = (((3.3 - sensorSpannung) / messWiderstand) * (1 / sensorSpannung)) * 1000000; 
  // Berechnung der Leitfähigkeit in µS (Mikrosiemens)
  
  return conductance;                        // Gibt geschätzten Leitfähigkeitswert zurück
}