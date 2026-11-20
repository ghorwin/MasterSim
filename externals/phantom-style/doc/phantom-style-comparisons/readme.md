# Vergleich der Phantom-Style-Varianten

Stand: 20.11.2026

Dieses Verzeichnis enthält einen visuellen Vergleich der Phantom-Style-Varianten, die als Grundlage für
`externals/phantom-style` in Frage kommen, sowie die Skripte, mit denen der Vergleich erzeugt wurde.

**Ergebnis ansehen:** `phatom-fork-comparison/index.html` im Browser öffnen. Die Seite funktioniert
offline (ohne Internet fehlt nur die Webschrift IBM Plex, dann wird die Systemschrift verwendet).
Oben lässt sich zwischen heller und dunkler Palette umschalten und auswählen, welche Varianten
angezeigt werden.


## Inhalt

| Datei | Zweck |
|---|---|
| `gallery.cpp` | Test-Anwendung: baut dieselben Widgets für jeden Stil auf und speichert sieben Bereiche als PNG |
| `CMakeLists.txt` | baut `gallery_<variante>` für jedes Unterverzeichnis in `VARIANTS_DIR`, dazu `gallery_fusion` (Qt-Fusion als Referenz) |
| `fetch_variants.py` | sammelt die Quelltexte der Varianten (Forks von GitHub zu festen Commits, Original aus `../../src`, VICUS aus lokalem Checkout) |
| `render_shots.py` | baut die Galerien und rendert alle Screenshots (hell/dunkel) |
| `make_page.py` | erzeugt `index.html` aus den Screenshots; enthält auch die Beobachtungen je Widget-Gruppe |
| `create_comparison.sh` | dünner Wrapper, ruft die drei Python-Skripte nacheinander auf |
| `phatom-fork-comparison/` | erzeugte Vergleichsseite (`index.html` + `shots/`) |


## Vergleich neu erzeugen

Voraussetzungen: Qt 6 (Widgets), CMake, Compiler, Python 3, Internetzugang (für die GitHub-Forks).

```bash
./create_comparison.sh /pfad/zu/SIM-VICUS
```

- Zwischendateien (Quelltexte der Varianten, Build) landen in `work/` (per `.gitignore` ausgeschlossen).
- Das SIM-VICUS-Repository ist privat, deshalb wird die VICUS-Variante aus einem lokalen Checkout
  kopiert (`externals/phantom/src`). Ohne Pfadangabe wird VICUS übersprungen und fehlt auf der Seite.
- Die Fork-Commits sind in `fetch_variants.py` (`FORKS`) fest eingetragen; für einen Vergleich mit
  neueren Ständen dort die SHAs anpassen.
- Gerendert wird ohne Bildschirm (`QT_QPA_PLATFORM=offscreen`) mit 2-facher Skalierung, Schrift
  Noto Sans 9 pt, mit fest definierter heller und dunkler Palette, damit alle Varianten exakt dieselben
  Farben bekommen.
- Die Beobachtungen auf der Seite stehen als Text in `make_page.py` (`SECTIONS`). Wenn sich Varianten
  ändern, müssen sie dort von Hand nachgezogen werden.

Einschränkungen der Screenshots:

- Hover- und Fokus-Zustände sind nicht zu sehen (keine Maus beim Rendern).
- Der Fortschrittsbalken für unbekannte Dauer (busy) ist animiert und wird als Einzelbild zu einem
  zufälligen Zeitpunkt aufgenommen; `sliders_progress.png` ist deshalb nicht bei jedem Lauf identisch.


## Verglichene Varianten

| Variante | Quelle | Stand | Anmerkung |
|---|---|---|---|
| Original | `externals/phantom-style/src` (Kopie von Qt-Widgets/phantom-style = randrew/phantomstyle von 2020, von uns auf Qt 6 portiert) | – | randrew/phantomstyle: letzter Commit Mai 2021, 42 Forks |
| JulienMaille | github.com/JulienMaille/phantomstyle, Commit `356703a` | 16.09.2026 | 89 Commits vor dem Original, aktivster Fork |
| cristianadam | github.com/cristianadam/phantomstyle, Commit `c654231` | 29.12.2025 | 14 Commits vor dem Original |
| jcelerier | github.com/jcelerier/phantomstyle, Commit `e49cf4e` | 06.10.2026 | 30 Commits vor, 1 hinter dem Original |
| VICUS | SIM-VICUS `externals/phantom`, Commit `266c8ce` | 14.11.2026 | IBK-intern, ca. 7.150 statt 5.200 Zeilen |
| Fusion | Qt 6 eingebaut | – | nur als Referenz |

Alle Forks stehen unter LGPL-2.1. Weitere Forks (vladimir-kraus, NicoG60, beojan, blquinn, erladion,
ekaaty) haben nur 1–17 eigene Commits und sind seit 2021–2024 inaktiv; alle übrigen Forks haben keine
eigenen Änderungen. Sie wurden nicht verglichen.


## Erkenntnisse

### Original (unsere Kopie)

- Klassischer Phantom-Look.
- Für Qt 6 waren mehrere Anpassungen nötig (u. a. `QStringRef` → `QStringView`,
  `QPalette::background()` → `window()`, `SH_ScrollBar_StopMouseOverSlider` →
  `SH_Slider_StopMouseOverSlider`, eigenes `hash_combine` statt `QtPrivate::QHashCombine`);
  die Forks enthalten vergleichbare Korrekturen bereits.
- Schwäche: In der dunklen Palette sind die nicht angehakten Check-Boxen in Baumansichten kaum zu sehen.

### JulienMaille

- Der einzige klar gepflegte GitHub-Fork. Er übernimmt die Änderungen der anderen aktiven Forks:
  cristianadams Qt-6-Port, Dark-Theme-Unterstützung und Tab-Markierung, jceleriers Qt-6.5- und
  Device-Pixel-Ratio-Korrekturen. Dazu kommen Qt-6.11-Korrekturen (fast dieselben wie in unserer
  Kopie) und Absicherungen im Farbcode.
- Neue Funktionen: animierter Busy-Fortschrittsbalken, schmale Scrollbars ohne Pfeiltasten,
  Gruppenbox-Titel auf der Rahmenlinie (Windows-Stil), optionale Gitterlinien im Tabellenkopf,
  Icons in der Menüleiste, Hover-Hervorhebung bei Buttons.
- Sichtbar anderer Look: flacher (abgerundete Ecken abgeschaltet), keine Lichtkante an Rahmen,
  Chevron-Pfeile, gedrückte/umgeschaltete Buttons in der Hervorhebungsfarbe getönt, Slider-Spur bis zum
  Griff gefüllt, blauer Balken am aktiven Tab. Die meisten Unterschiede sind `static const`-Schalter am
  Anfang von `phantomstyle.cpp` und lassen sich zurückstellen.
- Haken: verwendet `PM_ButtonIconLabelMargin`, das nur in einem vom Autor gepatchten Qt existiert.
  Mit Standard-Qt ist das ein Compile-Fehler; nach Entfernen dieses einen `case` (macht
  `fetch_variants.py` automatisch) baut er ohne Warnungen gegen Qt 6.11.
- Schwächen: Das Icon auf einem Push-Button ist bei 2-facher Skalierung unscharf; Issue-Tracker
  auf GitHub abgeschaltet; CMake-Dateien nur für das Plugin, nicht für die Bibliothek.

### cristianadam

- Original plus Qt 6, klein und konservativ: Qt-6-Port, MSVC-Fix, CMake-Dateien für Bibliothek,
  Plugin und Demo.
- Sieht fast aus wie das Original. Neu: Markierungsbalken am aktiven Tab, schmale Scrollbars,
  Chevron-Pfeile in Baumansichten, höhere Zeilen; Check-Boxen in Bäumen bleiben im Dunkelmodus sichtbar.
- Schwäche: Bei angehakten Menüeinträgen mit Icon wird nur das Icon gezeichnet, der Haken fehlt, der
  Zustand ist also nicht erkennbar.
- Seit Dezember 2025 ruhig; der einzige der drei Forks mit aktivem Issue-Tracker.

### jcelerier

- Umgestaltung für die Anwendung ossia score: oranger Akzent, flache Buttons, umgestaltete Combo-Boxen,
  Check-/Radio-Boxen und Scrollbars, Hover-Zustände überall; Slider springen an die Klickposition.
- Mit heller Palette nicht brauchbar: angehakte Check-Boxen und Radio-Buttons, angehakte Tool-Buttons
  und die ankreuzbare Gruppenbox sind kaum zu erkennen. Menü-Buttons haben keinen Pfeil, angehakte
  Einträge in Item-Views werden als Kreuz gezeichnet.
- 5 Warnungen mit Qt 6.11. Wegen der Bindung an den Look einer einzelnen Anwendung keine gute Basis.

### VICUS

- Am stärksten überarbeitete Variante, moderner Fluent-ähnlicher Stil statt des klassischen
  Phantom-Looks. Hängt von keinem anderen VICUS-Code ab und baut ohne Änderungen gegen Qt 6.11 (nur
  Deprecation-Warnungen). Die Akzentfarbe kommt aus der Highlight-Farbe der Palette.
- Merkmale: stärker abgerundete Ecken (auch Popup-Menüs und Rahmen von Item-Views), Default-Button mit
  Akzent-Ring, Check-/Radio-Boxen mit Akzent gefüllt und animiert, Unterstreichungs-Tabs ohne Rahmen um
  die Seite, Auswahl in Views als zarte Akzenttönung mit kurzem Akzentbalken am Zeilenanfang,
  Overlay-Scrollbars, die beim Überfahren breiter werden, Slider mit Akzent-Spur und Ring-Griff,
  Popup-Menüs mit neutral grauer statt Akzent-Hervorhebung.
- Schwäche: Eine Scrollbar außerhalb eines Scroll-Bereichs zeichnet keinen Hintergrund
  (`scrollAreaViewportColor` liefert dort eine ungültige Farbe); im Dunkelmodus erscheinen dadurch
  weiße Streifen. Scrollbars in Views und Textfeldern sind nicht betroffen. Abhilfe wäre eine
  Ersatz-Hintergrundfarbe in `CC_ScrollBar`.
- Hinweis: Der Busy-Fortschrittsbalken wurde kurz nach dem Start aufgenommen und wirkt im Screenshot
  fast leer; im Betrieb läuft ein Balken durch.
- VICUS setzt in der Anwendung zusätzlich eigene Stylesheets für Item-Views; diese gehören nicht zum
  Stil und sind in den Screenshots nicht enthalten.


## Fazit

- **Klassischen Phantom-Look behalten:** cristianadam (wenige, konservative Änderungen) bzw. unsere
  eigene Qt-6-Kopie.
- **Meiste Korrekturen und laufende Pflege auf GitHub:** JulienMaille; flacherer, Windows-ähnlicher
  Look, erfordert die Ein-Zeilen-Anpassung wegen `PM_ButtonIconLabelMargin`.
- **Modernster Look und gleiches Erscheinungsbild wie die anderen IBK-Programme:** VICUS; wird
  intern gepflegt, die Scrollbar-Schwäche müsste noch behoben werden.
- jcelerier scheidet aus (heller Modus unbrauchbar).

Für MasterSim wird vorerst unsere Kopie des Originals (`externals/phantom-style`) verwendet; sie soll
später angepasst werden.
