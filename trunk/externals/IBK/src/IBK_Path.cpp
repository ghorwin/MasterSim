#include "IBK_Path.h"

#if defined(_WIN32)
	#include <Windows.h>
	#include <direct.h>
	#include <sys/types.h>
#else
	#include <unistd.h>
	#include <utime.h>
	#include <errno.h>
	#include <dirent.h>
#endif

#include <sys/stat.h>

#include <locale>
#include <ctime>
#include <cctype>
#include <algorithm>
#include <cerrno>
#include <cstdio>
#include <fstream>
#include <string>


#include "IBK_messages.h"
#include "IBK_assert.h"
//#include "utf8/utf8.h"


namespace IBK {


#if defined(_WIN32)
/*! Class encapsulates a windows file handler.
*/
class FileHandlerWrapper {
public:
	FileHandlerWrapper(const std::string& filename, bool write = false) :
		m_handle(0),
		m_valid(false),
		m_error(0)
	{
		try {
			std::wstring wfilename = UTF8ToWstring(filename);
			DWORD desiredAccess = write ? FILE_WRITE_ATTRIBUTES : 0;
			DWORD shareMode = write ? FILE_SHARE_WRITE : FILE_SHARE_READ;
			m_handle = CreateFileW(wfilename.c_str(), desiredAccess, shareMode, NULL,
								OPEN_EXISTING, 0, NULL);
			m_valid = m_handle != INVALID_HANDLE_VALUE;
		}
		catch(...) {
			m_valid = false;
		}
		if( !m_valid)
			m_error = GetLastError();
	}
	~FileHandlerWrapper() {
		CloseHandle(m_handle);
	}
	HANDLE handle() const { return m_handle; }
	bool   isValid() const { return m_valid; }
	DWORD  error() const { return m_error; }

private:
	HANDLE m_handle;
	bool m_valid;
	DWORD	m_error;
};

// helper functions in order to prevent compiler warning in function file_size
template<int S>
int64_t FileSizeReturner(LARGE_INTEGER FileSize) {
	return FileSize.LowPart;
}

// specialized function for int parameter 8
template<>
int64_t FileSizeReturner<8>(LARGE_INTEGER FileSize) {
	return FileSize.QuadPart;
}

std::string GetLastErrorMessage(int id = 0) {
	DWORD errorId = id == 0 ? GetLastError() : id;
	char* lpMsgBuf = 0;
	FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		errorId,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPSTR) &lpMsgBuf,
		0,
		NULL );
	if(lpMsgBuf != 0) {
		std::string errmsg = lpMsgBuf;
		LocalFree(lpMsgBuf);
		return errmsg.substr(0, errmsg.size() - 2); // remove EOL at end of string
	}
	return "Cannot retrieve error message with id " + IBK::val2string(errorId);
}

#endif

Path::Path()
{}


Path::Path(const std::string& path) {
	set(path);
}


Path::Path(const char * const path) {
	std::string pathStr = path;
	set(pathStr);
}


Path& Path::operator=(const std::string& path) {
	set(path);
	return *this;
}


Path& Path::operator=(const char * const path) {
	set(std::string(path));
	return *this;
}


void Path::clear() {
	m_path.clear();
}


void Path::swap(Path& src) {
	m_path.swap(src.m_path);
}


std::string Path::str() const {
	return m_path;
}

std::string Path::osStr() const {
	std::string result = m_path;
#if defined(_WIN32)
	std::replace(result.begin(), result.end(), '/', '\\');
#else
// do nothing
#endif
	return result;
}


#if defined(_WIN32)
std::wstring Path::wstr() const {
	return IBK::UTF8ToWstring(m_path);
}

std::wstring Path::wstrOS() const {
	std::string result = m_path;
	std::replace(result.begin(), result.end(), '/', '\\');
	return IBK::UTF8ToWstring(result);
}

#endif



Path Path::absolutePath() const {

	/*! Removes relative parts if possible.
		A leading . or .. will be replaced by using current directory.
	*/

	// no path exists
	if( m_path.empty())
		return *this;

	// the path is already absolute
	if( isAbsolute())
		return *this;

	// path with placeholder cannot be converted into absolut path without resolving the placeholder
	if( hasPlaceholder())
		return *this;

	Path result(m_path);
	Path currentPath = current();
	// in case of './path' remove the './'
	if( result.m_path.size() > 1 && result.m_path[0] == '.' && result.m_path[1] == '/')
		result.m_path = result.m_path.substr(2);
	currentPath = currentPath / result.m_path;

	result = currentPath;
	result.removeRelativeParts();

	return result;
}


Path Path::relativePath(const Path& otherPath, bool* success) const {

	const char * const FUNC_ID = "[Path::relativePath]";

//	Makes nothing if the path is empty.
//	\code
//	Path = "."
//	Path = "/"
//	Path = ""
//	Path = "d:"
//	relPath = ""
//	relPath = "."
//	relPath = ".."
//	relPath = "/"
//	\endcode
//	If relpath is empty, it will be assumed as / (root path) and simply return path.
//	\code
//	relative_path("bla/blub.txt", "bla"); // returns 'blub.txt'
//	relative_path("bla/blubber", "bla/blub"); // returns '../blubber'
//	\endcode


	if( success)
		*success = false;

	if( m_path.empty() )
		return *this;

	if( !otherPath.isValid() )
		return *this;

	Path absolute = absolutePath();

	std::string driveCurrent = absolute.drive();
	std::string driveRelPath = otherPath.drive();

	// cannot create relative path for windows if drives are different
	if( driveCurrent != driveRelPath)
		return Path();

	if ( !absolute.isValid() ){
		IBK::IBK_Message( "Got empty file name!", MSG_WARNING, FUNC_ID);
		return *this;
	}

	// final Path to return
	std::string finalPath;

	try {

		// std::string result = IBK::relative_path(absolute.m_path, relPath.m_path);
		// std::string relative_path(const std::string& filename, const std::string& relpath) {

		std::string fname = absolute.m_path;
		std::string name = absolute.filename().str();
		std::string fpath = absolute.parentPath().str();

		remove_trailing_slash(fpath);

		std::string path = otherPath.str();
		trim(path);
		remove_trailing_slash(path);

		// if path is empty or relpath is equal to filename, it must be the root path, so we return the filename
		if (path.empty() || path == fname)
			return absolute;

		// make a backup copy of the old string
		std::string oldfpath = fpath;
		while (string_nocase_find(path, fpath)==std::string::npos) {

			size_t pos = fpath.rfind('$');
			if (pos==std::string::npos) {

				size_t pos = fpath.rfind('/');
				if (pos!=std::string::npos)
					fpath.erase(pos);
				else
					// not possible to find relative path, probably different drives
					return absolute;
			}
			else
				// not possible to find relative path, probably different drives
				return absolute;

		};

		// compose first part of post string
		std::string postStr = oldfpath.substr(fpath.size());
		if (postStr.empty())
			postStr = name;
		else
			postStr += '/'+name;

		// now go back the path until path is equal to fpath
		std::string preStr;
		while (!string_nocase_compare(path, fpath)) {
			size_t pos = path.rfind('/');
			if (pos!=std::string::npos) {
				path.erase(pos);
				preStr+="../";
			}
			else
				// not possible to find relative path, probably different drives
				return absolute;
		}

		// strip trailing / from preStr
		if (!preStr.empty()) {
			preStr.erase(preStr.size()-1);
		}
		finalPath = preStr + postStr;

		// if we have a root path, remove the leading slash
		if (!finalPath.empty() && finalPath[0] == '/')
			finalPath = finalPath.substr(1,std::string::npos);
	}

	/// \todo check which routine emit exceptions here, if non remove this code
	catch(...) {
		return *this;
	}

	if( success)
		*success = true;

	return Path(finalPath);

}


std::string Path::drive() const {

	if( m_path.size() < 2 || m_path[1] != ':')
		return "";

	std::string newDrive = m_path.substr(0,1) + ":";
	return newDrive;

}


Path Path::parentPath() const {

	std::string::size_type pos;
	std::string resPath = m_path;

	if (!isValid())
		throw IBK::Exception(IBK::FormatString("Invalid path: '%1'").arg(m_path), "[Path::parentPath]");

	if (resPath.size() == 2) {
		pos = resPath.find(':');
		if (pos != std::string::npos) {
			throw IBK::Exception("No parent possible.", "[Path::parentPath]");
		}
	}

	pos = resPath.rfind('/');
	if (pos != std::string::npos) {
		resPath = resPath.substr(0, pos);
		return Path(resPath);
	}
	else {
		return Path(".");
	}
}


bool Path::isValid() const {
	return m_path.find_first_not_of(" \t") != std::string::npos;
}


bool Path::isFile() const {
	return directoryExists(m_path) == IsFile;
}


bool Path::isDirectory() const {
#if defined(_WIN32)
	if(isDrive())
		return true;
#endif
	return directoryExists(m_path) == DirectoryExists;
}


bool Path::exists() const {
#if defined(_WIN32)
	if(isDrive())
		return true;
#endif
	return directoryExists(m_path) != NoSuchDirectory;
}


bool Path::isAbsolute() const {
	return (m_path.size() >= 1 && m_path[0] == '/') || (m_path.size() >= 2 && m_path[1] == ':');
}


Path Path::filename() const {

	const char * const FUNC_ID = "[Path::filename]";

	// IBK::Path("").filename() --> exception
	if (m_path.empty())
		throw Exception( IBK::FormatString("Invalid path: '%1'").arg(m_path), FUNC_ID );

	if (m_path.size() == 1){

		// IBK::Path(".")
		if (m_path == "." || m_path == "/") {
			return Path("");
		}

	} else if ( (m_path.size() == 2) ){

		// IBK::Path("C:").filename() --> ""  (weil "C:" == "C:\\" == "C:/" == "C:" + "/" + "")
		// IBK::Path("./")
		if ( (m_path[1] == ':') || (m_path == "./") ){
			return Path("");
		}

	} else {

		// IBK::Path("C:\\blubb\\bla\\blub").filename() --> "blub" <- constructor handles this
		// IBK::Path("./blub.txt").filename() --> "blub.txt"
		// IBK::Path("../blub.txt").filename() --> "blub.txt"
		// search for last / in string and create a substring
		size_t pos = m_path.rfind('/');
		if (pos != std::string::npos){
			return Path( m_path.substr( pos+1, std::string::npos ) );
		}

	}

	// IBK::Path("blub.txt").filename() --> "blub.txt"
	return *this;

}


std::string Path::extension() const {

	if ( exists() && isDirectory() ){
		return "";
	}

	if ( exists() && isFile()) {

		// IBK::Path("../README").extension() --> ""
		std::string fullFilename = filename().str();

		std::string::size_type pos = fullFilename.find_last_of(".");
		if( pos != std::string::npos ) {
			std::string ext = fullFilename.substr( pos+1, std::string::npos );
			return ext;
		} else {
			return "";
		}

	} // if (isFile()) {

	// if it doesn't exist we assume it to be a file
	if (!exists()){

		// IBK::Path("../README").extension() --> ""
		std::string fullFilename = filename().str();

		std::string::size_type pos = fullFilename.find_last_of(".");
		if( pos != std::string::npos ) {
			std::string ext = fullFilename.substr( pos+1, std::string::npos );
			return ext;
		} else {
			return "";
		}

	}
	return "";
}


Path Path::withoutExtension() const {

	// ./dfghjk
	if (isDirectory()){
		return *this;
	}

	// ./fgh/gh/../dfghj.a
	if (isFile()){

		std::string::size_type pos = m_path.find_last_of(".");
		if( pos != std::string::npos ) {
			std::string ext = m_path.substr( 0, pos );
			return Path(ext);
		}

	} // if (isFile())

	// if it doesn't exist we assume it to be a file
	if (!exists()){

		std::string::size_type pos = m_path.find_last_of(".");
		if( pos != std::string::npos ) {
			std::string ext = m_path.substr( 0, pos );
			return Path(ext);
		}

	}

	return *this;
}


void Path::operator/=(const Path& subDir) {

	if (subDir == "/" || subDir == "." || !subDir.isValid()) {
		return;
	}

	if (subDir.m_path[0] != '/')
		m_path.push_back('/');

	m_path += subDir.m_path;
}


void Path::operator/=(const std::string& subDir) {

	if (subDir.empty())
		return;

	// constructor removes trailing slashes but remind root dir
	Path subDirPath(subDir);

	if (subDirPath == "/" || subDirPath == "." ){
		return;
	}

	*this /= subDirPath;
}


void Path::addPath(const IBK::Path & subDir) {

	if (subDir == "/" || subDir == "." || !subDir.isValid())
		return;

	*this /= subDir;
}


void Path::addExtension( const std::string &myExt ) {

	std::string ext = myExt;
	IBK::trim(ext);

	if (ext.empty())
		return;

	if (ext.size() == 1 && ext[0] == '.')
		return;

	if (ext[0] != '.')
		ext = "." + ext;

	m_path += ext;
}


bool Path::hasPlaceholder() const {

	if( m_path.find("{") != std::string::npos && m_path.find("}") != std::string::npos)
		return true;
	else
		return false;

}


std::string Path::placeholderString() const {

	std::string::size_type pos1 = m_path.find("{");
	std::string::size_type pos2 = m_path.find("}", pos1);

	if ( pos1 == std::string::npos || pos2 == std::string::npos )
	{
		return "";
	}

	return m_path.substr(pos1 + 1, pos2 - pos1 - 1);
}


Path Path::withReplacedPlaceholders(const std::map<std::string, Path> & placeHoldersMap) const {

	/// \todo check if this code works against the following lines:
	/*! Tries to exchange existing placeholders by path from the given map.
		If the path doesn't contain a placeholder or the existing placeholder cannot be found in the map
		the returning path is equal to the existing.
		\code
			IBK::Path path = "${my path}/fname.txt";
			std::map<string,IBK::Path> placeholders;
			placeholders["my path"] = "C:\\data";
			// we call replace_path_placeholders to get the full path
			IBK::Path fullpath = path.withReplacedPlaceholders(placeholders);
			// fullpath is now "C:/data/fname.txt"
		\endcode
	*/


	const char * const FUNC_ID = "[Path::withReplacedPlaceholders]";

	std::string bracketBegin= "{";
	std::string bracketEnd = "}";
	std::vector<std::string> branches;
	Path newPath(m_path); // newPath holds current path, including placeholders

	// remember whether our path is a root path
	bool haveRootPath = false;

	bool placeholderFound = true;
	while (placeholderFound) {
		if (!newPath.str().empty() && newPath.str()[0] == '/')
			haveRootPath = true;
		// current path is exploded and stored in branches
		IBK::explode(newPath.str(), branches, '/');
		newPath.clear();

		placeholderFound = false;
		for (std::vector<std::string>::iterator bit=branches.begin(); bit!=branches.end(); ++bit) {
			IBK::Path placeholderPath;
			std::string completePlaceholder;
			for (std::map<std::string,IBK::Path>::const_iterator pit = placeHoldersMap.begin(); pit != placeHoldersMap.end(); ++pit) {
				completePlaceholder = "$" + bracketBegin + pit->first + bracketEnd;
				if (*bit == completePlaceholder) {
					if (pit->second.str().empty())
						throw IBK::Exception( IBK::FormatString("Path for placeholder '%1' is empty.").arg(pit->first), FUNC_ID);
					placeholderPath = pit->second;
					placeholderFound = true;
					break;
				}
			}

			// If *bit contains a known placeholder, than placeholderPath holds the corresponding path.
			if (placeholderPath.str().empty()) {
				if (newPath.str().empty()) {
					// check if *bit has a drive letter (in Windows), in this case do not prepend the /
					// otherwise, we have a root path and need to prepend the slash
					if (!IBK::Path(*bit).drive().empty() || !haveRootPath)
						newPath = *bit;
					else
						newPath /= *bit;
				}
				else
					newPath /= *bit;
			}
			else {
				// if placeHolderPath is an absolut path (on windows contains drive letters, on Unix starts with slash)
				// do not add it to the newPath, but instead set newPath to it, newPath must be an empty string in this case
				if (placeholderPath.str()[0] == '/' ||
					(placeholderPath.str().size() > 1 && placeholderPath.str()[1] == ':'))
				{
					if (newPath.str().empty())
						newPath = placeholderPath;
					else
						throw IBK::Exception( IBK::FormatString("Cannot add absolute path '%1' (from replacing placeholder '%2') to path part '%3'.")
							.arg(placeholderPath.str()).arg(completePlaceholder).arg(newPath.str()), FUNC_ID);
				}
				else {
					if (newPath.str().empty()) {
						// check if *bit has a drive letter (in Windows), in this case do not prepend the /
						// otherwise, we have a root path and need to prepend the slash
						if (!IBK::Path(placeholderPath).drive().empty() || !haveRootPath)
							newPath = placeholderPath;
						else
							newPath /= placeholderPath;
					}
					else
						newPath /= placeholderPath;
				}
			}
		}
	}

	// Still a placeholder in the filename? Than this is unknown to us
	std::string newPathString = newPath.str();
	std::string::size_type pos = newPathString.find("$" + bracketBegin);
	if (pos != std::string::npos) {
		std::string::size_type pos2 = newPathString.find(bracketEnd, pos);
		if (pos != std::string::npos) {
			std::string allPlaceholders;
			for (std::map<std::string,IBK::Path>::const_iterator it = placeHoldersMap.begin();
				it != placeHoldersMap.end(); ++it)
			{
				if (!allPlaceholders.empty())
					allPlaceholders += '\n';
				allPlaceholders += "$" + bracketBegin + it->first + bracketEnd + " = " + it->second.absolutePath().str();
			}
			throw IBK::Exception( IBK::FormatString("Unknown or undefined path placeholder '%1'. Known placeholders are:\n%2")
				  .arg(newPathString.substr(pos, pos2-pos+1)).arg(allPlaceholders), "[Project::replacePlaceHolders]");
		}
	}

	return newPath;
}


bool Path::extractPlaceholder(std::string & placeholderName, IBK::Path & relativePathPart) const
{
	std::string::size_type pos1 = m_path.find('{');
	if (pos1 == std::string::npos) {
		placeholderName.clear();
		relativePathPart = m_path;
		return false;
	}
	std::string::size_type pos2 = m_path.find('}', pos1+1);
	if (pos2 == std::string::npos) {
		placeholderName.clear();
		relativePathPart = m_path;
		return false;
	}
	placeholderName = m_path.substr(pos1+1, pos2-pos1-1);
	relativePathPart = m_path.substr(pos2+2); // mind to skip the '/' following the closing '}'
	if (placeholderName.empty())
		return false;
	else
		return true;

}


bool Path::insertPlaceholder(const std::string & placeholderName, const IBK::Path & placeholderPath){

	// get length of replacement
	std::string replacement = placeholderPath.str();
	if (m_path.compare(0, replacement.length(), replacement) == 0)
	{
		// ok - we found a match
		m_path = "${" + placeholderName + "}/" + m_path.substr( replacement.length()+1, std::string::npos );
		return true;

	} else {

		return false;

	}

}


unsigned int Path::branchCount() const {

	if( m_path.empty())
		return 0;

	unsigned int count = 1;
	std::string::size_type pos = m_path.find('/');
	while(pos != std::string::npos) {
		if( pos < m_path.size() - 1)
			++count;
		pos = m_path.find('/' , pos + 1);
	}
	return count;
}



void Path::removeRelativeParts() {

	if(m_path.empty())
		return;

	bool isRootRelated = (m_path[0] == '/');

	bool hasRel = m_path.find("/../") != std::string::npos;

	// check if drive exists, path contains relative parts and has no placeholder
	if( !hasRel || !isAbsolute() || hasPlaceholder()) {
		return;
	}

	std::vector<std::string> branches;
	IBK::explode(m_path, branches, '/');
	Path::removeRelativeParts(branches);
	// path not valid
	if(branches.empty()) {
		m_path.clear();
	}
	else {

		if ( isRootRelated ) {

			m_path.clear();
			for(std::vector<std::string>::iterator bit=branches.begin(); bit!=branches.end(); ++bit) {
				m_path += "/" + *bit;
			}

		} else {

			m_path = branches.front();
			for(std::vector<std::string>::iterator bit=branches.begin()+1; bit!=branches.end(); ++bit) {
				m_path += "/" + *bit;
			}

		}

	}
}


Path Path::subBranch(unsigned int begin, unsigned int count) const {

	const char * const FUNC_ID = "[Path::subBranch]";
	bool isLinuxAbsolute = false;

	if( begin == 0 && count == 0)
		return *this;

	if (!isValid())
		throw IBK::Exception( IBK::FormatString("Invalid path: '%1'").arg(m_path), FUNC_ID );

	if ( m_path[0] == '/' ) {
		isLinuxAbsolute = true;
	}

	std::vector<std::string> branches;
	IBK::explode(m_path, branches, '/');
	if (isLinuxAbsolute){
		branches.insert( branches.begin(), "/" );
	}

	// do not forget root directory (Linux) or Root drive
	unsigned int size = branchCount();

	if ( count > size )
		throw IBK::Exception( "Insufficient branches for given count.", FUNC_ID );

	if ( begin > size-1 )
		throw IBK::Exception( "Illegal index for branch start.", FUNC_ID );

	if( count == 0)
		count = size - begin;


	unsigned int end = begin + count;
	if( end > size)
		end = size;


	Path result;
	if( begin >= end)
		return result;

	result = branches[begin];
	for( unsigned int i=begin+1; i<end; ++i) {
		result /= branches[i];
	}

	return result;
}


bool Path::isRootPath(const Path& src) {

	IBK::Path cur = current();

	std::vector<std::string> currentBranches;
	std::vector<std::string> srcBranches;
	IBK::explode( cur.str(), currentBranches, '/' );
	IBK::explode( src.m_path, srcBranches, '/' );
	for(unsigned int i=0; i<currentBranches.size(); ++i) {
		if(i<srcBranches.size() && srcBranches[i] != currentBranches[i])
			return false;
	}
	return true;
}

bool Path::isRootPath( const Path& target, const Path& src ){

	std::vector<std::string> currentBranches;
	std::vector<std::string> srcBranches;
	IBK::explode( target.str(), currentBranches, '/' );
	IBK::explode( src.m_path, srcBranches, '/' );
	for(unsigned int i=0; i<currentBranches.size(); ++i) {
		if(i<srcBranches.size() && srcBranches[i] != currentBranches[i])
			return false;
	}
	return true;

}


int64_t Path::fileSize() const {

	if(!isFile())
		return -1;

#if defined(_WIN32)
	FileHandlerWrapper fileToRead(m_path, false);
	if( !fileToRead.isValid()) {
//		DWORD error = fileToRead.error();
		return 0;
	}
	DWORD FileSizeLow, FileSizeHigh;
	FileSizeLow = GetFileSize(fileToRead.handle(), &FileSizeHigh);
	if( FileSizeLow == 0)
		return 0;
	LARGE_INTEGER FileSize;
	FileSize.LowPart = FileSizeLow;
	FileSize.HighPart = FileSizeHigh;

	// removed warning for runtime constant expression test
//	if (sizeof(int64_t) == 8)
//		return FileSize.QuadPart;
//	return FileSize.LowPart;
	return FileSizeReturner<sizeof(int64_t)>(FileSize);

#else
	std::FILE *f = std::fopen(m_path.c_str(),"rb");  /* open the file in read only */
	long size = 0;
	if (std::fseek(f,0,SEEK_END)==0) /* seek was successful */
		size = std::ftell(f);
	std::fclose(f);
	return size;
#endif
}


std::time_t Path::lastWriteTime() const {

	std::time_t time = 0;
#if defined(_WIN32)
	FileHandlerWrapper fileToRead(m_path, false);
	if( !fileToRead.isValid()) {
//		DWORD error = fileToRead.error();
		return time;
	}
	FILETIME creationTime;
	FILETIME lastAccessTime;
	FILETIME lastWriteTime;
	if( !GetFileTime(fileToRead.handle(), &creationTime, &lastAccessTime, &lastWriteTime))
		return time;
// converting FILETIME via SYSTEMTIME to time_t
	LARGE_INTEGER jan1970FT;
	jan1970FT.QuadPart = 0;
	int64_t fakt64 = 1164447360;
	fakt64 *= 100000000;
	jan1970FT.QuadPart = fakt64; // january 1st 1970
	LARGE_INTEGER utcFT = *(LARGE_INTEGER*)(&lastWriteTime);
	uint64_t utcDosTime = (utcFT.QuadPart - jan1970FT.QuadPart) / 10000000;
	time = (time_t)utcDosTime;
#else

	/*
	Linux: three distinct timestamps associated with a file:
		- time of last access of contents (atime),
		- time of last modification of contents (mtime),
		- and time of last modification of the inode (metadata, ctime).
	*/

	//1. create a file attribute structure
	struct stat attrib;

	//2. get the attributes of afile.txt
	stat(m_path.c_str(), &attrib);

	//3. Get the last modified time and put it into the time structure
	time = attrib.st_mtime;

#endif
	return time;
}


std::time_t Path::fileTime() const{
	std::time_t time = 0;
#if defined(_WIN32)
	FileHandlerWrapper fileToRead(m_path, false);
	if( !fileToRead.isValid()) {
//		DWORD error = fileToRead.error();
		return time;
	}
	FILETIME creationTime;
	FILETIME lastAccessTime;
	FILETIME lastWriteTime;
	if( !GetFileTime(fileToRead.handle(), &creationTime, &lastAccessTime, &lastWriteTime))
		return time;
// converting FILETIME via SYSTEMTIME to time_t
	LARGE_INTEGER jan1970FT;
	jan1970FT.QuadPart = 0;
	int64_t fakt64 = 1164447360;
	fakt64 *= 100000000;
	jan1970FT.QuadPart = fakt64; // january 1st 1970
	LARGE_INTEGER utcFT = *(LARGE_INTEGER*)(&lastWriteTime);
	uint64_t utcDosTime = (utcFT.QuadPart - jan1970FT.QuadPart) / 10000000;
	time = (time_t)utcDosTime;
#else

	/*
	Linux: three distinct timestamps associated with a file:
		- time of last access of contents (atime),
		- time of last modification of contents (mtime),
		- and time of last modification of the inode (metadata, ctime).
	*/

	//1. create a file attribute structure
	struct stat attrib;

	//2. get the attributes of afile.txt
	stat(m_path.c_str(), &attrib);

	//3. Get the last modified time and put it into the time structure
	time = attrib.st_mtime;

#endif
	return time;
}


bool Path::setFileTime(	const IBK::Path& filename,
						int hour,
						int minute,
						int second,
						int day,
						int month,
						int year)
{
	const char * const FUNC_ID = "[Path::setFileTime]";

#if defined(_WIN32)

	FileHandlerWrapper fileToChange(filename.absolutePath().str(), true);
	if( !fileToChange.isValid()) {
		return false;
	}


	// get current date and time
	FILETIME ftLastAcc;
	GetFileTime(fileToChange.handle(),0,&ftLastAcc,0);

	SYSTEMTIME systime;
	systime.wYear = (WORD)year;
	systime.wMonth = (WORD)month;
	systime.wDay = (WORD)day;
	systime.wHour = (WORD)hour;
	systime.wMinute = (WORD)minute;
	systime.wSecond = (WORD)second;

	FILETIME ftLocal;
	int res = SystemTimeToFileTime(&systime, &ftLocal);
	if(!res) {
		std:: string errmsg = GetLastErrorMessage();
		IBK_Message(FormatString("Error in SystemTimeToFileTime '%1' !").arg(errmsg), MSG_ERROR, FUNC_ID);
		return false;
	}

	FILETIME ftm;
	res = LocalFileTimeToFileTime(&ftLocal,&ftm);
	if(!res) {
		std:: string errmsg = GetLastErrorMessage();
		IBK_Message(FormatString("Error in LocalFileTimeToFileTime '%1' !").arg(errmsg), MSG_ERROR, FUNC_ID);
		return false;
	}

	// this function updates creation and write time, last access time is kept constant
	res = SetFileTime(fileToChange.handle(),0,0,&ftLocal);
	if(!res) {
		std:: string errmsg = GetLastErrorMessage();
		IBK_Message(FormatString("Error '%1' !").arg(errmsg), MSG_ERROR, FUNC_ID);
		return false;
	}

	return true;
#else

	/*
	Linux: three distinct timestamps associated with a file:
		- time of last access of contents (atime),
		- time of last modification of contents (mtime),
		- and time of last modification of the inode (metadata, ctime).
	*/

	std::time_t timer;
	struct tm y2k;

	y2k.tm_hour = hour;
	y2k.tm_min = minute;
	y2k.tm_sec = second;
	y2k.tm_year = year-1900;
	y2k.tm_mon = month-1;
	y2k.tm_mday = day;

	// put it into the time structure
	timer = mktime(&(y2k));

	// create a file attribute structure
	struct stat attrib;
	struct utimbuf new_times;

	// get the attributes of afile.txt
	stat(filename.absolutePath().str().c_str(), &attrib);

	// keep atime unchanged and set last modified time
	new_times.actime	= attrib.st_atime;
	new_times.modtime	= timer;

	/// \todo go over to new posix standard -> utimes()
	// set access time back to where it started
	if ( utime(filename.absolutePath().str().c_str(), &new_times) == 0) {
		return true;
	} else {
		switch (errno) {
			case EACCES			: IBK_Message(FormatString("Cannot set file time to %1, access denied!").arg(filename), MSG_ERROR, FUNC_ID); break;
			case ENOENT			: IBK_Message(FormatString("Cannot set file time to %1, some component of the path doesn't exist!").arg(filename), MSG_ERROR, FUNC_ID); ;break;
			case EPERM			: IBK_Message(FormatString("Cannot set file time to %1, not enough permissions!").arg(filename), MSG_ERROR, FUNC_ID); break;
			case EROFS			: IBK_Message(FormatString("Cannot set file time to %1, read only file system!").arg(filename), MSG_ERROR, FUNC_ID); break;
		}
		return false;
	}



#endif
}


// static functions
Path Path::current(){

#if defined(_WIN32)
	wchar_t currentDir[FILENAME_MAX] = {0};
	if(! _wgetcwd(currentDir, FILENAME_MAX)) {
		switch(errno) {
			case ENOMEM: {
				IBK::IBK_Message( FormatString("Out of memory! Can not reserve %1 bytes").arg(FILENAME_MAX), IBK::MSG_ERROR, "[userDirectory()]", IBK::VL_ALL );
				break;
			}
			case ERANGE: {
				IBK::IBK_Message( FormatString("Path longer than %1 characters").arg(FILENAME_MAX), IBK::MSG_ERROR, "[userDirectory()]", IBK::VL_ALL );
				break;
			}
			default: {
				IBK::IBK_Message( "Couldn't deduce user directory. ", IBK::MSG_ERROR, "[userDirectory()]", IBK::VL_ALL );
			}
		}

		return Path("");
	}
	try {
		return Path( WstringToUTF8(currentDir) );
	}
	catch(...) {
		return Path("");
	}
#elif __GNUC__
	char currentDir[FILENAME_MAX] = {0};
	if(!getcwd(currentDir, FILENAME_MAX)) {
		/// \todo some more error handling here
		IBK::IBK_Message( "Couldn't deduce user directory. ", IBK::MSG_ERROR, "[userDirectory()]", IBK::VL_ALL );
		return Path("");
	}
	return Path(currentDir);
#else
	#error Implement this for your compiler
#endif

}


bool Path::setCurrent(const IBK::Path & p){

#if defined(_WIN32)
	if (_wchdir(p.wstr().c_str()) == 0) {
		return true;
	}
	else {
		std::string errmsg;
		switch(errno) {
			case ENOENT: errmsg = "Path not found"; break;
			case EINVAL: errmsg = "Invalid path"; break;
			default: errmsg = "Unknown error";
		}
		IBK::IBK_Message( errmsg, IBK::MSG_ERROR, "[setCurrent()]", IBK::VL_ALL );
	}

#elif __GNUC__

	if (chdir(p.str().c_str()) == 0) {
		return true;
	}

#else
	#error Implement this for your compiler
#endif

	return false;

}


bool Path::makePath( const IBK::Path & p ) {

	const char* FUNC_ID = "[IBK_Path::makePath]";

	if( !p.isValid() )
		return false;

	if( p.isFile())
		return false;

	if( p.isDirectory())
		return true;

	bool result = true;
	Path parentDir = p.parentPath();
	if( parentDir.isValid() && !parentDir.isDirectory())
		result = result && makePath( parentDir );

	if( result ){

#ifdef _WIN32
		try {
			std::wstring wdirname = UTF8ToWstring(p.str());
			int res = CreateDirectoryW(wdirname.c_str(), 0);
			if(!res) {
				std:: string errmsg = GetLastErrorMessage();
				IBK_Message(FormatString("Error in LocalFileTimeToFileTime '%1' !").arg(errmsg), MSG_ERROR, FUNC_ID);
				return false;
			}
		}
		catch(...) {
			IBK_Message("Cannot convert directory '"+p.str()+"', from UTF8 to Unicode!", MSG_ERROR, FUNC_ID);
			return false;
		}

#elif __GNUC__


		std::string s = p.str();
		mode_t mode = 0755;

		size_t pre=0,pos;
		std::string dir;
		int mdret;

		if(s[s.size()-1]!='/'){
			// force trailing / so we can handle everything in loop
			s+='/';
		}

		while((pos=s.find_first_of('/',pre))!=std::string::npos){
			dir=s.substr(0,pos++);
			pre=pos;
			if(dir.size()==0) continue; // if leading / first time is 0 length
			if((mdret=mkdir(dir.c_str(),mode)) && errno!=EEXIST){
				switch (mdret) {
					case EACCES			: IBK_Message(FormatString("Cannot create directory %1, access denied!").arg(p), MSG_ERROR, FUNC_ID); break;
					case EEXIST			: IBK_Message(FormatString("Cannot create directory %1, it alreay exists!").arg(p), MSG_ERROR, FUNC_ID); break;
					case ENOTDIR		: IBK_Message(FormatString("Cannot create directory %1, invalid path!").arg(p), MSG_ERROR, FUNC_ID); break;
					case ENOENT			: IBK_Message(FormatString("Cannot create directory %1, some component of the path doesn't exist!").arg(p), MSG_ERROR, FUNC_ID); ;break;
					case EFAULT			: IBK_Message("EFAULT", MSG_ERROR, FUNC_ID); break;
					case ELOOP			: IBK_Message("ELOOP", MSG_ERROR, FUNC_ID); break;
					case ENAMETOOLONG	: IBK_Message("ENAMETOOLONG", MSG_ERROR, FUNC_ID); break;
					case ENOMEM			: IBK_Message("ENOMEM", MSG_ERROR, FUNC_ID); break;
				}
				return false;
			}
		}

#else

#error Implement this for your compiler

#endif

		if( p.isDirectory())
			return true;

	}

	return false;

}


bool Path::remove(const IBK::Path & p) {

	// change current path to higher level before executing delete
	if ( isRootPath( current(), p ) ) {
		setCurrent( p.parentPath() );
	}

#if defined(_WIN32)
	const char* FUNC_ID = "[Path::remove]";

	SHFILEOPSTRUCTW stSHFileOpStruct;
	std::memset(&stSHFileOpStruct,0,sizeof(stSHFileOpStruct));

	try {
		std::wstring wdir = p.wstrOS();
		std::vector<wchar_t> wtempstr(wdir.begin(), wdir.end());
		wtempstr.push_back(L'\0');
		wtempstr.push_back(L'\0');
		stSHFileOpStruct.wFunc = FO_DELETE;
		stSHFileOpStruct.pFrom = &wtempstr[0];
		stSHFileOpStruct.fFlags = FOF_NOCONFIRMATION | FOF_SILENT | FOF_ALLOWUNDO;
		int nFileDeleteOprnRet = SHFileOperationW( &stSHFileOpStruct );
		if(nFileDeleteOprnRet != 0) {
			int abort = stSHFileOpStruct.fAnyOperationsAborted;
			if(abort) {
				IBK_Message(FormatString("Copy abort by user!"), MSG_ERROR, FUNC_ID);
			}
			else {
				std:: string errmsg = GetLastErrorMessage(nFileDeleteOprnRet);
				IBK_Message(FormatString("Error while copy '%1' !").arg(errmsg), MSG_ERROR, FUNC_ID);
			}
			return false;
		}
		return true;
	}
	catch(...) {
		return false;
	}
#else

	// delete path
	std::string str("rm -rf ");
	str.append(p.str().c_str());
	if ( std::system( str.c_str() ) ) {
		return false;
	} else {
		return true;
	}
#endif

}


bool Path::copy(const IBK::Path & source, const IBK::Path & target){

	const char* FUNC_ID = "[Path::copy]";

	if ( !source.exists() ) {
		return false;
	}

	// prevent recursive copy
	if ( IBK::Path::isRootPath(target, source) ) {
		return false;
	}

	if ( source.isFile() ) {

		// check if target allready exists
		if ( target.exists() && target.isFile() ){
			/// \todo think about this code
			IBK::IBK_Message( IBK::FormatString("We will overwrite %1").arg(target), MSG_WARNING, FUNC_ID );
		}

		if ( target.exists() && target.isDirectory() ){

			// IBK::Path::copy("/tmp/blubb.txt", "/tmp/blubbs");
			// 'blubbs' is a directory -> "cp blubb.txt blubbs" -> "/tmp/blubbs/blubb.txt"

			IBK::Path targetFile = target / source.filename();

			#if defined(_WIN32)

				#pragma warning(disable:4800)
				BOOL failIfExists = FALSE;
				return CopyFileW(source.absolutePath().wstr().c_str(), targetFile.absolutePath().wstr().c_str(), failIfExists);
				#pragma warning(default:4800)

			#else

				std::ifstream in(source.absolutePath().str().c_str(), std::ios::binary);
				std::ofstream out(targetFile.absolutePath().str().c_str(), std::ios::binary);
				out << in.rdbuf();
				out.flush();
				return out.good();

			#endif

		}

		// IBK::Path::copy("/tmp/blubb.txt", "/tmp/bla.txt");
		// same as "cp blubb.txt bla.txt" in /tmp
		#if defined(_WIN32)
		#pragma warning(disable:4800)
			return CopyFileW(source.absolutePath().wstr().c_str(), target.absolutePath().wstr().c_str(), FALSE);
		#pragma warning(default:4800)
		#else
			std::ifstream in(source.absolutePath().str().c_str(), std::ios::binary);
			std::ofstream out(target.absolutePath().str().c_str(), std::ios::binary);
			out << in.rdbuf();
			out.flush();
			return out.good();
		#endif

	} // if ( source.isFile() )

	if ( source.isDirectory() ) {

		if ( (target.exists() && target.isDirectory()) || !target.exists() ){

			// IBK::Path::copy("/tmp/blubbs", "/tmp/blubb.txt");
			// 'blubbs' is a directory, 'blubb.txt' must also be a directory, otherwise function returns false

			#if defined(_WIN32)

			// both paths must be double 0 terminated
				std::wstring sourceDirString = source.absolutePath().wstrOS();
				std::vector<wchar_t> sourceDir(sourceDirString.begin(), sourceDirString.end());
//                if(!target.exists()) {
					sourceDir.push_back(L'\\');
					sourceDir.push_back(L'*');
//                }
				sourceDir.push_back(L'\0');
				sourceDir.push_back(L'\0');

				std::wstring targetDirString = target.absolutePath().wstrOS();
				std::vector<wchar_t> targetDir(targetDirString.begin(), targetDirString.end());
				targetDir.push_back(L'\0');
				targetDir.push_back(L'\0');

				SHFILEOPSTRUCTW s = {0};
				s.wFunc = FO_COPY;
				s.pTo = &targetDir[0];
				s.pFrom = &sourceDir[0];
				s.fFlags = FOF_SILENT | FOF_NOCONFIRMMKDIR | FOF_NOCONFIRMATION | FOF_NOERRORUI;
				int res = SHFileOperationW( &s );
				if(res != 0) {
					int abort = s.fAnyOperationsAborted;
					if(abort) {
						IBK_Message(FormatString("Copy abort by user!"), MSG_ERROR, FUNC_ID);
					}
					else {
						std:: string errmsg = GetLastErrorMessage(res);
						IBK_Message(FormatString("Error while copy '%1' !").arg(errmsg), MSG_ERROR, FUNC_ID);
					}
					return false;
				}
				return true;
			#else

				DIR *dp;
				struct dirent *ep;
				dp = opendir (source.absolutePath().str().c_str());

				if (dp == NULL)
					return false; // should have been tested for before
				// get file list
				std::list<std::string> filelist;
				while ((ep = readdir (dp))) {
					/// \todo check for file type
					filelist.push_back(ep->d_name);
				}

				// remove local path and parent path from file list
				std::list<std::string>::iterator it1 = std::find( filelist.begin(), filelist.end(), "." );
				if ( it1 != filelist.end() ) {
					filelist.erase( it1 );
				}
				std::list<std::string>::iterator it2 = std::find( filelist.begin(), filelist.end(), ".." );
				if ( it2 != filelist.end() ) {
					filelist.erase( it2 );
				}

				// create target directory
				int res = 0;
				if (!target.exists()){
					res = mkdir(target.absolutePath().str().c_str(), 0755); // unix/linux handles umask automatically for you
					if (res != 0)
						return false;
				}
				for (std::list<std::string>::const_iterator it = filelist.begin(); it != filelist.end(); ++it) {
					copy(source / *it, target / *it);
				}

				(void) closedir (dp);
				return true;

			#endif

		}/* else {
//			return false;
		}*/

	} // if ( source.isDirectory() )

	return false;

}


bool Path::move(const IBK::Path & source, const IBK::Path & target){

	const char * const FUNC_ID = "[Path::move]";

	// prevent recursive path move
	if ( target.isRootPath(source) ) {
		return false;
	}

#if defined(_WIN32)
	// both paths must be double 0 terminated
	std::wstring sourceDirString = source.absolutePath().wstrOS();
	std::vector<wchar_t> sourceDir(sourceDirString.begin(), sourceDirString.end());
	if(!target.exists() && !source.isFile()) {
		sourceDir.push_back(L'\\');
		sourceDir.push_back(L'*');
	}
	sourceDir.push_back(L'\0');
	sourceDir.push_back(L'\0');

	std::wstring targetDirString = target.absolutePath().wstrOS();
	std::vector<wchar_t> targetDir(targetDirString.begin(), targetDirString.end());
	targetDir.push_back(L'\0');
	targetDir.push_back(L'\0');

	SHFILEOPSTRUCTW s = {0};
	s.wFunc = FO_MOVE;
	s.pTo = &targetDir[0];
	s.pFrom = &sourceDir[0];
	s.fFlags = FOF_SILENT | FOF_NOCONFIRMMKDIR | FOF_NOCONFIRMATION | FOF_NOERRORUI;
	int res = SHFileOperationW( &s );
	if(res != 0) {
		int abort = s.fAnyOperationsAborted;
		if(abort) {
			IBK_Message(FormatString("Move abort by user!"), MSG_ERROR, FUNC_ID);
		}
		else {
			std:: string errmsg = GetLastErrorMessage(res);
			IBK_Message(FormatString("Error while move directory '%1' !").arg(errmsg), MSG_ERROR, FUNC_ID);
		}
		return false;
	}
	return true;
#elif defined(__GNUC__)

	if ( !remove(target) ){
		IBK_Message( FormatString("Cannot remove override target %1, probably access denied!").arg(target), MSG_ERROR, FUNC_ID );
	}
	int result = std::rename(source.str().c_str(), target.str().c_str());
	if ( result == 0 ) {
		return true;
	} else {
		switch (errno) {
			case EACCES			: IBK_Message(FormatString("Cannot move to %1, access denied!").arg(target), MSG_ERROR, FUNC_ID); break;
			case ENOENT			: IBK_Message(FormatString("Cannot move to %1, some component of the path doesn't exist!").arg(target), MSG_ERROR, FUNC_ID); ;break;
			case EPERM			: IBK_Message(FormatString("Cannot move to %1, not enough permissions!").arg(target), MSG_ERROR, FUNC_ID); break;
			case EROFS			: IBK_Message(FormatString("Cannot move to %1, read only file system!").arg(target), MSG_ERROR, FUNC_ID); break;
		}
	}

#else
	#error Write this for your compiler
#endif

	return false;

}


Path Path::fromURI(std::string uripath) {

	if (uripath.find("file:/") != std::string::npos) {
		if (uripath.find("file:///") != std::string::npos) {
			uripath = uripath.substr(8);
		}
		else {
			uripath = uripath.substr(6);
		}
		IBK::trim(uripath);
		// if not on windows, prepend path with a slash
		if (uripath.size()>2) {
			if (uripath[1] != ':') {
				uripath = "/" + uripath;
			}
		}
	}
	return IBK::Path(uripath);
}


#if defined(_WIN32)
Path::DriveInformation Path::driveType() const {

	if(m_path.size() != 2 || m_path[1] != ':')
		return DI_NO_DRIVE;

	// can be a existing drive
	// check if drive exist on system
	char targetPath[MAX_PATH] = {'\0'};
	DWORD res = QueryDosDeviceA(m_path.c_str(), targetPath, MAX_PATH);
	if(res == 0)
		return DI_UNKNOWN;
	unsigned int type = GetDriveTypeA(m_path.c_str());
	return static_cast<DriveInformation>(type);

}

bool Path::isDrive() const {
	DriveInformation dinfo = driveType();
	return dinfo >= DI_REMOVABLE && dinfo <= DI_RAMDISK;
}
#endif // defined(_WIN32)


/*******************************
 *
 * Protected Functions
 *
********************************/

void Path::removeRelativeParts(std::vector<std::string>& branches) {
	if(branches.empty())
		return;
	std::vector<std::string>::iterator it = std::find(branches.begin(), branches.end(), "..");
	// no relative parts found
	if(it == branches.end())
		return;
	// relative part found at beginning - path not valid or relative
	if(it == branches.begin()) {
		branches.clear();
		return;
	}
	branches.erase(it-1, it+1);
	removeRelativeParts(branches);
}


void Path::remove_trailing_slash(std::string& path) const {
	if (path.empty() || path.size() == 1)
		return;
	std::string::size_type pos1 = path.rfind('/');
	std::string::size_type pos2 = path.rfind('\\');
	std::string::size_type lastPos = path.size()-1;
	if (pos1 == lastPos || pos2 == lastPos) {
		path = path.substr(0, lastPos);
	}
}


std::string Path::remove_trailing_slash_copy(const std::string& path) const {
	if (path.empty())
		return path;
	std::string res = path;
	std::string::size_type pos1 = res.rfind('/');
	std::string::size_type pos2 = res.rfind('\\');
	std::string::size_type lastPos = res.size()-1;
	if (pos1 == lastPos || pos2 == lastPos) {
		res = res.substr(0, lastPos);
	}
	return res;
}


void Path::set(const std::string& path) {

	m_path = path;

	IBK::trim(m_path);

	if (!m_path.empty())
		std::replace(m_path.begin(), m_path.end(), '\\', '/');

	remove_trailing_slash(m_path);
	removeRelativeParts();

	// IBK::Path("./blub.txt") == "blub.txt"; //  (leading ./ is not stored)
	if ( m_path.size() > 2 && m_path[0] == '.' && m_path[1] == '/' ){
		m_path.replace( 0, 2, "");
	}

	// any first character followed by : must be upper case
	if ( m_path.size() >= 2 && m_path[1] == ':' ){
		m_path = firstCharToUpperUtf8(m_path);
	}

}

/*******************************
 *
 * Private Functions
 *
********************************/

Path::DirExistsResult Path::directoryExists(const std::string& dirname) const {

	std::string cleanPath = remove_trailing_slash_copy(dirname);

#ifdef _WIN32
	// implementation for MS VC
	struct _stat buf;
	try {
		std::wstring wfilename = UTF8ToWstring(cleanPath);
		int res = _wstat(wfilename.c_str(), &buf);
		if (res == -1)
			return NoSuchDirectory;

		// check if it is a directory
		if (buf.st_mode & _S_IFDIR)
			return DirectoryExists;

		return IsFile;
	}
	catch(...) {
		return NoSuchDirectory;
	}
#elif __GNUC__
	struct stat buf;
	int res = stat(cleanPath.c_str(), &buf);
	if (res == -1) {
		switch (errno) {
			case EACCES : IBK_Message("Cannot access file/directory '"+dirname+"'!", MSG_ERROR); break;
		}
		return NoSuchDirectory;
	}
	// check if it is a directory
	if (S_ISDIR(buf.st_mode))
		return DirectoryExists;

	return IsFile;

#else
#error "Implement this for your compiler!"
#endif
}


std::string Path::firstCharToUpperUtf8(const std::string& orig) const {

	std::string retVal;
	retVal.reserve(orig.size());
	std::mbstate_t state = std::mbstate_t();
	std::vector<char> buf(MB_CUR_MAX + 1, '\0');
	size_t i = 0;
	if (orig.size() > 0) {
		if (orig[i] > 0) {
			retVal += static_cast<char>(toupper(orig[i]));
			++i;
		} else {
			wchar_t wChar;
			int len = mbrtowc(&wChar, &orig[i], MB_CUR_MAX, &state);
			// If this assertion fails, there is an invalid multi-byte character.
			// However, this usually means that the locale is not utf8.
			// Note that the default locale is always C. Main classes need to set them
			// To utf8, even if the system's default is utf8 already.
			IBK_ASSERT(len > 0 && len <= static_cast<int>(MB_CUR_MAX));
			i += len;
			int ret = wcrtomb(&buf[0], towupper(wChar), &state);
			IBK_ASSERT(ret > 0 && ret <= static_cast<int>(MB_CUR_MAX));
			buf[ret] = 0;
			retVal += &buf[0];
		}
	}
	for (; i < orig.size(); ++i) {
		retVal += orig[i];
	}
	return retVal;
}


/*******************************
 *
 * Global Functions
 *
********************************/

const Path operator/(const Path& lhs, const Path& rhs) {
	Path result = lhs;
	result /= rhs;
	return result;
}

const Path operator/(const Path& lhs, const std::string& rhs) {
	Path result = lhs;
	result /= Path(rhs);
	return result;
}

const Path operator/(const std::string& lhs, const Path& rhs) {
	Path result(lhs);
	result /= rhs;
	return result;
}


/*******************************
 *
 * Path Class Interface End
 *
********************************/

//void remove_relative_path_parts(std::string& path) {
//	// make sure we deal with /
//	std::replace(path.begin(), path.end(), '\\', '/');
//	// look for .. in the string
//	std::string::size_type pos = 0;
//	std::string::size_type dots_pos;
//	while ((dots_pos = path.find("/..", pos)) != std::string::npos) {
//		// try to find a slash prior to the position with the dots
//		std::string::size_type slash_pos = path.rfind("/", dots_pos-1);
//		if (slash_pos != std::string::npos && path.substr(slash_pos+1, dots_pos - slash_pos - 1) != "..") {
//			path = path.substr(0, slash_pos) + path.substr(dots_pos+3);
//			pos = slash_pos;
//		}
//		else
//			pos = dots_pos + 3;
//		if (pos >= path.size()) break;
//	}
//}

//std::string replace_spaces(const std::string& path) {
//	// for unix, replace spaces in path with escaped spaces
//	std::string fname;
//	size_t pos = path.find(" ");
//	size_t lastpos = 0;
//	while (pos != std::string::npos) {
//		fname += path.substr(lastpos, pos-lastpos) + "\\ ";
//		lastpos = pos+1;
//		pos = path.find(" ", lastpos);
//	}
//	fname += path.substr(lastpos);
//	return fname;
//}

//bool filter(char c)
//{
//	return c==' ' || c==':' || c=='*' || c=='?' || c=='"' || c=='<' || c=='>' || c=='/' || c=='\\';
//}

//std::string remove_special_chars( const std::string& path) {

//	std::string str = path;
//	str.erase(std::remove_if(str.begin(), str.end(), filter), str.end());  // remove spaces from string
//	return str;
//}

} // namespace IBK
