# STM32 Access Controller – Meilensteinplan

## 1. Ziel des Plans

Der Access Controller soll schrittweise praktisch umgesetzt werden.  
Der Schwerpunkt liegt auf dem Aufbau von Kenntnissen in:

- C
- STM32-Mikrocontrollerentwicklung
- GPIO
- Timer
- Interrupts
- UART
- SPI
- I²C
- PWM
- Debugging
- Zustandsautomaten
- Fehlerbehandlung
- Tests
- technischer Dokumentation
- Datenblattarbeit
- grundlegender Treiberentwicklung

Requirements und Dokumentation werden parallel zur Implementierung weitergeführt und nicht vollständig vorab abgeschlossen.

---

## 2. Arbeitsprinzip

Für jeden Meilenstein gilt möglichst:

1. notwendiges Konzept verstehen
2. relevanten Hardware- und Datenfluss nachvollziehen
3. kleinsten sinnvollen Versuch aufbauen
4. eigene Erwartung formulieren
5. selbst implementieren
6. auf echter Hardware testen
7. Fehler systematisch untersuchen
8. Ergebnis erklären können
9. relevante Requirements, Tests und Dokumentation aktualisieren

Ein Meilenstein gilt nicht nur dann als abgeschlossen, wenn die Funktion technisch arbeitet, sondern wenn die wesentlichen Zusammenhänge verstanden und erklärt werden können.

### 2.1 Einsatz von CubeIDE, CubeMX und HAL

Für das Projekt wird folgende Strategie verwendet:

- **STM32CubeIDE wird als Entwicklungs-, Build-, Flash- und Debugging-Umgebung verwendet.**
- **CubeMX wird zur Clock-, Pin- und Peripheriekonfiguration verwendet.**
- **STM32 HAL darf für die produktive Ansteuerung der STM32-Peripherie verwendet werden.**
- HAL und CubeMX dürfen jedoch nicht als Black Box verwendet werden.
- Bei zentralen Peripherien werden die zugrunde liegenden STM32-Register, Clock-Zusammenhänge und Datenflüsse zusätzlich untersucht.
- Ausgewählte Funktionen werden einmal direkt über CMSIS/Registerzugriffe umgesetzt, um die Wirkung der HAL nachvollziehen zu können.
- Es wird vorerst keine zusätzliche LL-Abstraktion eingeführt.
- Die eigentliche Applikationslogik und die projektspezifischen Gerätetreiber werden selbst entwickelt.

### 2.2 Was selbst implementiert wird

Insbesondere selbst entwickelt werden:

- Zustandsautomat und Ereignislogik
- Tür- und Riegellogik
- Blockierungs- und Timeoutlogik
- Fehlerbehandlung
- Tasterentprellung
- Servo-Modul
- MFRC522-Gerätetreiber
- grundlegender SSD1306-Gerätetreiber
- Whitelist- und Persistenzlogik
- spätere UART-Puffer- beziehungsweise Kommandoverarbeitung, falls benötigt

Für den MFRC522 wird **keine fertige MFRC522-Komplettbibliothek für die Kernfunktionalität verwendet**.  
Die Registerzugriffe und die für Kartenerkennung und UID-Auslesen benötigte Treiberlogik werden anhand des Datenblatts selbst hergeleitet und implementiert.

---

## 3. Meilensteinplan

### Meilenstein 1 – Entwicklungsumgebung und Board kennenlernen

- NUCLEO-F446RE anschließen
- STM32CubeIDE einrichten
- Projektstruktur kennenlernen
- Rolle von CubeIDE, CubeMX, HAL und CMSIS unterscheiden
- CubeMX-Konfiguration grob verstehen
- generierten Initialisierungscode grob lokalisieren
- Verbindung zum integrierten ST-LINK prüfen

**Ziel:**  
Die grundlegende STM32-Entwicklungsumgebung verstehen und einordnen können, welche Aufgaben die Werkzeuge übernehmen und welcher Code selbst entwickelt wird.

---

### Meilenstein 2 – Erstes Projekt bauen und flashen

- minimales STM32-Projekt erstellen
- Projekt kompilieren
- Firmware auf den STM32 übertragen
- Programm starten
- Reset-Verhalten beobachten
- grob nachvollziehen, wie Startup-Code, `main()` und Initialisierung zusammenhängen

**Ziel:**  
Den Ablauf von Quellcode über Build und Flash bis zur Programmausführung auf dem Mikrocontroller verstehen.

---

### Meilenstein 3 – GPIO-Ausgang und LED

- GPIO als Ausgang mit CubeMX/HAL konfigurieren
- LED ein- und ausschalten
- GPIO-Zustände im Code verändern
- Pin-Konfiguration nachvollziehen
- zugehörige STM32-Register wie Clock-Freigabe, `MODER` und `BSRR` kennenlernen
- als gezielte Bare-Metal-Übung eine LED einmal ohne `HAL_GPIO_WritePin()` direkt über CMSIS/Registerzugriffe schalten
- HAL- und Registervariante miteinander vergleichen

**Ziel:**  
Grundlegende digitale Ausgänge verstehen und nachvollziehen können, was unter einem HAL-Aufruf auf Registerebene geschieht.

---

### Meilenstein 4 – Debugger verwenden

- Breakpoints setzen
- Programm schrittweise ausführen
- Variablen beobachten
- Call Stack ansehen
- Registeransicht verwenden
- GPIO-Register während der Ausführung beobachten
- später Watchpoints verwenden

**Ziel:**  
Den Debugger als zentrales Werkzeug für Inbetriebnahme und Fehlersuche sicher einsetzen und Softwarezustand mit Hardware-Registern verbinden können.

---

### Meilenstein 5 – GPIO-Eingang und Taster

- Taster als digitalen Eingang anschließen
- GPIO-Eingang konfigurieren
- HIGH- und LOW-Zustände erkennen
- Zustandsänderungen im Programm auswerten
- zugehöriges Input-Register im Debugger beobachten

**Ziel:**  
Verstehen, wie externe digitale Signale vom Mikrocontroller eingelesen und auf Register- sowie Softwareebene verarbeitet werden.

---

### Meilenstein 6 – Pull-up, Pull-down und Entprellung

- Floating Inputs verstehen
- Pull-up und Pull-down vergleichen
- interne und externe Pull-Widerstände einordnen
- mechanisches Tasterprellen beobachten
- geeignete Entprellung selbst implementieren
- Entprellung ohne blockierende Delays umsetzen

**Ziel:**  
Digitale Eingänge zuverlässig auswerten und typische reale Hardwareeffekte berücksichtigen.

---

### Meilenstein 7 – Zeitbasis und nicht blockierende Timeouts

- Systemzeit verwenden
- Zeitdifferenzen berechnen
- einfache Timeouts selbst implementieren
- blockierende Delays mit nicht blockierenden Abläufen vergleichen
- Timer-Overflow berücksichtigen
- verstehen, aus welcher Hardware-Zeitbasis die verwendete Systemzeit entsteht
- relevante Timer-/SysTick-Register im Debugger ansehen

**Ziel:**  
Zeitabhängiges Verhalten umsetzen, ohne die Main Loop unnötig zu blockieren, und die Verbindung zwischen Hardware-Zähler und Software-Zeitbasis verstehen.

---

### Meilenstein 8 – Interrupts

- einfachen Interrupt konfigurieren
- Polling und Interrupts vergleichen
- Interruptquelle, Interruptflag und NVIC unterscheiden
- Interrupt Service Routine verstehen
- ISR möglichst kurz halten
- `volatile` im Zusammenhang mit Interrupts verstehen
- Interruptablauf im Debugger nachvollziehen
- mindestens einmal relevante Interrupt-/NVIC-Register ansehen

**Ziel:**  
Interruptbasierte Ereignisverarbeitung verstehen und erklären können, wie ein Hardwareereignis bis zur ISR gelangt.

---

### Meilenstein 9 – UART und Diagnose

- UART mit CubeMX/HAL konfigurieren
- Baudrate und Frame-Aufbau verstehen
- Daten vom STM32 an den PC senden
- Statusmeldungen ausgeben
- Fehler- und Diagnoseausgaben strukturieren
- UART-Register und Statusflags grob kennenlernen
- später interruptbasierten Empfang einführen
- bei Bedarf eigenen Ringpuffer implementieren

**Ziel:**  
UART als Kommunikations- und Diagnosewerkzeug verwenden und sowohl die HAL-Nutzung als auch die wesentlichen Hardwaremechanismen dahinter verstehen.

---

### Meilenstein 10 – Servo, Timer und PWM

- Hardware-Timer für PWM konfigurieren
- Timer-Clock bestimmen
- Prescaler selbst berechnen
- Counter-Frequenz bestimmen
- `PSC`, `ARR`, `CNT` und `CCR` verstehen
- Frequenz, Periode und Pulsbreite selbst herleiten
- MG90S ansteuern
- Verriegelungs- und Entriegelungsposition kalibrieren
- relevante Timerregister im Debugger mit den eigenen Berechnungen vergleichen
- mindestens eine Änderung vorhersagen, Register-/Konfigurationswert ändern und Wirkung messen

**Ziel:**  
PWM nicht nur über CubeMX konfigurieren, sondern die zugrunde liegende Timerhardware und die mathematischen Zusammenhänge sicher erklären können.

---

### Meilenstein 11 – Tür- und Bolzensensoren

- Türpositionssensor integrieren
- Endsensor für Position `LOCKED` integrieren
- Endsensor für Position `UNLOCKED` integrieren
- gültige und widersprüchliche Sensorkombinationen erkennen
- Sensorzustände für Initialisierung und Fehlererkennung verwenden
- Sensorauswertung als eigenes Modul strukturieren

**Ziel:**  
Das reale mechanische System mit der Firmware rückkoppeln und den tatsächlichen Zustand von Tür und Riegel zuverlässig erfassen.

---

### Meilenstein 12 – SPI-Grundlagen auf dem STM32

- SPI-Datenfluss auf dem STM32 nachvollziehen
- Master-/Slave-Rolle verstehen
- Clock Polarity und Clock Phase verstehen
- Datenbreite, Bitreihenfolge und Baudrate/Prescaler einordnen
- Chip Select bewusst selbst steuern
- SPI mit CubeMX/HAL konfigurieren
- relevante STM32-SPI-Register wie Control-, Status- und Datenregister kennenlernen
- einen einfachen SPI-Transfer einmal gezielt auf Registerebene nachvollziehen beziehungsweise als kleine Bare-Metal-Übung umsetzen
- anschließend HAL für die produktive SPI-Kommunikation des Projekts verwenden

**Ziel:**  
Verstehen, was der STM32 für einen SPI-Transfer hardwareseitig ausführt und warum die gewählte SPI-Konfiguration zum angeschlossenen Gerät passen muss.

---

### Meilenstein 13 – Eigenen MFRC522-Treiber entwickeln

- MFRC522-Datenblatt als primäre Quelle verwenden
- Registermodell des MFRC522 kennenlernen
- Aufbau der SPI-Kommandobytes für Registeradresse, Lesen und Schreiben selbst herleiten
- eigene Funktion zum Schreiben eines MFRC522-Registers implementieren
- eigene Funktion zum Lesen eines MFRC522-Registers implementieren
- Registerzugriffe zunächst mit einem einfachen Identifikations-/Versionsregister testen
- Reader initialisieren
- benötigte Status-, FIFO- und Command-Register nachvollziehen
- RFID-Karte erkennen
- für den Projektumfang erforderlichen Kommunikationsablauf zur Karte verstehen
- UID auslesen
- Kartenentfernung erkennen
- Kommunikationsfehler und Timeouts behandeln
- Treiberschnittstelle in eigener `.c`-/`.h`-Datei definieren
- keine fertige MFRC522-Komplettbibliothek für diese Kernfunktionalität übernehmen

**Ziel:**  
Anhand eines realen Datenblatts einen eigenen einfachen Gerätetreiber entwickeln: von SPI-Registerzugriffen bis zur für das Projekt benötigten RFID-Funktionalität.

---

### Meilenstein 14 – I²C und eigener SSD1306-Grundtreiber

- I²C konfigurieren
- START, Adresse, ACK/NACK, Daten und STOP grundlegend verstehen
- Adressierung des SSD1306 nachvollziehen
- eigene grundlegende Funktionen zum Senden von Commands und Daten implementieren
- SSD1306 anhand des Datenblatts beziehungsweise der technischen Dokumentation initialisieren
- Display-RAM-Grundidee verstehen
- einfache Ausgabe darstellen
- für Textdarstellung dürfen begrenzte Hilfsdaten wie ein Font verwendet werden; die grundlegende Displaykommunikation soll verstanden bleiben
- Systemzustände anzeigen
- bereits definierte Displaymeldungen integrieren

**Ziel:**  
Eine zweite serielle Schnittstelle praktisch einsetzen und einen einfachen Gerätetreiber auf einer HAL-basierten I²C-Transportebene selbst aufbauen.

---

### Meilenstein 15 – Firmwaremodule und Schichten strukturieren

- Hardwarezugriff und Applikationslogik trennen
- zwischen STM32-HAL, Gerätetreibern und Applikationslogik unterscheiden
- `.c`- und `.h`-Dateien sinnvoll einsetzen
- öffentliche und modulinterne Funktionen unterscheiden
- `static`, `const`, `enum`, `struct` und Fehlercodes gezielt verwenden
- globale Daten möglichst reduzieren
- klare Abhängigkeiten zwischen Modulen herstellen
- verhindern, dass die Applikationslogik direkt MFRC522- oder STM32-Register kennen muss

**Ziel:**  
Aus den einzelnen Versuchen eine nachvollziehbare, geschichtete und wartbare Firmwarearchitektur entwickeln.

---

### Meilenstein 16 – Zustandsautomat integrieren

- Systemzustände definieren
- relevante Ereignisse definieren
- Zustandsübergänge selbst implementieren
- RFID, Sensoren, Servo, Display und Zeitverhalten miteinander verbinden
- ungültige Übergänge verhindern
- Hardwaretreiber nicht direkt über den Systemzustand entscheiden lassen
- Zustandslogik soweit möglich getrennt von Hardwarezugriff testen

**Ziel:**  
Das komplette Systemverhalten über einen klaren ereignisgesteuerten Zustandsautomaten steuern.

---

### Meilenstein 17 – Fehlerfälle und Tests

- widersprüchliche Sensorzustände testen
- fehlende Zielposition des Bolzens testen
- RFID-Kommunikationsfehler simulieren
- SPI-/I²C-Timeouts beziehungsweise Fehlerpfade prüfen
- Reset in verschiedenen Systemzuständen testen
- Kartenentfernung und erneute Zugriffsversuche testen
- Treiberfunktionen auf definierte Fehlercodes prüfen
- Functional Requirements mit Verification verknüpfen

**Ziel:**  
Das System und die selbst entwickelten Treiber nicht nur im Normalfall, sondern auch unter definierten Fehlerbedingungen systematisch prüfen.

---

### Meilenstein 18 – Flash und Persistenz

- Whitelist zunächst aus `const`-Daten herauslösen
- Flash-Aufbau des STM32 verstehen
- relevante Randbedingungen des Flash-Schreibens nachvollziehen
- HAL beziehungsweise vorhandene STM32-Flash-Funktionen für den eigentlichen Hardwarezugriff verwenden
- eigenes Datenformat für die Whitelist definieren
- Daten speichern und wieder einlesen
- ungültige Daten erkennen
- Versionsfeld und gegebenenfalls Prüfinformation definieren
- Verhalten bei unterbrochenem Schreiben betrachten

**Ziel:**  
Persistente Konfiguration sicher und nachvollziehbar umsetzen und dabei Hardwarezugriff von eigener Daten- und Fehlerlogik trennen.

---

### Meilenstein 19 – Watchdog und Robustheit

- sinnvolle Watchdog-Fehlerfälle identifizieren
- Watchdog konfigurieren
- Funktionsweise und relevante Hardwaregrundlagen verstehen
- definiertes Recovery-Verhalten festlegen
- sicheren Systemzustand nach Fehler oder Reset prüfen
- prüfen, an welcher Stelle der Watchdog bewusst zurückgesetzt werden darf

**Ziel:**  
Grundlegende Mechanismen für robuste und wiederanlaufende Embedded-Systeme kennenlernen und nicht nur mechanisch konfigurieren.

---

### Meilenstein 20 – Abschluss und Portfolio

- README vervollständigen
- Requirements aktualisieren
- Architektur dokumentieren
- Schichten zwischen HAL, Gerätetreibern und Applikation darstellen
- Zustandsdiagramm pflegen
- Hardwareaufbau dokumentieren
- Testfälle und wichtige Fehler dokumentieren
- eigenen MFRC522-Treiber und dessen Datenblattbezug dokumentieren
- ausgewählte Registerübungen und daraus gewonnene Erkenntnisse kurz dokumentieren
- Git-Repository aufräumen
- Fotos und optional Demo-Video erstellen

**Ziel:**  
Das Projekt so aufbereiten, dass Aufbau, Funktionsweise, technische Entscheidungen und eigener Entwicklungsanteil für Außenstehende nachvollziehbar sind.

---

### Meilenstein 21 – Vorstellungsgespräch vorbereiten

- zentrale Architekturentscheidungen erklären können
- Rolle von CubeMX und HAL im eigenen Projekt erklären können
- erklären können, was HAL bei GPIO, SPI und Timern grundsätzlich abstrahiert
- ausgewählte STM32-Register und deren Zweck erklären können
- eigenen MFRC522-Treiber vom Registerzugriff bis zum UID-Auslesen erklären können
- Unterschied zwischen STM32-Peripherietreiber und externem Gerätetreiber erklären können
- typische Debugging-Fälle aufbereiten
- verwendete Schnittstellen vergleichen können
- Timer, Interrupts und PWM erklären können
- Zustandsautomat begründen können
- Grenzen der RFID-UID-Authentifizierung erklären können
- eigene Lernfortschritte und Schwierigkeiten reflektieren

**Ziel:**  
Das Projekt nicht nur zeigen, sondern sowohl auf Anwendungs- als auch auf Hardwareabstraktions- und Treiberebene technisch sicher erklären können.

---

## 4. Begleitende Arbeit während aller Meilensteine

### 4.1 Requirements

Die Requirements werden parallel weiterentwickelt.

Insbesondere:

- offene `TBD`-Werte erst festlegen, wenn praktische Erfahrungen vorhanden sind
- neue Fehlerfälle ergänzen, wenn sie während der Entwicklung erkannt werden
- Verhalten präzisieren, wenn die Implementierung Unklarheiten aufdeckt
- Requirements nicht unnötig vor der praktischen Umsetzung vervollständigen

---

### 4.2 Tests und Verification

Tests werden nicht erst am Projektende geschrieben.

Während der Entwicklung:

- für jede wichtige Funktion mindestens einen Testfall definieren
- erwartetes Verhalten vor dem Test festlegen
- Fehlerfälle bewusst testen
- selbst entwickelte Treiberfunktionen separat testen
- möglichst mit kleinen Hardwaretests beginnen, bevor ein Treiber in die Gesamtanwendung integriert wird
- Functional Requirements mit passenden Verification-Methoden verknüpfen

---

### 4.3 Dokumentation

Dokumentation wird fortlaufend ergänzt.

Wichtige Inhalte:

- technische Entscheidungen
- relevante Messwerte
- Hardwareaufbau
- Fehlerursachen
- Debugging-Vorgehen
- Architekturänderungen
- Erkenntnisse aus Tests
- relevante Datenblattstellen
- bei zentralen Peripherien Zusammenhang zwischen HAL-Konfiguration und Hardware-Registern

---

### 4.4 C- und Embedded-Lernen

C-Themen werden möglichst dann vertieft, wenn sie im Projekt praktisch benötigt werden.

Beispiele:

- `enum` bei Zuständen und Events
- `struct` für Modulzustände und Konfigurationen
- Pointer bei Treibern und Puffern
- `static` bei Modulgrenzen
- `const` bei fester Konfiguration
- `volatile` bei Interrupts
- Bitmasken bei Status- und Fehlerflags
- Arrays bei UID-Listen
- Fehlercodes bei Treiberschnittstellen
- Registerzugriffe über `->`
- bitweises Setzen, Löschen und Prüfen von Registerfeldern
- Datenbreiten und Masken bei Kommunikationsprotokollen

Themen wie Speicherlayout, Undefined Behaviour, Integer-Promotion oder Pointerarithmetik werden zusätzlich separat geübt, wenn sie im Projekt nicht ausreichend vorkommen.

---

### 4.5 Regel für Bibliotheken und generierten Code

Fertige Bibliotheken oder generierter Code dürfen verwendet werden, wenn sie nicht den zentralen Lerninhalt ersetzen.

Für zentrale Lernbereiche gilt:

- zuerst verstehen, welche Aufgabe die Bibliothek oder HAL abstrahiert
- wichtige Parameter und Datenflüsse selbst erklären können
- bei geeigneten Stellen Register oder Datenblatt nachvollziehen
- keine Komplettbibliothek übernehmen, wenn gerade deren Treiberlogik gelernt werden soll
- fremden Code nur übernehmen, wenn seine Rolle und Grenzen verstanden werden

Insbesondere gilt für dieses Projekt:

- STM32 HAL: erlaubt
- CubeMX-Codegenerierung: erlaubt
- CMSIS/Registerzugriff: gezielt für Lernübungen
- STM32 LL: vorerst nicht erforderlich
- fertige MFRC522-Komplettbibliothek: nicht für die Kernfunktionalität
- SSD1306-Font-/Zeichendaten: dürfen als Hilfsdaten verwendet werden; grundlegende I²C-/Displaykommunikation soll selbst verstanden und implementiert werden

---

## 5. Erfolgskriterien des Gesamtprojekts

Das Projekt gilt als erfolgreich, wenn:

- das System praktisch funktioniert,
- die wesentlichen Firmwareteile verstanden werden,
- typische Fehler systematisch eingegrenzt werden können,
- wichtige technische Entscheidungen begründet werden können,
- die Rolle von CubeMX/HAL und die darunterliegende Hardware grob erklärt werden kann,
- ausgewählte STM32-Peripherien auch auf Registerebene nachvollzogen wurden,
- mindestens der MFRC522-Gerätetreiber selbst anhand des Datenblatts entwickelt wurde,
- Requirements und Tests nachvollziehbar mit dem System verbunden sind,
- und das Projekt in einem Vorstellungsgespräch technisch erklärt werden kann.
