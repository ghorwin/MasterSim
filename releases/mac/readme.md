# MasterSim – Deployment auf macOS

Dieses Verzeichnis enthält alles, um aus dem Quelltext ein auslieferbares Disk-Image
(`MasterSim_<version>_macos_universal.dmg`) mit Oberfläche **und** Kommandozeilen-Solver zu erstellen.

Das frühere Verfahren (Python-`createRelease.py`, `deploy.sh.in`, `qt_menu.nib`, Ruby-`mkdmg`,
Qt 5 aus fest verdrahteten Jenkins-Pfaden) ist durch ein einzelnes Shell-Skript ersetzt.

## Was wird ausgeliefert?

```
MasterSim <version>  (dmg, APFS, lzma-komprimiert)
├── MasterSim.app                      Universal Binary (arm64 + x86_64), macOS ≥ 11
│   └── Contents
│       ├── Info.plist                 aus Info.plist.in erzeugt (Version, Bundle-ID, ...)
│       ├── MacOS/mastersim-gui        Oberfläche
│       ├── MacOS/mastersim            Kommandozeilen-Solver (wird von der GUI hier gesucht)
│       ├── Frameworks/Qt*.framework   von macdeployqt eingebunden
│       ├── PlugIns/                   Qt-Plugins (platforms, styles, imageformats, ...)
│       └── Resources/                 Icon, translations/, examples/ (data/examples/darwin64)
├── Applications -> /Applications      Drag & Drop-Installation
├── MasterSim_Manual.pdf / MasterSim_Handbuch.pdf
├── MasterSim - Online Manual/Handbuch.webloc
├── ReadMe.txt                         Hinweise für Anwender (CLI, Rosetta)
└── LICENSE.txt
```

Der Solver liegt bewusst *im* Bundle: So gibt es nur ein Element zum Installieren, die GUI findet
ihn ohne Konfiguration (`MSIMViewSimulation.cpp` sucht `<appDir>/mastersim`), und er wird
zusammen mit der App signiert/notarisiert. Kommandozeilen-Nutzer rufen
`/Applications/MasterSim.app/Contents/MacOS/mastersim` auf oder legen sich einen Symlink an.

## Voraussetzungen

| Werkzeug | Hinweis |
|---|---|
| Xcode bzw. Command Line Tools | `clang`, `codesign`, `install_name_tool`, `notarytool`, `stapler` |
| Qt 6 (universal) | z. B. `~/Qt/6.9.3/macos` (Online-Installer oder `aqtinstall`); sonst `AQT_QT_PREFIX` setzen |
| CMake | im `PATH`, oder automatisch aus `~/Qt/Tools/CMake`, `/Applications/CMake.app`, Homebrew |
| *optional:* Apple Developer ID | für eine Weitergabe ohne Gatekeeper-Warnung |

Die Qt-6-Frameworks des Online-Installers sind bereits Universal Binaries; MasterSim selbst wird über
`MSIM_UNIVERSAL_BINARIES` (Standard auf macOS) für `arm64;x86_64` gebaut. Mindest-macOS ist 11.0
(`CMAKE_OSX_DEPLOYMENT_TARGET` im Top-Level-`CMakeLists.txt`).

## Schnellstart

```bash
releases/mac/create_dmg.sh
```

Ergebnis: `releases/mac/dist/MasterSim_<version>_macos_universal.dmg` (`dist/` ist in `.gitignore`).

Ohne Developer ID wird ad-hoc signiert. Das Image funktioniert dann auf dem eigenen Mac; auf
anderen Rechnern blockiert Gatekeeper den ersten Start (Abhilfe dort: Rechtsklick → Öffnen, bzw.
Systemeinstellungen → Datenschutz & Sicherheit → „Dennoch öffnen“, oder
`xattr -dr com.apple.quarantine /Applications/MasterSim.app`).

### Optionen von `create_dmg.sh`

| Option | Bedeutung |
|---|---|
| `--skip-build` | vorhandene Binaries aus `bin/release` verwenden (müssen ein *deployment*-Build sein, wird geprüft) |
| `--skip-test` | Regressionstests nach dem Build überspringen |
| `--sign "<Identity>"` | mit Developer-ID-Zertifikat und Hardened Runtime signieren (alternativ `MSIM_SIGN_IDENTITY`) |
| `--notarize <Profil>` | dmg notarisieren und Ticket anheften (alternativ `MSIM_NOTARY_PROFILE`) |

Weitere Umgebungsvariablen: `AQT_QT_PREFIX` / `AQT_QT_VERSION` (Qt-Kit), `MSIM_BUNDLE_ID`
(Standard `io.github.ghorwin.MasterSim`).

## Ablauf im Detail

1. **Build** – `build/build.sh release deployment` baut Solver, GUI und Test-FMUs als Universal Binary.
   Der Schalter `deployment` setzt `IBK_DEPLOYMENT`; erst dann sucht die GUI Übersetzungen und Beispiele
   in `MasterSim.app/Contents/Resources` statt im Quelltextbaum. Anschließend laufen die Regressionstests.
   Hinweis: Der Deployment-Build überschreibt `bin/release`. Für die Entwicklung danach wieder normal
   `build/build.sh` aufrufen.
2. **Bundle zusammenstellen** – `mastersim-gui.app` wird als `MasterSim.app` nach `dist/MasterSim-<version>/`
   kopiert, der Solver nach `Contents/MacOS/`, dazu `Info.plist` (aus `Info.plist.in`, Version aus
   `MSIM_Constants.cpp`, Build-Nummer = Anzahl Git-Commits), Übersetzungen und Beispiele.
   Die FMUs in `data/examples/darwin64` sind alte x86_64-only-Builds; im Bundle werden sie durch die
   gleichnamigen, beim Build erzeugten Universal-FMUs aus `data/tests/darwin64` ersetzt (sonst laufen
   die Beispiele auf Apple Silicon nur unter Rosetta).
3. **macdeployqt** – kopiert Qt-Frameworks und -Plugins ins Bundle und setzt die Install-Names um.
   Danach entfernt das Skript alle absoluten `LC_RPATH`-Einträge (Build-Verzeichnis, Qt-Installation) und
   prüft, dass kein Binary mehr auf Pfade der Build-Maschine verweist.
4. **Signieren** – von innen nach außen (Plugins, Frameworks, Solver, App); `codesign --deep` wird zum
   Signieren nicht mehr verwendet (von Apple abgekündigt). Mit Developer ID: Hardened Runtime +
   Zeitstempel. Solver und App erhalten `entitlements.plist` mit
   `com.apple.security.cs.disable-library-validation`, weil FMU-Bibliotheken
   (`binaries/darwin64/*.dylib`) fremd- oder unsigniert sind und sonst nicht per `dlopen()` geladen
   werden könnten.
5. **dmg** – `hdiutil create -fs APFS -format ULMO` (lzma, ab macOS 10.15 lesbar). Bei Developer ID wird
   auch das dmg signiert.
6. **Notarisierung** (optional) – `xcrun notarytool submit --wait`, `xcrun stapler staple`, Prüfung mit
   `spctl`. Danach startet die App auf jedem Mac ohne Warnung, auch offline.

## Einmalige Einrichtung für signierte/notarisierte Releases

1. Mitgliedschaft im Apple Developer Program; im Developer-Portal ein Zertifikat
   **„Developer ID Application“** erzeugen und in den Schlüsselbund importieren. Kontrolle:
   ```bash
   security find-identity -v -p codesigning
   ```
2. App-spezifisches Passwort unter <https://account.apple.com> anlegen und ein notarytool-Profil im
   Schlüsselbund speichern (Team-ID siehe Developer-Portal):
   ```bash
   xcrun notarytool store-credentials mastersim-notary --apple-id "<apple-id>" --team-id "<TEAMID>"
   ```
3. Release bauen:
   ```bash
   releases/mac/create_dmg.sh --sign "Developer ID Application: <Name> (<TEAMID>)" --notarize mastersim-notary
   ```
   Schlägt die Notarisierung fehl, liefert `xcrun notarytool log <submission-id> --keychain-profile mastersim-notary`
   die Details (meist fehlender Zeitstempel/Hardened Runtime bei einer Datei im Bundle).

## Release-Checkliste

1. `LONG_VERSION` in `MasterSim/src/MSIM_Constants.cpp` erhöhen, Changelog pflegen.
2. Übersetzungen aktualisieren (`MasterSimulatorUI/resources/translations/release_tr.sh`) und
   Handbuch-PDFs in `doc/manual` neu erzeugen.
3. `releases/mac/create_dmg.sh --sign ... --notarize ...` – Tests müssen grün sein.
4. dmg auf einem *anderen* Mac (idealerweise einmal Intel, einmal Apple Silicon) testen:
   - App per Drag & Drop installieren, starten, ein Beispiel vom Willkommensbildschirm öffnen und simulieren.
   - Sprache auf Deutsch umstellen (Übersetzung wird geladen?).
   - `/Applications/MasterSim.app/Contents/MacOS/mastersim --help` im Terminal.
5. Prüfen des Bundles (ohne zweiten Mac):
   ```bash
   codesign -dv --verbose=4 releases/mac/dist/MasterSim-*/MasterSim.app
   spctl --assess --type execute -vv releases/mac/dist/MasterSim-*/MasterSim.app
   ```
   `spctl` meldet „accepted / Notarized Developer ID“ nur bei signierten + notarisierten Builds.
6. dmg als Release-Asset auf GitHub hochladen, Git-Tag setzen.

## Dateien in diesem Verzeichnis

| Datei | Zweck |
|---|---|
| `create_dmg.sh` | Hauptskript (Build → Bundle → Signatur → dmg → Notarisierung) |
| `Info.plist.in` | Vorlage für `Contents/Info.plist` (Platzhalter `@VERSION@`, `@BUNDLE_ID@`, ...) |
| `entitlements.plist` | Hardened-Runtime-Ausnahmen (Laden fremder FMU-Bibliotheken) |
| `ReadMe-dmg.txt` | Kurzanleitung für Anwender, landet als `ReadMe.txt` im dmg |
| `*.webloc` | Links zum Online-Handbuch, landen im dmg |

## Mögliche Erweiterungen

- Hübsches dmg-Fenster (Hintergrundbild, Icon-Positionen) mit
  [`create-dmg`](https://github.com/create-dmg/create-dmg) oder `dmgbuild` statt `hdiutil`.
- Dateizuordnung für `.msim` (`CFBundleDocumentTypes` in `Info.plist.in`) – erfordert, dass die GUI
  `QFileOpenEvent` auswertet, was sie derzeit nicht tut.
- Automatischer Build in GitHub Actions (`macos-latest`, Qt per `aqtinstall`, Zertifikat als
  verschlüsseltes Secret, `notarytool` mit App-Store-Connect-API-Key statt Apple-ID).
