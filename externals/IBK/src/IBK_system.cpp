/*	Copyright (c) 2001-2017, Institut für Bauklimatik, TU Dresden, Germany

	Written by A. Nicolai, H. Fechner, St. Vogelsang, A. Paepcke, J. Grunewald
	All rights reserved.

	This file is part of the IBK Library.

	Redistribution and use in source and binary forms, with or without modification,
	are permitted provided that the following conditions are met:

	1. Redistributions of source code must retain the above copyright notice, this
	   list of conditions and the following disclaimer.

	2. Redistributions in binary form must reproduce the above copyright notice,
	   this list of conditions and the following disclaimer in the documentation
	   and/or other materials provided with the distribution.

	3. Neither the name of the copyright holder nor the names of its contributors
	   may be used to endorse or promote products derived from this software without
	   specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
	ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
	DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
	ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
	(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
	LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
	ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
	SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


	This library contains derivative work based on other open-source libraries.
	See OTHER_LICENCES and source code headers for details.

*/

#include "IBK_system.h"

#include "IBK_Path.h"

#if defined(_WIN32)
	#include <windows.h>
    #include <shellapi.h>
#else
	#include <sys/statvfs.h>
#endif

namespace IBK {



unsigned int getFreeDiskSpace(const IBK::Path& dir) {

#if defined(_WIN32)
	DWORD sectorsPerCluster;
	DWORD bytesPerSector;
	DWORD freeClusters;
	DWORD totalClusters;

	std::string drive = dir.drive();
	drive += "\\";

	//get disk space for current drive
	bool success = (GetDiskFreeSpaceA(
		drive.c_str(), //current drive
		&sectorsPerCluster, //sectors per cluster
		&bytesPerSector, //bytes per sector
		&freeClusters, //free clusters
		&totalClusters //total clusters
	) == 1);

	if(!success) {
		return 0;
	}

	unsigned int kBPerCluster = bytesPerSector * sectorsPerCluster / 1024;

	return kBPerCluster * freeClusters / 1024;

#else


	struct statvfs	fs;
	int				returnCode = 0;

	returnCode = statvfs( dir.absolutePath().c_str(), &fs );

	if (returnCode == 0)
		return fs.f_bsize * fs.f_bfree / 1024;

	/// \todo error code handling
	return 0;

#endif
}

namespace {

#ifdef _WIN32
std::string wideToUtf8(const wchar_t *wstr) {
    if (wstr == nullptr)
        return std::string();

    const int size = WideCharToMultiByte(
        CP_UTF8, 0,
        wstr, -1,
        nullptr, 0,
        nullptr, nullptr
        );

    if (size <= 0)
        return std::string();

    std::string result(static_cast<size_t>(size - 1), '\0');

    WideCharToMultiByte(
        CP_UTF8, 0,
        wstr, -1,
        &result[0], size,
        nullptr, nullptr
        );

    return result;
}
#endif

} // anonymer Namespace

std::vector<std::string> argumentsUtf8(int argc, char *argv[]) {

    std::vector<std::string> args;

#ifdef _WIN32
    int wargc = 0;
    wchar_t **wargv = CommandLineToArgvW(GetCommandLineW(), &wargc);

    if (wargv == nullptr)
        return args;

    args.reserve(static_cast<size_t>(wargc));

    for (int i = 0; i < wargc; ++i)
        args.push_back(wideToUtf8(wargv[i]));

    LocalFree(wargv);

#else
    args.reserve(static_cast<size_t>(argc));

    for (int i = 0; i < argc; ++i)
        args.push_back(std::string(argv[i]));
#endif

    return args;
}


} // namespace IBK

