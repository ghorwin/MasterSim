This directory holds the release versions of third-party libraries.

In order to keep the subversion repository small, please refrain from
updating these binary files too often.

The extracted source code of the libraries needs to be placed in
the *externals* subdirectory. See readme file in this directory.

Some of the libraries are only source for code portions, that were used
in the MasterSim library.


Notes on zLib 1.2.8 compilation on Windows
------------------------------------------

* see http://www.tannerhelland.com/5076/compile-zlib-winapi-wapi-stdcall/

Procedure in brief:


1.    Extract the entire zLib file and navigate to the /contrib/masmx86 folder. Open the “bld_ml32.bat” file in a text editor.
2.    Add the “/safeseh” switch to both lines in that file (e.g. “ml /safeseh /coff /Zi /c /Flmatch686.lst match686.asm”). Then save and exit.
3.    Navigate to the /contrib/vstudio/vc11/ folder. Open the zlibvc.sln file in your newly installed Visual Studio 2012 Express.
4.    In the Solution Explorer (top-right by default), right-click “zlibstat” then select “Properties” at the bottom.
5.    Go to Configuration Properties -> C/C++ -> Preprocessor, and in the Preprocessor Definitions line remove “ZLIB_WINAPI;” (don’t forget to remove the trailing semicolon).
6.    Now, we need to fix a recently introduced problem that relies on Win8 functionality. In the Solution Explorer, navigate to zlibvc -> iowin32.c. Double-click to open the file.
7.    Find the line of text that reads “#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_APP)”. Change this line to “#if WINAPI_FAMILY_ONE_PARTITION(WINAPI_FAMILY_DESKTOP_APP, WINAPI_PARTITION_APP)”. (Thanks to this link for this fix.)
8.    zLib uses a Version number declaration that can cause the build process to fail. To fix this, go back to the Solution Explorer, then navigate to zlibvc -> zlibvc.def. Double-click to open.
9.    Change the line that reads “VERSION 1.2.8” to read “VERSION 1.28”.
10.   Finally, go to the Build -> Configuration Manager menu and change the Active Solution Configuration to “Release”.
11.   Exit that window and press F7 (or click the Build -> Build Solution menu). The project should successfully build.
12.   You can find your newly compiled zlibwapi.dll file in the /contrib/vstudio/vc11/x86/ZlibDllRelease/ folder.
