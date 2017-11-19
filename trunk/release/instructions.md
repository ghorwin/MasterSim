Release/Deployment Instruction
==============================

This subdirectory holds scripts to generate releases/deploy MasterSimulator on different platforms.
To create a release, change into the OS subdirectory and execute the `create_release.*` scripts.

For Windows you need InnoSetup installed and have to copy third-party libraries for Qt and VisualStudio
into the subdirectory VC14.

When uploading to SourceForge, rename the `release_notes.md` file to `readme.me` and place it into the `releases` directory.


After uploading new releases to SourceForge, also update the news.html on bauklimatik-dresden.de for infos on
the Welcome Page.

