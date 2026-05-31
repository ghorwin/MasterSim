# Release procedure

## Notizen zum Veröffentlichen auf Launchpad

- ggfs. muss das Ablaufdatum für den gpg Key aktualisiert werden:

```
gpg --edit-key andreas.nicolai@gmx.net

$ expire
...
$quit
```

- dann hochladen

```
gpg --send-keys --keyserver keyserver.ubuntu.com 34FC6FB934502913B4C1DCA86E0814BD3FCA8338
```

Beim Veröffentlichen mehrerer Ubuntu-Releases muss der Source-Tarball übereinstimmen.
D.h. zuerst den Quelltext in ALLEN VMs testbauen. Dann in einer VM veröffentlichen und
den Source-Tarball: mastersim_0.9.7.orig.tar.xz in alle anderen VMs kopieren

1. in der jeweiligen Release-VM das Script `scripts/update_source_code_archive.sh` ausführen,
 es wird das MasterSim-repo geklont, die aktuelle Versionsnummer (bspw. 0.9.7) ausgelesen und 
 die Datei mastersim_0.9.7.orig.tar.xz erstellt
2. Das Archiv mit dem bereits veröffentlichten Quelltextarchiv überschreiben
3. im jeweiligen ubuntu-xx Verzeichnis das Release-Prozedere durchlaufen




## Preparations

- Increase version number and release date in `MasterSim/src/MSIM_Constants.cpp`
- Update man-page:

[source,bash]
----
./MasterSimulator --man-page > ../../MasterSimulator/doc/MasterSimulator.1
----

- update manual


## First deb package for a new release/new version

When creating new release for first Debian/Ubuntu distro:

```bash
> scripts/update_source_code_archive.sh
```

Then change into distro-specific directory and run

```bash
> dch
```

to create new changelog entry. Edit the top line to reflect the new version number, e.g.

```
mastersim (0.9.5-1~jammy~ppa1) UNRELEASED; urgency=medium

  * update for release 0.9.5

 -- Andreas Nicolai <andreas.nicolai@gmx.net>  Thu, 22 Dec 2022 18:50:35 +0100

mastersim (0.9.4-1~jammy~ppa1) jammy; urgency=medium

  * first release for Jammy

 -- Andreas Nicolai <andreas.nicolai@gmx.net>  Thu, 19 May 2022 16:15:10 +0200

```

Note that the top line contains the tag UNRELEASED. Also make sure, that the VERSION NUMBER is updated!

Next run

```bash
> dch --release
```

Now the line will read:

```
mastersim (0.9.5-1~jammy~ppa1) jammy; urgency=medium
```

Then:

```bash
> ../scripts/newUpstreamVersion.sh
Building source package 0.9.5-1~jammy~ppa1 for upstream version 0.9.5
Was the changelog updated correctly? Press any key or CTRL+C to abort!

(press key to continue)

....
Upload source archive to Launchpad? Remember to build binary package at least once for testing!

(press CTRL+C to aboard upload!!!)
```

and fix any warnings potentially issued.

Next build the package:

```bash
> ../scripts/buildBinary.sh
```

and if all is successful, again by:

```bash
> ../scripts/newUpstreamVersion.sh
# this time just press a key to start upload
```

## deb packages for other releases

Download already published original source code archive from launchpad, and extract:

```bash
> tar -xf mastersim_0.9.3.orig.tar.xz
```

Then follow procedure as described above.



