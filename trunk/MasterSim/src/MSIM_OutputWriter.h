#ifndef MSIM_OUTPUTWRITER_H
#define MSIM_OUTPUTWRITER_H

#include <vector>
#include <iosfwd>
#include <utility>

#include <IBK_Path.h>

#include "MSIM_ProgressFeedback.h"

namespace DATAIO {
	class DataIO;
}

namespace MASTER_SIM {

class Project;
class Slave;

/*! Handles creation and writing of master outputs (output variables of slaves). */
class OutputWriter {
public:
	/*! Holds dataIO for an output file - data container and output variable mapping. */
	struct OutputFileData {
		OutputFileData() : m_dataIO(NULL) {}

		/*! Pointer to DataIO container (owned). */
		DATAIO::DataIO										*m_dataIO;
		/*! References to all outputs from all slaves that should go into this file. */
		std::vector<std::pair<const Slave*, unsigned int> >	m_outputMapping;
	};

	/*! Mapping of output file types stored as DataIO. */
	enum OutputFileTypes {
		OF_BOOLEAN,
		OF_INTEGER
	};


	/*! Constructor, initializes pointers. */
	OutputWriter();
	/*! Destructor, closes all files and releases memory. */
	~OutputWriter();

	/*! Creates/opens output file.
		\param reopen If true, attempts to reopen an existing output file. Use this when restarting a simulation.
	*/
	void openOutputFiles(bool reopen);

	/*! Creates progress log and sets up progress reporting class. */
	void setupProgressReport();

	/*! Updates outputs when scheduled.
		This function is typically called after each successful step and decides
		internally, whether outputs shall be written already, or not.
	*/
	void writeOutputs(double t);


	/*! Directory where to write result files to. */
	IBK::Path				m_resultsDir;
	/*! Directory where to write statistic files to. */
	IBK::Path				m_logDir;

	/*! Project file (master sim configuration) path, written to DataIO header. */
	std::string				m_projectFile;

	/*! Pointer to project data (not owned). */
	Project					*m_project;

	/*! Vector of pointers to slave instances (not owned). */
	std::vector<Slave*>		m_slaves;

	/*! Last time point when outputs were written. */
	double					m_tLastOutput;

	ProgressFeedback		m_progressFeedback;

	/*! Output data containers.
		For each data type different files.
		Boolean outputs converted to 0 or 1 and stored in a single DataIO, index 0.
		Integer outputs stored in single DataIO, index 1.
	*/
	std::vector<OutputFileData>		m_outputFiles;

	/*! All output files of type real. */
	std::vector<OutputFileData>		m_realOutputFiles;

	/*! Holds string output values.
		String outputs are written all together in one csv file (not a DataIO container).
	*/
	std::ofstream											*m_stringOutputs;
	/*! Contains references to all outputs that are strings. */
	std::vector< std::pair<const Slave*, unsigned int> >	m_stringOutputMapping;

	/*! Vector to cache output quantities in before appending data to DataIO container. */
	std::vector<double>										m_valueVector;

	/*! Holds progress output. */
	std::ofstream											*m_progressOutputs;

};

} // namespace MASTER_SIM

#endif // MSIM_OUTPUTWRITER_H
