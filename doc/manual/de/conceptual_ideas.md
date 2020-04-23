
Grundsätzliche Bedingungen
------------------------

Grundsätzlich können gleichbleibende grundlegende Bedingungen erlangt werden, indem wiederholt Eingänge bestimmt werden, Ausgänge von Slaves wiederhergestellt werden **ohne** dazwischen `doStep()` aufzurufen. Dadurch muss der Status der Slaves nicht gespeichert und wiederhergestellt werden, sodass die Wiederholung grundsätzlicher Bedingungen auch mit FMI v1 möglich ist.

Die Ausführung vom Master-Algorithmus kann wiederverwendent werden, wenn die Aufrufe der folgenden Funktionen:

- `doStep()`
- `currentState()`   (only FMI2)
- `setState()` (only FMI2)

durch eine Markierung deaktiviert sind.

Nicht verbundende Eingänge brauchen nur einmal auf ihre Startwerte gesetzt werden. Ebenso müssen Parameter nur einmal durch den Master festgelegt sein *vor* der Wiederholung grundsätzlicher Bedingungen.

Konvergenztest
--------------------------

Für alle sich wiederholenden Algorithmen findet sich eine Annäherung, wenn:

- alle ganzzahligen Werte übereinstimmen
- alle booleschen Werte übereinstimmen
- alle String-Werte übereinstimmen
- die WRMS-Norm aller realen Werte übereinstimmt

Die weighted root mean square norm (WRMS-norm) wird folgendermaßen errechnet:

~~~
#!code.cpp
	double norm = 0;
	for (unsigned i=0; i<m_realytNextIter.size(); ++i) {
		double diff = m_realytNextIter[i] - m_realytNext[i];
		double absValue = std::fabs(m_realytNextIter[i]);
		double weight = absValue*m_project.m_relTol + m_project.m_absTol;
		diff /= weight;
		norm += diff*diff;
	}

	norm = std::sqrt(norm);
~~~


Das Papier kann von der MasterSim-Web-Seite heruntergeladen werden -_ Abschnitt Dokumentation.

Bedeutende Kombinationen von Algorithmen
---------------------------------------------------------

### 1. Gauss-Jacobi (nicht wiederholbar, kein Fehlertest) ###

* funktioniert mit FMI v1 und v2
* keine Wiederholung
* kein Fehlertest
* fixierte Orte für Schritte

Ist limitiert hinsichtlich Fehlern und Stabilität. Optional unterstützt er die Parallelisierung (siehe Diskussion über Leistungsoptimierung).

### 2. Gauss-Seidel (nicht wiederholbar, kein Fehlertest) ###

* funktioniert mit FMI v1 and v2
* keine Wiederholung
* kein Fehlertest
* fixierte Schrittgröße

Einschränkungen von Fehlern und Stabilität. Ein wenig besser als Gauss-Jacobi.

### 3. Gauss-Seidel (nicht wiederholbar, kein Fehlertest) ###

* arbeitet mit FMI v2
* keine Wiederholung
* Fehlertest
* anpassungsfähige Schrittgrößen, die Schrittgröße ist angepasst, basierend auf der Annäherungsrate

### 4. Gauss-Seidel (wiederholbar, festgelegte Schritte, mit optionalem mutmaßlichem Fehlertest) ###

* benötigt FMI v2
* nutzt Wiederholungen
* optionaler Fehlertest, das Ausfall eines Fehlertests stoppt den Master
* fixierte Schrittgröße, ein Scheitern der Annäherung wird den Master stoppen.

Das Fixieren der Zeitschritte erlaubt einen Leistungsvergleich mit anderen Algorithmen. 

### 5. Gauss-Seidel (Wiederholung mit optionalem Fehlertest) ###

* benötigt FMI v2
* benutzt Wiederholungen
* optionaler Fehlertest, ein Ausfall Fehlertests stoppt den Master
* anpassungsfähige Schrittgröße (Reduktion bei Annäherungsfehlern und Anstieg bei schneller Annäherung, Reduktion beim Scheitern des Fehlertests)

Stabil durch die Zeitschrittreduktion, Fehlerkontrolle möglich.

### 6. Newton (Wiederholung mit optionalem Fehlertest) ###

* benötigt FMI v2
* nutzt Wiederholungen
* optionaler Fehlertest
* anpassungsfähige Schrittgröße (Reduktion bei Annäherungsfehlern und Anstieg bei schneller Annäherung, Reduktion beim Scheitern des Fehlertests)

Stabil durch die Zeitschrittreduktion, Fehlerkontrolle möglich. Annäherungsrate und Erfolg verbessert über Gauss-Seidel.

## FMU-Vorausetzungen ##

Variieren nach den ausgewählten Algorithmus-Optionen, FMU muss gesicherte Fähigkeiten haben:

### Der Gebrauch der Zeitschrittanpassung ###

* FMU kann variable Zeitschritte handhaben

### Der Gebrauch von Wiederholung ###

* FMU kann zurückgesetzt werden (FMI v2)

### Der Gebrauch der Fehlerkontrolle mit der Zeitschritt-Anpassung ###

* FMU kann zurückgesetzt werden (FMI v2)
* FMU kann variable Zeitschritte handhaben


Master-Algorithmen
--------------------------

### Startbedingungen ###

Alle Algorithmen beginnen mir den folgenden Bedingungen:

* alle Slaves befinden sich im Zeitlevel t
* die Ausgangsvariablen-Zwischenspeicher der Slaves werden auf den Stand des Zeitlevels t gebracht
* globale variable Vektoren werden auf den Stand des Zeitlevels t gebracht

Wenn die Wiederholung *und* die Schrittanpassung möglich ist, wird erwartet, dass der FMU-Slave-Status bereits gespeichert ist. Andernfalls werden sie zu Beginn einer Wiederholung eines Algorithmus gespeichert.  

### Gauss-Jacobi ###
**Notiz:** MasterSim realisiert nur den sich nicht wiederholenden Gauss-Jacobi-Algorithmus, weil zwei Gauss-Jacobi-Wiederholungen mit zwei sich nicht wiederholenden Schritten mit halber Schrittgröße übereinstimmen - und die letztere Version ist noch akkurater und stabiler. Deshalb wird der Gauss-Jacobi immer ohne Wiederholung, Fehlerprüfung und Zeitschritt-Anpassung umgesetzt.

~~~
Wiederholen aller Zyklen:
  
  Wiederholen aller Slaves im Zyklus:

    setzen der Eingänge für Slaves die Variablen des Zeitlevels t
    rechtzeitiges fördern des Slave (doStep() und das Aufnehmen der Ausgänge im Zwischenspeicher)
    synchonisieren der gecachten Ausgänge mit variablen Vektoren für das Zeitlevel t+1

kopieren der Variablen vom Zeitlevel t+1 zum variablen Vektor des Zeitlevels t
~~~

### Gauss-Seidel-Wiederholungen ###

Ob die sich wiederholende oder die nicht wiederholende Version benutzt wird, ist durch das Wiederholungs-Limit (==1, no iteration) eingeschränkt.

Gelöst ist die Festpunkt-Wiederholung

     x* = S(x)

Wo `S(x)` das Resultat der Beurteilung aller Slaves ist und die Abbildung der Aus- und Eingänge. 


~~~
kopieren der Variablen des Zeitlevel t zu dem Variablenvektor des Zeitlevels t+1

wiederholen aller Zyklen:

  Wiederholungsschleife < max.Wiederholungen:
  
    bei Wiederholungen:
      kopieren von Variablen aus dem Zeitlevel t+1 zum Backup-Vektor (für einen Annäherungs-Check)
      bei einer Wiederholung > 1:
        restoreSlaveStates()
        
    wiederholen aller Slaves im Zyklus:

      setzen von Eingängen für Slave-nutzende Variablen des Zeitlevels t+1 (partiell aktualisiert durch vorherige Slaves)
      rechtzeitiges fördern von Slaves (doStep() und das Caching von Ausgängen)
      synchronisieren von gecachten Ausgängen mit variablen Vektoren für das Zeitlevel t+1

  bei Wiederholung und numSlaves > 1:
    doConvergenceTest()
    bei Erfolg:
      abbrechen

kopieren von Variablen vom Zeitlevel t+1 zum variablen Vekor vom Zeitlevel t
~~~

Spezielle Besonderheiten der Gauss-Seidel-Umsetzung:

TODO: Korr.Orig.s.u. dicontinuous

- für Zyklen mit einem Slave wird keine Wiederholung durchgeführt, somit ist ein Rollback notwendig
- falls Slaves nicht kontinuierliche Ausgänge melden (vom Statusereignis/Zeitereignis) wird Gauss-Seidel eventuell nicht konvergieren, falls die Reduktionsschleife des äußeren Zeitschritt den Zeitschritt unter ein sicheres Limit reduziert, fällt der Algorithmus zurück auf *nicht wiederholender Gauss-Seidel*

### Newton ###
Der Newton-Algorithmus ist immer wiederholend. Der Algorithmus verwendet eine modifizierte Newton-Methode, bei der der Jacobische nur einmal, zu Beginn eines jeden Schrittes, aktualisiert wird.

Die Ursache der Probleme zu finden stammt aus der re-arrangierten fixierten Wiederholung:

    0 = x - S(x) = G(x)
    
mit dem Jacobischen

    dG/dx = I - dS/dx . 

In dem Algorithmus wird jeder Zyklus individuell behandelt. Zyklen mit einem einzigen Slave werden nicht wiederholt. Zyklen ohne gekoppelte Variablen ( das heißt, keine echten Zyklen) werden nicht wie Newtonsche behandelt.

~~~



~~~

Einrichtungsmethode der Zeitschritte
-------------------------------------------

Variable Schritte der Datenübertragung sind nur für FMUs v2 mit der Fähigkeit zur Wiederholung implementiert.

Prinzipieller Algorithmus:

~~~
Schleife bis t > tEnd:
  Schleife bis das Konvergieren und der Fehlertest abgelaufen sind:
    einen Schritt machen
    bei fehlender Konvergenz:
      Schritt reduzieren und erneut versuchen
    Fehlertest durchführen
    wenn der Fehler zu groß ist:
      Schritt reduzieren und erneut versuchen
  
 einschätzen der Schrittgröße für den nächsten Schritt
~~~

### Fehlerhafte Einschätzung ###

Der Fehlertest wird mit der Technik der Schrittverdopplung durchgeführt.. Der erste Schritt wird berechnet (er wurde an den Fall eines sich wiederholenden Algorithmus angeglichen). Der selbe Schritt wird dann noch einmal mit zwei Schritten halber Größe berechnet, eine Lösung einer höheren Ordnung ergebend.
Die Differenz zwischen den methodischen Ordnungen wird genutzt, um den lokalen Trunkierungsfehler einzuschätzen. Dies wird in Analogie zu 'Backward Euler' getan, welches die methodische Ordnung 1 hat. Angenommen, Sie haben die Lösung yh als Lösung mit dem originalen Schritt und das Ergebnis y2 mit halben Schritten, so wird der Fehler geschätzt mit:

    error = 2 || yh - yh2 ||
    
Seit wir mit Vektoren operieren, wird eine geeignete Norm (WRMS norm) für die Berechnung der Skalare yh und yh2 genutzt.


### Durchführung ###

Der Fehlertest ist optional und wurde richtig durchgeführt, nachdem das Ergebnis sich angenähert hat. In diesem Fall wird die neue Lösung (für den Zeitpunkt t + Schrittgröße) im temporären Vektor 'xxxNext' gespeichert. 

Der Fehelertest-Algorithmus kopiert diese Lösung zunächst zum Vektor xxxFirst. Danach setzt er die Slaves auf den Zeitpunkt t zurück und berechnet zwei Schritte der Größe Schrittgröße/2. Falls irgend einer dieser Schritte während dieses Versuchs bei der Annäherung scheitert, wird die Schrittgröße **nicht** angepasst, sondern der Fehlertest als gescheitert markiert (wenn sich der lange Schritt annähert, muss sich auch der kleinere annähern, es sei denn, irgendetwas Maßgebliches ist in der Mitte des langen Schrittes passiert).

### Abrunden der Fehlerberücksichtigung ###

Wenn zwei Schritte der Größe h2 = h/2 genommen werden, kann das Runden von Fehlern zu einem Status, der beim Zeitpunkt `2*h2 != h` endet, führen. Da das Ende der Fehlertest-Slaves bei der Zeit `t + 2*h2` positioniert ist, berechnen wir die aktuelle lange Schrittgröße `h = 2*h2` neu und speichern sie als lange Schrittgröße.

## Geschätzte Ergebnisse mittels 'Richardson Extrapolation' verbessern  ##

Für den Fehlertest müssen wir 3 Schritte der Datenübertragung nutzen anstatt von einem für ein einziges Intervall. Diese extra Arbeit kann ebenso genutzt werden, um eine bessere Schätzung für die Lösung zu erstellen, indem die Lösung des einzelenen Schrittes mit der Lösung des dualen Schrittes (mit einer höheren Ordnung) kombiniert wird. Mit der 'Richardson Extrapolation' werden beide Lösungen kombiniert, um eine bessere Schätzung des Ergebnisses zu erzeugen.

== Modifizierung/Fixierung des FMU-Inhalts


FMU-Import


Obwohl zwei Bibliotheken zum Import von FMUs existieren (FMILibrary und FMI++), arbeitet keine ausreichend gut in diesem Zusammenhang. Die FMILibrary wurde angelegt, doch die angebundene Ausgabe auf Linux/MacOS, FMI++ unterstützt das Importieren von FMUs für die CoSimulation v.2 nicht.

Da der Master lediglich die Funktion des Importierens benötigt, ist der direkteste Weg damit umzugehen, die Funktion zum Importieren manuell zu erzeugen mit dem Fokus auf diese Aufgaben:

* Entpacken von FMUs (wird mittels Bibliotheksfunktion 'miniunzip' erledigt)
* Lesen der 'ModelDescription.xml'
* Laden dynamischer Bibliotheken und Importieren der Funktionshinweise (plattformspezifisch, für Windows und Mac/Linux)
* Verpacken von Aufrufen für FMUs, sodass der Master-Algorithmus mit einer komfortableren Schnittstelle arbeiten kann in Ergänzung zu den ursprünglichen Schnittstellenfunktionen des fmi.

Für das Koppeln mit der gemeinsamen Bibliothek/DLL einer FMU werden die Funktionshinweise und Rückruffunktionen gebraucht. Die Rückruffunktionen sind typischerweise unabhängig von der individuellen FMU oder vom Slave und werden nur einmal für FMI1 und FMI2 innerhalb des Modell-Managers definiert.

Die Funktionshinweise sind für jede geladene dynamische Bibliothek (FMU) genau bezeichnet, wie auch die ModelDescription. Somit ist ein FMU durch diese Eigenschaften kodiert:

* der Pfad zum FMU und Entpack-Verzeichnis
* Funktionshinweise
* der Kontext der Modellbeschreibung

Dies ist alles in der Kategorie FMU gespeichert.

### Entpacken von FMU / Fehlerbeseitigungs-Funktionen ###

Das Standardverhalten des Master-Simulators sollte es sein, alle referenzierten FMU-Dateien zu entpacken. Dennoch kann es für den Zweck der Fehlerbeseitigung sinnvoll sein den Entpack-Teil zu überspringen und den Master anzuweisen, bereits entpackte FMUs zu nutzen. In diesem Fall sollte der Master versuchen, die modelDescription.xml-Datei an dem Ort, wo sie nach dem Entpacken verortet wäre, zu lesen.

Diese Funktionalität wird durch das Befehlszeilenargument *skip-unzip* [siehe Befehlszeilen-Argumente](CommandLineArguments) ausgelöst. Dies ist besonders nützlich, falls Sie verschiedene Optionen in der `modelDescription.xml`-Datei ausprobieren wollen, ohne jedesmal das FMU-Archiv extrahieren/komprimieren zu müssen.

#### Pfad zum FMU-Entpacken ####

Standardmäßig werden FMUs in einem Pfad jeweilig zum Arbeitsverzeichnis mit dem, einmalig aus dem FMU-Dateinamen erzeugten Pfad-Namen, entpackt und im Falle einer Uneindeutigkeit mit einem zusätzlichen Zähler versehen. Grundsätzlich wird für jede zu entpackende FMU, ein Pfad erzeugt. Falls ein anderes FMU mit demselben Dateinamen (aber vielleicht mit anderem relativem/absoluten Pfad) schon vorher extrahiert wurde, wird der FMU-Pfad mit der Schablone 

    <working-dir>/fmus/<filename-without-fmu-extension>_<counter>
  
formiert, wo der Zähler wächst bis ein eindeutiger Datei-Pfad gefunden wird.  

Der Benutzer möchte das vielleicht außer Kraft setzen, indem das Entpack-Verzeichnis für ein oder mehr FMUs spezifiziert wird. Dies sollte am besten in der Master-Projekt-Datei getan werden, als optionale Meta-Data. Fehlerkontrollen müssen gemacht werden, um einen Nutzer vorm Überschreiben eines Pfades anderer FMUs zu bewahren. 

### Die Arbeitsverzeichnisse der FMU-Slaves ###

Jedes Exemplar einer FMU wird als Simulations-*Slave* bezeichnet. Aus einem einzigen FMU-Archiv können mehrere Realisierungen erzeugt werden. Daher ist das Arbeitsverzeichnis für einen Simulations-Slave ein anderes als das Extraktions-/Entpack-Verzeichnis einer FMU-Datei.

Innerhalb des Masters, wird der Datei-Pfad zu jedem Arbeitsverzeichnis eines Slaves aus einem einzigartigen Simulations-Slave-Namen erzeugt, welches in der Master-Datei definiert ist. 

Da Slave-Namen einzigartig sein müssen, ist der entsprechende Datei-Pfad ebenfalls einzigartig. 

    <working-dir>/slaves/<slave-name>
 
Die Namen der Slaves müssen den Regeln folgen, die für erzeugte Dateinamen gelten. Der erzeugte Verzeichnisname passiert jeden Slave via Parameter, welcher das FMU erfordert, um einen solchen Pfadparameter für die Ausgangsdatei zu unterstützen.

#### Umgang mit Zeiteinheiten ####

Die Masterzeit (und die Zeit, die an FMUs gesendet wird) ist nicht fest an irgendeine naturwissenschaftliche Einheit gebunden. Deshalb wird jeder Ausgang von einem Master, der Bezug nimmt auf Zeitpunkte, nur den anonymen Zeitstempel nutzen. Allerding ist es aus praktischen Gründen schwierig, einen Fehler, der zur Zeit t=1284382.21 auftritt, zu interpretieren.

Innerhalb des Mastercodes wird jeder Zeitausgang in einer Funktion des Zeit-Distanz-Formats formatiert, welche konfiguriert werden kann, um anonyme Zeitstempel zu nutzen oder um Masterzeit als Zeitversatz in Sekunden zu interpretieren. Darüber hinaus kann sie genutzt werden, um Bezugszeit zu referenzieren, die der Masterzeit zugefügt wird (ausgleichend), mit dem Ziel, bedeutende lokale Zeitstempel zu bekommen.

**Konzept**:
Die vom Master genutzte Einheit (die auch vereinbart ist für alle Slaves, die im gleichen Szenario genutzt werden) können durch die Projektdatei-Parameter spezifziert werden. Anschließend, vor den Ausgängen, wird die Masterzeit von dieser gegebenen Einheit in Sekunden umgewandelt, bevor irgendeine Ausgabe stattfindet. 



# Besonderheiten der Anwendung #

## ResultsRootDir-Parameter ##
Immer wenn ein FMU einen String-Parameter mit Namen `ResultsRootDir` festlegt, wird der Parameter automatisch zum oben beschriebenen Dateienstandort zugefügt, es sei denn der Benutzer hat den Parameter manuell spezifiziert.

## Referenzen zur Simulationszeit Zeit/Datum ##

Immer wenn ein FMU einen String-Parameter mit Namen `ReferenceTimeStamp` festlegt, setzt der Master die Referenzzeit/-datum automatisch und konsequent unter die FMUs.

TODO: Determine and fix format for time stamp


# Wissenschaftliche Publikationen #

* Nicolai, A.: *Co-Simulations-Masteralgorithmen - Analyse und Details der Implementierung am Beispiel des Masterprogramms MASTERSIM*, http://nbn-resolving.de/urn:nbn:de:bsz:14-qucosa2-319735
> This publication (in german) analyses a test case using different algorithms and parameters and discusses obtained results (and errors) in detail.

* Nicolai, A.: *Robust and accurate co-simulation master algorithms applied to FMI slaves with discontinuous signals using FMI 2.0 features*, Proceedings of the 13th International Modelica Conference, 2019, https://modelica.org/events/modelica2019/proceedings/html/papers/Modelica2019paperP04.pdf
> A discussion of the slope-based error test procedure and a best-practice approach on adapting time steps with FMI v2.0 features.

* Schierz T., Arnold M., and Clauß C.; *Co-simulation with communication step size control in an fmi-compatible master algorithm*, In 9th International Modelica Conference 2012, Modelica Association, 2012
> This publication discusses the local error estimation technique and time step adjustment algorithm implemented in MasterSim.


# Algorithmische Ideen/Konzepte #

### Verdichtete Ausgabedateien ###

Ausgänge könnten weiter gefiltert werden (um Speierplatz zu reduzieren), indem *nicht viel geändert werden muss*. Dies benötigt einen zusätzlichen Test, sodass Ausgänge nur dann geschrieben werden, wenn:

- die Differenzen zwischen berechneten Werten und interpolierten Werten sich zu stark unterscheiden, sodass das *letzte* Intervall geschrieben wird.

Dieser Algorithmus kann individuell für jede Ausgangsmenge umgesetzt werden und sich als Ausgangsdatei mit unterschiedlichen Zeitpunkten ergeben (aber mit höchstmöglicher Genauigkeit). Die Idee ist das, wenn Ausgangsdateien analysiert werden, der Handlungsverlauf normalerweise mit Polygonen durch Datenpunkte erzeugt wird, welche lineare Interpolationen an allen Punkten zwischen Proben von Ausgangsdaten implizieren. Wenn allerdings drei Beispieldaten in dieser Weise ausgerichtet sind, dass die mittlere nah genug am interpolierten Wert liegt, kann dieser Ausgang übersprungen werden.

Der Algorithmus sieht folgendermaßen aus:

- Erhalt von gegenwärtigen und letzten Beispieldaten, um (tp_1, v_1), (tp_2, v_2)zu schreiben. 
- Erhalt neuer Ausgänge (tp_3, v_3)
- Berechnen interpolierter Werte bei tp_2 using v_1 und v_3 --> v_2intp
- Vergleich alter und neuer interpolierter Werte:  

     abs((v_2- v_2intp)/(max(v_2, v_2intp)+eps)) > Grenzwert
     
- Wenn der Grenzwert überschritten wird, Speichern von tp_2,v_2 in der Datei, Verändern von Werten 2 -> 1.
- Änderung Muster 3 -> Muster 2

****

