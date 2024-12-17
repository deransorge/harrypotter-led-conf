# Benutzeranleitung für das LED-Controller-System mit ESP32

## Voraussetzungen:

Ein Smartphone, Tablet oder PC mit WLAN und Browser.
### 1. Erstinbetriebnahme
Stromversorgung anschließen:
Schließe die Lampe an eine passende Stromquelle an (z. B. USB 5V).

WLAN-Access Point suchen:
Nach dem Start sendet der ESP32/die Lampe ein eigenes WLAN aus, wenn er noch nicht mit einem bestehenden WLAN verbunden ist. Dieses WLAN heißt standardmäßig: **ESP32_Setup** <br />
Das Passwort lautet: **12345678** (falls im Code nicht anders definiert).

Mit dem Setup-WLAN verbinden:
Öffne auf deinem Smartphone/Tablet/PC die WLAN-Einstellungen und verbinde dich mit dem Netzwerk **ESP32_Setup**. Gebe das Passwort ein, falls nötig.

Einrichtungs-Webseite aufrufen:
Öffne einen Browser (Chrome, Firefox, Safari etc.) und geben Sie in die Adresszeile ein:
**http://192.168.4.1**
Daraufhin erscheint die Einrichtungsseite des ESP32-LED-Controllers.

### 2. WLAN-Konfiguration
Auf der Einrichtungsseite werden alle verfügbaren WLAN-Netzwerke angezeigt. Gehe wie folgt vor:

WLAN auswählen:
Wähle aus der angezeigten Liste dein heimisches WLAN aus.

Passwort eingeben:
Gebe das Passwort deines WLAN-Netzwerks ein.

Speichern:
Klicke auf **"Speichern"**. <br />
Der ESP32/die Lampe speichert nun die WLAN-Daten und startet neu.

  > [!Important]
  > Sollte der ESP32 sich erfolgreich mit deinem WLAN verbinden, sendet er kein eigenes Setup-WLAN mehr aus. Stattdessen bezieht er eine IP-Adresse aus deinem Heimnetzwerk.

3. Zugriff über das eigene Heimnetzwerk
Nach dem Neustart versucht der ESP32, sich mit deinem Heim-WLAN zu verbinden. Schaue in deinem Router nach, welche IP-Adresse dem ESP32 zugewiesen wurde (z. B. in der Geräteliste des Routers). <br />

Rufe anschließend von einem Gerät, das im gleichen WLAN ist (Smartphone, PC, etc.), im Browser die IP-Adresse des ESP32 auf, z. B.:
**http://[Ihre-ESP32-IP]**

### 4. LED-Steuerungsseite
Auf dieser Seite findest du:

Farbwahl:
Ein Farbfeld erlaubt dir, die gewünschte Farbe für alle LEDs auszuwählen. <br />
Wähle eine Farbe per Klick im Farbwähler und klicke auf **"Farbe setzen"**. Die LEDs leuchten anschließend in dieser Farbe, sofern kein aktiver Effekt läuft, der die Farbe überschreibt.

Effekt-Auswahl:
Ein Dropdown-Menü bietet verschiedene Lichteffekte an, wie z. B.:

Statische Farbe
Blinken
Füllen (Laufender Fülleffekt)
Rainbow-Füllen (Regenbogeneffekt)
Blitz (Lightning)
Dementor Alarm (flackernd, unheimlich)
Disco (zufällige Farben)
Feuer (Flackernde Feuereffekte)
Meteor (Laufender Lichtpunkt)
Regentropfen (Farbtropfen, die verblassen)
Wähle einen Effekt aus und klicke  auf "Effekt setzen". Die LEDs zeigen nun das gewählte Muster.

LEDs Ein/Aus:
Ein Button ermöglicht das Ein- oder Ausschalten der LEDs, ohne die Konfiguration zu verlieren. Klicke auf **"LEDs Ein/Aus"**, um zwischen An und Aus zu wechseln.

> [!WARNING]
> WLAN-Daten zurücksetzen: <br />
Sollte es notwendig sein, die WLAN-Konfiguration des ESP32 zurückzusetzen, klicke auf den entsprechenden Button. Danach startet der ESP32 neu und sendet wieder sein eigenes Setup-WLAN aus, sodass du erneut eine Konfiguration vornehmen kannst.

> [!Important]
> Dank der asynchronen Kommunikation (AJAX) bleibst du auf derselben Seite, während deine Befehle im Hintergrund ausgeführt werden. Du siehst also direkt die Wirkung deiner Aktionen auf die LEDs, ohne dass sich die Seite neu lädt.

### 5. Tipps und Hinweise
Abstand zum Router:
Stelle sicher, dass der ESP32 eine stabile WLAN-Verbindung hat. Bei schwachem Signal kann es länger dauern, bis die IP im Heimnetz sichtbar wird.

Speicherung der Daten:
Der ESP32 speichert die eingegebenen WLAN-Daten dauerhaft. Auch nach einem Stromausfall verbindet er sich erneut mit dem gespeicherten Netzwerk.

Farb- und Effekteinstellungen:
Die vorgenommenen Einstellungen (gewählte Farbe, Effekt, LEDs an/aus) werden bei einem Neustart nicht zwingend dauerhaft gespeichert. Der Standardmodus nach dem erfolgreichen Einrichten ist in der Regel alle LEDs rot leuchten zu lassen.

Mit dieser Anleitung kannst du deinen ESP32-basierten LED-Controller komfortabel einrichten, ins heimische WLAN integrieren und von dort aus jederzeit Farbe, Effekte und den Zustand der LEDs steuern. Viel Spaß bei der Lichtershow!
