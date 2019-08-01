#include "MSIMPostProcBindings.h"

#include <QDir>

#include <fstream>

#include <MSIM_Project.h>

#include <IBK_StringUtils.h>
#include <IBK_QuantityManager.h>
#include <IBK_Path.h>
#include <IBK_FormatString.h>

#include "MSIMSettings.h"
#include "MSIMProjectHandler.h"

#include <QProcess>


// Forward declarations of templates. Definitions are below at end of file
extern const char * const DEFAULT_SESSION_TEMPLATE;


IBK::Path MSIMPostProcBindings::defaultSessionFilePath(const QString & projectFile) {
	IBK::Path projectPath = IBK::Path(projectFile.toStdString());
	// if path is valid, use filename as session name, otherwise use generic name (i.e. root path selected)
	IBK::Path sessionFile = projectPath.withoutExtension() + ".p2";
	return sessionFile;
}


void MSIMPostProcBindings::generateDefaultSessionFile(const QString & projectFile) {
	IBK::Path sessionFile = defaultSessionFilePath(projectFile);

#if defined(_WIN32)
	#if defined(_MSC_VER)
		std::ofstream strm(sessionFile.wstr().c_str());
	#else
		std::string filenameAnsi = IBK::WstringToANSI(sessionFile.wstr(), false);
		std::ofstream strm(filenameAnsi.c_str());
	#endif
#else
	std::ofstream strm(sessionFile.c_str());
#endif

	// generate directories and

	// replace placeholders
	IBK::Path projectFilePath = IBK::Path(projectFile.toStdString() );
	IBK::Path subdir = projectFilePath.filename().withoutExtension();
	std::string content = DEFAULT_SESSION_TEMPLATE;
	content = IBK::replace_string(content, "${PROJECT_RESULT_BASE_DIR}", subdir.str());
	strm << content << std::endl;
}



// *** MSIMPostProcHandler ***


#ifdef _WIN32

struct handle_data {
	unsigned long process_id;
	HWND window_handle;
};


BOOL is_main_window(HWND handle) {
	return GetWindow(handle, GW_OWNER) == (HWND)0 && IsWindowVisible(handle);
}


BOOL CALLBACK enum_windows_callback(HWND handle, LPARAM lParam) {
	handle_data& data = *(handle_data*)lParam;
	unsigned long process_id = 0;
	GetWindowThreadProcessId(handle, &process_id);
	if (data.process_id != process_id || !is_main_window(handle))
		return TRUE;
	data.window_handle = handle;
	return FALSE;
}


HWND find_main_window(unsigned long process_id) {
	handle_data data;
	data.process_id = process_id;
	data.window_handle = 0;
	EnumWindows(enum_windows_callback, (LPARAM)&data);
	return data.window_handle;
}

#else
	#include <sys/types.h>
	#include <signal.h>
#endif // _WIN32


MSIMPostProcHandler::MSIMPostProcHandler() :
	m_postProcHandle(0)
{

}


int MSIMPostProcHandler::reopenIfActive() {
#if _WIN32
	if (m_postProcHandle != 0) {
		DWORD exitCode;
		BOOL res = GetExitCodeProcess(m_postProcHandle, &exitCode);
		if (!res) {
			m_postProcHandle = 0;
			return 1;
		}
		if (exitCode == STILL_ACTIVE) {
			DWORD postProcId = GetProcessId(m_postProcHandle);
			HWND postProcWinHandle = find_main_window(postProcId);
			BringWindowToTop(postProcWinHandle);
			return 0;
		}
	}
#else
	// send 0 kill signal and see if the error code tells is, that the process is still there
	if (m_postProcHandle != 0) {
#if defined(Q_OS_MAC)
//		NSRunningApplication * pp_app = runningApplicationWithProcessIdentifier(m_postProcHandle);
#else
		int killres = kill(m_postProcHandle, 0);
		if (killres == 0) {
			// process lives!
			// TODO : raise window to front
			return 0;
		}
		else {
			m_postProcHandle = 0;
			return 1;
		}
#endif
	}
#endif
	return 1;
}


bool MSIMPostProcHandler::spawnPostProc(const std::string & sessionFile) {
#if _WIN32
	IBK::FormatString cmdLine;
	if (sessionFile.empty()) {
		cmdLine = IBK::FormatString("\"%1\"")
				.arg(MSIMSettings::instance().m_postProcExecutable.toStdString());
	}
	else {
		cmdLine = IBK::FormatString("\"%1\" \"%2\"")
		.arg(MSIMSettings::instance().m_postProcExecutable.toStdString())
		.arg(sessionFile);
	}

	// Use WinAPI to create a PostProc process
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory( &si, sizeof(si) );
	si.cb = sizeof(si);
	ZeroMemory( &pi, sizeof(pi) );
	const unsigned int lower_priority = 0x00004000;

	std::string cmd = cmdLine.str();
	// Start the child process.
	if( !CreateProcess( NULL,	// No module name (use command line).
		&cmd[0],				// Command line.
		NULL,					// Process handle not inheritable.
		NULL,					// Thread handle not inheritable.
		FALSE,					// Set handle inheritance to FALSE.
		lower_priority,			// Create with priority lower then normal.
		NULL,					// Use parent's environment block.
		NULL,					// Use parent's starting directory.
		&si,					// Pointer to STARTUPINFO structure.
		&pi )					// Pointer to PROCESS_INFORMATION structure.
	)
	{
		return false;
	}
	// store process handle for later
	m_postProcHandle = pi.hProcess;
#else
	// spawn post-proc process
	QProcess p;
	p.setProgram(MSIMSettings::instance().m_postProcExecutable);
	QStringList args;
	if (!sessionFile.empty())
		args += QString::fromStdString(sessionFile);
	bool res = p.startDetached(MSIMSettings::instance().m_postProcExecutable, args, QString(), &m_postProcHandle);
//		if (!res) {
//			QMessageBox::critical(this, tr("Error starting external application"), tr("Post-processing '%1' could not be started.")
//				.arg(DelSettings::instance().m_postProcExecutable));
//		}
//		return false;
	return res;
#endif
	return true;
}



const char * const DEFAULT_SESSION_TEMPLATE =
		"<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"
		"<PostProc version=\"2.1.1\">\n"
		"	<!--List of all directories-->\n"
		"	<Directories>\n"
		"		<Directory>\n"
		"			<Path>.</Path>\n"
		"			<SubDir Color=\"#416fce\" Checked=\"1\">.</SubDir>\n"
		"			<SubDir Color=\"#ffaa00\" Checked=\"1\">${PROJECT_RESULT_BASE_DIR}</SubDir>\n"
		"			<ExpandedSubDir>.</ExpandedSubDir>\n"
		"		</Directory>\n"
		"	</Directories>\n"
		"\n"
		"	<!--~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~-->\n"
		"\n"
		"	<!--Split window state.-->\n"
		"	<Splitting>\n"
		"		<Widget id=\"1\" parent=\"0\" viewTable=\"0\" mapperIndex=\"-1\" size1=\"1\" size2=\"1\" orientation=\"2\" />\n"
		"		<FocusedWidget>0</FocusedWidget>\n"
		"	</Splitting>\n"
		"\n"
		"	<!--~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~-->\n"
		"\n"
		"</PostProc>\n";
