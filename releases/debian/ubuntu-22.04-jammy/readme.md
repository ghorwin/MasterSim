# Software Release Info


## Deployment System 

Setup based on VirtualBox

### Basis-System

- Virtual Box, 22.04 Desktop Image, Minimal System
- nach Installation:

```bash
sudo apt install build-essential 
sudo apt update && sudo apt upgrade -y
```

- neu booten, VBox Extensions einlegen und installieren
- neu booten

### Pakete für Debian Package Deployment

```bash
sudo apt install cmake git debhelper lintian devscripts zlib1g-dev qtbase5-dev libqt5core5a libqt5concurrent5 libqt5gui5 libqt5network5 libqt5svg5-dev libqt5xml5 libqt5widgets5 p7zip-full
```

### Einrichten für Launchpad (Keys)

in .profile oder .bashrc:

```bash
export DEBFULLNAME="Andreas Nicolai"
export DEBEMAIL="andreas.nicolai@gmx.net"
```

gpg-Schlüssel export-Datei `private.key` auf VM kopieren und importieren:

```bash
gpg --import private.key
```

### Repo Clonen und Release bauen

ssh key for ssh/git server auf VM kopieren

```bash
~/.ssh/id_rsa 
~/.ssh/id_rsa.pub
```

mastersim-private repo clonen.

## Steps to update a package

Update version info in changelog by running

```bash
dch
```

```bash
$ ../scripts/newUpstreamVersion.sh
```

When prompted to upload source package, stop on first run with CTRL+C.

Then run

```bash
$ ../scripts/buildBinary.sh
```

and check compilation.

Afterwards re-run 

```bash
$ ../scripts/newUpstreamVersion.sh
```

and this time upload the files.

