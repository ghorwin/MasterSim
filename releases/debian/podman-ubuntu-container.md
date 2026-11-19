# Ubuntu-Container auf Fedora mit podman/docker

## Prinzip

Ein Container emuliert nichts und ist keine VM. Er nutzt den Kernel des Hosts (Fedora) und bringt nur das *Userland* von Ubuntu mit: `apt`, `glibc`, Compiler, Qt-Pakete, `dpkg`, `lintian` usw. Was man darin baut, wird also gegen echte Ubuntu-Bibliotheken gebaut. Für den Paketbau ist das ideal.

Unterschiede zu einer VM betreffen nur Kernel-nahe Dinge, z.B. Kernelmodule oder systemd als PID 1.

`podman` ist auf Fedora vorinstalliert. Es läuft *rootless* (ohne root-Rechte und ohne Daemon) und versteht dieselben Befehle wie `docker`.

## 1. Interaktiv ein Ubuntu starten

```bash
podman run -it --rm docker.io/library/ubuntu:24.04 bash

# im Container:
cat /etc/os-release            # -> Ubuntu 24.04 (Noble)
apt-get update && apt-get install -y build-essential devscripts lintian
```

- Die Ubuntu-Version wählt man über den Tag: `ubuntu:22.04`, `ubuntu:24.04`, `ubuntu:26.04` oder per Codename `ubuntu:noble`.
- `--rm` löscht den Container beim Beenden, installierte Pakete gehen also verloren. Das heruntergeladene Image bleibt im lokalen Cache.

## 2. Host-Verzeichnis in den Container einbinden

```bash
podman run -it --rm -v "$PWD":/work:Z -w /work docker.io/library/ubuntu:24.04 bash
```

- `/work` im Container ist direkt das aktuelle Verzeichnis des Hosts. Gebaute Dateien landen also sofort auf dem Host.
- `:Z` ist auf Fedora nötig, sonst blockiert SELinux den Zugriff mit „Permission denied“.

## 3. Eigenes Image mit vorinstallierten Build-Abhängigkeiten

Damit nicht bei jedem Build alle Pakete neu installiert werden müssen, baut man ein eigenes Image. Die Datei heißt `Containerfile` und hat dieselbe Syntax wie ein `Dockerfile`:

```dockerfile
FROM docker.io/library/ubuntu:24.04
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential devscripts equivs lintian fakeroot
COPY control /tmp/control
RUN cd /tmp && mk-build-deps -i -r -t "apt-get -y --no-install-recommends" /tmp/control
```

`mk-build-deps` (aus dem Paket `equivs`) liest die `Build-Depends` aus `debian/control` und installiert sie.

```bash
# Image bauen (Kontextverzeichnis = debian/, enthält control)
podman build -t mastersim-deb:noble -f Containerfile debian/

# Paket im Container bauen
podman run --rm -v "$PWD":/work:Z -w /work/mastersim-0.9.8 \
    mastersim-deb:noble dpkg-buildpackage -b -us -uc
```

Das Image wird einmal gebaut und danach wiederverwendet. podman baut es nur dann neu, wenn sich das `Containerfile` oder `control` ändert (Layer-Cache).

## 4. Nützliche Befehle

| Befehl | Zweck |
|---|---|
| `podman images` | lokale Images anzeigen |
| `podman ps -a` | Container anzeigen (auch beendete) |
| `podman rmi <image>` | Image löschen |
| `podman system prune` | ungenutzte Container/Images aufräumen |
| `podman run --name x …` (ohne `--rm`) | Container bleibt erhalten … |
| `podman start -ai x` | … und kann später wieder betreten werden |

## 5. Stolpersteine auf Fedora

- **SELinux:** Volumes immer mit `:Z` (nur dieser Container) oder `:z` (von mehreren Containern genutzt) einbinden.
- **Benutzer-IDs:** Rootless entspricht „root“ im Container der eigenen UID auf dem Host. Dateien, die im Container entstehen, gehören auf dem Host also dem eigenen Benutzer. Mit `--userns=keep-id` arbeitet man im Container direkt unter der eigenen UID.
- **Warnung „The cgroupv2 manager is set to systemd but there is no systemd user session available“:** Sie tritt auf, wenn keine (vollständige) systemd-User-Session läuft, z.B. nach `su` oder `sudo -u`. `podman run` funktioniert oft trotzdem. `podman build` scheitert dann aber bei `RUN`-Schritten mit `sd-bus call: Access denied … interactive authentication`, selbst wenn `/run/user/<uid>/bus` existiert. Abhilfe: `podman --cgroup-manager=cgroupfs build …`. Das globale Flag muss *vor* dem Unterbefehl stehen. `release.sh` setzt es immer.
- **Fremde Architekturen** (z.B. arm64 auf einem x86-Rechner) bräuchten echte Emulation über `qemu-user-static` und `--arch arm64`. Das ist langsam und für amd64-Pakete unnötig.

## 6. Verwendung im MasterSim-Release

Das Skript `release.sh` (Logik in `scripts/msimdeb/container.py`) nutzt genau diesen Ansatz:

- Pro Ubuntu-Serie aus `series.json` gibt es ein Image (`mastersim-deb:noble`, `mastersim-deb:resolute`). Es enthält die Paketierwerkzeuge und alle Build-Abhängigkeiten aus `debian/control`. Das ersetzt die bisherigen VirtualBox-VMs.
- `dpkg-buildpackage` und `lintian` laufen im Container, das Arbeitsverzeichnis `work/<serie>/…` ist eingebunden.
- GPG-Signatur und Upload zu Launchpad laufen auf dem Host. Der private Schlüssel kommt also nie in einen Container.
- Mit `CONTAINER_ENGINE=docker ./release.sh …` kann man statt podman auch docker verwenden.
