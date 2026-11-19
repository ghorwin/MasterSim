# Debian/Ubuntu-Pakete für MasterSim (Launchpad PPA)

Die Pakete werden im PPA [`ppa:ghorwin/sim`](https://launchpad.net/~ghorwin/+archive/ubuntu/sim) veröffentlicht. Launchpad baut die Binärpakete selbst, hochgeladen werden nur **signierte Source-Pakete**.

## Überblick

```
releases/debian/
  debian/                      Paketierungs-Vorlage, gemeinsam für alle Ubuntu-Serien
  series.json                  Ubuntu-Serien, für die gebaut wird: {"codename": "version"}
  release.sh                   Start-Wrapper  ->  scripts/release.py
  scripts/release.py           Kommandozeile (Schritte, Optionen)
  scripts/msimdeb/             Python-Logik (nur Standardbibliothek)
    config.py                    Pfade, PPA, Tarball-Ausschlüsse
    version.py                   Versionsnummern, Changelog pro Serie
    tarball.py                   Quelltextarchiv aus git-Tag
    container.py                 Ubuntu-Container (podman/docker)
    package.py                   Binär-Testbuild und Source-Paket
    sign.py                      gpg-Signatur von .dsc/.changes (ersetzt debsign)
    upload.py                    FTP-Upload zu Launchpad (ersetzt dput)
  podman-ubuntu-container.md   Erklärung zu Ubuntu-Containern unter Fedora
  work/                        alle erzeugten Dateien (von git ignoriert)
```

Alle Debian-Werkzeuge (`dpkg-buildpackage`, `lintian`, Build-Abhängigkeiten) laufen in einem **Ubuntu-Container pro Serie**. Auf dem Host werden nur `python3`, `git`, `gpg` und `podman` (oder `docker`) gebraucht, also keine VMs und kein Debian-System. Der private gpg-Schlüssel bleibt auf dem Host.

### Versionsschema

Aus dem Changelog-Eintrag `mastersim (0.9.8-1)` in der Vorlage wird pro Serie automatisch:

```
mastersim (0.9.8-1~ubuntu24.04~ppa1) noble; urgency=medium
mastersim (0.9.8-1~ubuntu26.04~ppa1) resolute; urgency=medium
```

Die Endung `~ppa1` ist die PPA-Revision (`--ppa-rev`). Sie wird nur erhöht, wenn dieselbe Debian-Version erneut hochgeladen werden muss.

`~ubuntuXX.YY` sortiert, anders als Codenamen, auch über den alphabetischen Umbruch der Codenamen hinweg richtig. Ältere Uploads im Format `0.9.7-1~noble~ppa1` sind kleiner als `0.9.8-…`, Upgrades funktionieren also.

### Das Quelltextarchiv

`mastersim_<ver>.orig.tar.xz` wird per `git archive` aus dem Tag `v<ver>` erzeugt. Es enthält keine Dokumentationsquellen, Testdaten, `third-party/`, `externals/zlib/` und keine vorkompilierten Binärdateien (`.fmu`, `.so`, `.dll`, `.pdf` …). Die Liste steht in `scripts/msimdeb/config.py`.

Das Archiv ist **reproduzierbar**: Derselbe Tag ergibt immer ein byte-identisches Archiv. Das ist wichtig, weil alle Serien im PPA denselben orig-Tarball teilen. Launchpad lehnt einen Upload ab, wenn sich ein bereits hochgeladener Tarball unterscheidet. Das Skript überschreibt deshalb ein vorhandenes, abweichendes Archiv nur mit `--force`.

Das Define `IBK_BUILDING_DEBIAN_PACKAGE` wird über `debian/rules` gesetzt. Der Quelltext im Archiv bleibt unverändert.

## Einmalige Einrichtung

```bash
sudo dnf install podman python3 git gnupg2     # Fedora (auf Ubuntu: apt install podman …)
```

Den gpg-Schlüssel, der bei Launchpad registriert ist, im Schlüsselbund des Hosts haben. Bei Bedarf das Ablaufdatum verlängern und den Schlüssel neu hochladen:

```bash
gpg --edit-key andreas.nicolai@gmx.net      # dann: expire … save
gpg --send-keys --keyserver keyserver.ubuntu.com 34FC6FB934502913B4C1DCA86E0814BD3FCA8338
```

Optional, falls nicht der Standard-Schlüssel benutzt werden soll: `export DEBSIGN_KEYID=34FC6FB9…`

## Release-Ablauf

### 1. Vorbereitung im Quelltext

- Versionsnummer `LONG_VERSION` in `MasterSim/src/MSIM_Constants.cpp` erhöhen.
- Manpages aktualisieren (`MasterSimulator/doc/mastersim.1`, `MasterSimulatorUI/doc/mastersim-gui.1`).
- Committen und den Release-Tag setzen: `git tag v0.9.8 && git push --tags`

### 2. Changelog-Eintrag

Oben in `debian/changelog` (in diesem Verzeichnis) einen Eintrag für die neue Version hinzufügen. Distribution und Datum ersetzt das Skript pro Serie automatisch:

```
mastersim (0.9.8-1) UNRELEASED; urgency=medium

  * Release 0.9.8

 -- Andreas Nicolai <andreas.nicolai@gmx.net>  Thu, 19 Nov 2026 21:00:00 +0100
```

Bei reinen Paketierungs-Korrekturen ohne neue Upstream-Version wird die Debian-Revision erhöht (`0.9.8-2`).

### 3. Bauen, prüfen, hochladen

```bash
cd releases/debian

./release.sh tarball     # work/mastersim_0.9.8.orig.tar.xz aus Tag v0.9.8
./release.sh build       # Binär-Testbuild + lintian für alle Serien (Container)
./release.sh source      # Source-Pakete + lintian
./release.sh sign        # gpg-Signatur (fragt nach der Passphrase)
./release.sh upload      # fragt nach [y/N], dann FTP-Upload zu Launchpad
```

oder alles in einem Durchgang: `./release.sh all`

Danach schickt Launchpad eine Mail (accepted/rejected). Der Baufortschritt ist auf der PPA-Seite zu sehen.

### Nützliche Optionen

| Option | Bedeutung |
|---|---|
| `--series noble` | nur bestimmte Serien (kommagetrennt) |
| `--ref HEAD` | Tarball aus einem beliebigen git-Stand statt aus dem Tag, nur für Testbuilds |
| `--ref worktree` | Tarball inkl. noch nicht committeter Änderungen an getrackten Dateien, nur für Testbuilds |
| `--ppa-rev 2` | erneuter Upload derselben Debian-Version (`…~ubuntu24.04~ppa2`) |
| `--key <id>` | gpg-Schlüssel für die Signatur |
| `--dry-run` | Upload: nur die Dateien auflisten |
| `--force` | vorhandenes, abweichendes orig-Archiv überschreiben |

`CONTAINER_ENGINE=docker ./release.sh …` verwendet docker statt podman.

## Neue Ubuntu-Serie hinzufügen / alte entfernen

Eintrag in `series.json` ergänzen bzw. löschen, z.B. `"questing": "25.10"`. Das Container-Image `docker.io/library/ubuntu:<version>` muss existieren. Für EOL-Serien nimmt Launchpad keine Uploads mehr an.

## Fehlersuche

- **Build-Abhängigkeiten:** Sie stehen nur in `debian/control`. Das Container-Image wird automatisch neu gebaut, wenn sich die Datei ändert.
- **In den Build-Container schauen:**
  `podman run -it --rm -v $PWD/work/noble/binary:/work:Z mastersim-deb:noble bash`
- **Images aufräumen:** `podman rmi mastersim-deb:noble mastersim-deb:resolute`
- **cgroup-/sd-bus-Fehler bei podman** (`sd-bus call: Access denied`, z.B. nach `su`): Das Skript ruft podman deshalb immer mit `--cgroup-manager=cgroupfs` auf. Siehe auch `podman-ubuntu-container.md`.
