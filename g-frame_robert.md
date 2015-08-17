# G-Frame

## Programmierung generell
####Fehlermeldungen
##### duplicate symbol in
Bedeutet, das irgendwelche quellen doppelt kompiliert werden oder denen gleichen Namen. Beispielsweise wenn zwei Main-Funktionen vorhanden sind. Kann durchaus pasieren wenn unnötigerweisen Beispielprogramme mit kompiliert werden.
Stackoverflow dazu: [https://stackoverflow.com/questions/3380972/xcode-duplicate-symbol-main](http://)  
			
	Ah..I figure out it's that I have multiple entries under Targets/Compiled Sources. I removed them and the problem is solved. The multiple entry thing probably has to do with Git merge.
##### ld: warning: directory not found for option
[https://stackoverflow.com/questions/9458739/ld-warning-directory-not-found-for-option](http://)  
	
	If it is a "directory not found for option '-L/..." error That means it's a Library Error, and you should try to:
    Click on your project (targets)
    Click on Build Settings
    Under Library Search Paths, delete the paths
    
	If it is a "directory not found for option '-F/..." That means it's a Framework Error, and you should try to:
	Click on your project (targets)
    Click on Build Settings
    Under Frameworks Search Paths, delete the paths

Might happens when you move referenced file around

##### Runtime error wenn keine Netzwerkverbindung besteht
Im normalfall muss immmer ein Netzwerk vorhanden sein, da das iPad via Wifi verbunden ist. Möglicherweise sollte trotzdem eine Fehlerbehandlung oder ähnliches stattfinden, falls die Verbindung während des Betriebs abbricht.

## Styleentwicklung

## ScrizzleStyle
Der Style besteht aus mehreren (im Moment 3) Linien, die um die Originale gezeichnete Linie sinusförmig pulsieren.

#### Probleme

##### Multiple Strokes
Ich stehe gerade vor  der Frage, wie ich aus einem Pinselstrich, den mir die Maus bzw. Toucheingabe liefert, mehrere Strokes erzeugen kann. Die zusätzlichen Strokes sollen mit zufallszahlen ergänzt werden.
Ein Erzuegen der Strokes in der Rendermethode würde dazu führen, das bei jedem Renderdurchgang auch die Zufallzahlen neu generiert werden, was jedoch ausdrücklich nicht gewünscht ist.
###### Lösung
Zufallszahlen sind nicht nötig. Durch verschiedene Parameter wie Amplitude, Wellenlänge und Änderungsgeschwindigkeit sind die Linien ausreichend unterschiedlich.

## Output für brasilianische LED-Screens

Um zwischen den verschiedenen Augabemodi umschalten zu können, ist es nötig eine Variable zu setzen welche festlegt, wie das Bild ausgeben werden soll. Die Modi und die zugehörigen Tasten sind:
	
	1 - LED_1
	2 - LED_2
	3 - SESI_FACADE
	4 - PROJECTOR

Je nach eingestelltem Modus wird die Ausgaben Größe eingestellt und eventuell weitere Anpassungen vorgenommen (siehe SESI-Fassade).

#### SESI Facade

Die genaue Ansteuerung der Fassade ist noch unklar. Die Spezifikationen, die wir erhalten haben fordern die Ausgabe via Syphon Server mit den Dimensionen 1024 x 768, wobei der eigentliche Bereich, den die Fassade am Ende darstellt nur ein kleiner Teilbereich des Bildes ist.
![image](/Users/rob/Documents/The Constitute/g-frame/brasil/SP Urban MASK G-Frame.png)
Um die Ausgabe von einem rechteckigen Bild auf die 3 Panels anzupassen haben ich auch den Quellcode von *Thiago Hersan* ([link](https://github.com/thiagohersan/QuarterFramePerSecond/blob/4586264eac6e25087978a1810b32f358884d9064/openFrameworks/QuarterFramePerSecond/src/ofApp.cpp#L174-L196)) orientiert, welchen ich von Lina bekommen habe.
Das generelle Vorgehen hier sieht folgendermaßen aus:

	mCanvas.allocate(1024,768,OF_IMAGE_COLOR);
    mCanvas.grabScreen(0, 0, 1024, 768);
    mCanvas.resize(mPanelPositionAndSize.width, mPanelPositionAndSize.height);
    toPanels(mCanvas, mPanels);`

Das originale Bild, also die eigentliche Ausgabe der gFrame-App, wird in einer Texture `mCanvas` gespeichert.
Diese wird per `resize(216,167)` auf die Dimensionen der LED-Fassade gebracht. Der Aufruf von `toPanel` sorgt schließlich für die Anpassung auf die LED-Panels.  
Das angepasste Bild muss nun noch an der richtigen Position auf dem "Bildschirm" gezeichnet werden und per Syphon Server ausgegeben werden. Dies geschieht mit Hilfe eines *Framebuffer-Objekts* in der `update()`-Methode der GFrame-App:

	ofFbo tempFBO;
    tempFBO.allocate(1024, 768);
    tempFBO.begin();
    fiespMask.draw(0,0);
    ofBackground(128);
    mPanels.draw(37,259);
    tempFBO.end();
    syphonMainOut.publishTexture(&tempFBO.getTextureReference());

#### LED 1 & LED 2

Je nachdem wie die beiden Screens angesteuert werden genügt es vermutlich einfach die Ausgabe Auflösung anzupassen. Im Moment wird die Drücken von *1* oder *2* einfach die Fenstergröße der GFrame-App angepasst und der komplette Screen per Syphon ausgegeben.

## Einbindung des Multitouch Rahmens

Für die Einbindung des Multitouch-Rahmens habe ich bisher auf ein OpenFrameworks-PlugIn zurück gegriffen, welches speziell für (von?) PQLabs (dem Rahmenhersteller) entwickelt wurde. Der Rahmen kann aber auch mittels TUIO-Protokoll kommunizieren. Das ist hilfreich, da die TUIO-Funktionalität auch zum Testen mit anderen Touch-Devices (z.B. iPad) bereits implementiert ist. Somit entfallen zusätzliche Eventhandler. Außerdem hat das ofxPQLabs Plugin immer wieder für nicht identifizierbare Fehler gesorgt.
Das PQLabs-Plugin verursachte außerdem Schwierigkeiten in Zusammenspiel mit ofxXMLSettings

## Abspeichern von Einstellungen

Da das PQLabs-Addon nicht mehr verwendet wird steht auch der Nutzung von ofxXmlSettings nichts mehr im Wege. Jetzt muss überlegt werden welche Einstellungen gespeichert werden müssen bzw. vor dem Programmstart editierter sein müssen.

##DMX
Anschlussbelegung am LED Treiber:

	COM blau
	CH1 weiß
	CH2 rot
	CH3 braun

Speakon Stecker

	2+ -> +
	1+ -> R
	1- -> G
	2- -> B

Anschlußbelegung am Rahmen

	KABEL	 LED
	------------
	blau	 +
	rot		 grün
	grün     rot
	schwarz	 blau
	