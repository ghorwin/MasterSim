#ifndef PROJECT_H
#define PROJECT_H

namespace MASTER_SIM {

/*! Stores content of configuration/project file. */
class Project {
public:
	Project();

	/*! Reads project file.
		Throws an exception if reading fails.
	*/
	void read(const IBK::Path & prjFile);


	/*! Simulation end time point. */
	double m_tEnd;
};

} // namespace MASTER_SIM


#endif // PROJECT_H
