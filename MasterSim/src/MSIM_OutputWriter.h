#ifndef MSIM_OUTPUTWRITER_H
#define MSIM_OUTPUTWRITER_H

#include <vector>
#include <iosfwd>
#include <utility>

#include <IBK_Path.h>

#include "MSIM_ProgressFeedback.h"

namespace MASTER_SIM {

class Project;
class AbstractSlave;

/*! Handles creation and writing of master outputs (output variables of slaves). */
class OutputWriter {
public:

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
	void appendOutputs(double t);


	/*! Directory where to write result files to. */
	IBK::Path				m_resultsDir;
	/*! Directory where to write statistic files to. */
	IBK::Path				m_logDir;

	/*! Project file (master sim configuration) path, written to DataIO header. */
	std::string				m_projectFile;

	/*! Pointer to project data (not owned). */
	Project					*m_project;

	/*! Vector of pointers to slave instances (not owned). */
	std::vector<AbstractSlave*>		m_slaves;

	/*! Next time point when outputs will be written again.
		If set to -1, outputs will always be written.
	*/
	double					m_tEarliestOutputTime;

	/*! Cached time point when output were last written (only needed to test if final output shall be written).
		Outputs are always written at end of simulation, except when last step was skipped because hStep was
		too small (rounding error issue).
	*/
	double					m_tLastOutput;

	ProgressFeedback		m_progressFeedback;

	std::vector< std::pair<const AbstractSlave*, unsigned int> >	m_boolOutputMapping;
	std::vector< std::pair<const AbstractSlave*, unsigned int> >	m_intOutputMapping;
	std::vector< std::pair<const AbstractSlave*, unsigned int> >	m_realOutputMapping;

	/*! Holds number output values in csv format suitable for PostProc 2.
		The numbers are written in order boolean, integer and double(real) values.
		Lastly, all double parameters are written to file (later, it will be possible to filter out certain variables).
	*/
	std::ofstream													*m_valueOutputs;

	/*! Holds string output values.
		String outputs are written all together in one csv file (not a DataIO container).
	*/
	std::ofstream													*m_stringOutputs;
	/*! Contains references to all outputs that are strings.
		The vector contains pairs of a slave and the associated index of the variable within
		the m_stringOutputs vector of the slave.
	*/
	std::vector< std::pair<const AbstractSlave*, unsigned int> >	m_stringOutputMapping;

	/*! Holds progress output. */
	std::ofstream													*m_progressOutputs;

};

} // namespace MASTER_SIM

#endif // MSIM_OUTPUTWRITER_H
