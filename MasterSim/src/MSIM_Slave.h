#ifndef MSIM_FMUSLAVE_H
#define MSIM_FMUSLAVE_H

#include <string>
#include <vector>

#include "fmi/fmiFunctions.h"
#include "fmi/fmi2Functions.h"

#include "MSIM_FMIVariable.h"

namespace MASTER_SIM {

class FMU;

/*! Holds data for a simulation slave, that is a single instance of an FMU.
	Mind that there may be several slaves instantiated by a single FMU.
*/
class Slave {
public:
	/*! Initializing constructor. */
	Slave(FMU * fmu, const std::string & name);

	/*! Destructor, frees instance and cleans up memory. */
	~Slave();

	/*! Calls FMU instantiation function to create this simulation slave.
		This function essentially populates the m_components pointer.
	*/
	void instantiateSlave();

	/*! Tells the slave to integrate up the tEnd.
		At end of function, all output quantities are updated.
		\param tEnd A time larger than m_tCurrent
		\param noSetFMUStatePriorToCurrentPoint Flag is passed on in call to slave.
		\return Returns either fmiStatus or fmi2Status value (depending on FMU type) to be handled by the master algorithm.
	*/
	int doStep(double tEnd, bool noSetFMUStatePriorToCurrentPoint);

	/*! Call getFMUState() function in fmu and retrieves current state as pointer.

	*/
	void * currentState() const;

	/*! Sets the state of the FMU (roll-back to recorded state). */
	void setState(void * slaveState);

	/*! Retrieve all output quantities from slave and store in local vectors.*/
	void cacheOutputs();

	/*! Sets an input variable in the master.

	*/
	void setReal(int globalVariableIdx, double value);

	/*! Simulator/slave ID name. */
	std::string					m_name;

	/*! Index of this slave in global slave vector of master. */
	unsigned int				m_slaveIndex;

	/*! Cached output variables of type bool, updated at end of doStep(). */
	std::vector<fmi2Boolean>	m_boolOutputs;
	/*! Cached output variables of type int, updated at end of doStep(). */
	std::vector<int>			m_intOutputs;
	/*! Cached output variables of type string, updated at end of doStep(). */
	std::vector<std::string>	m_stringOutputs;
	/*! Cached output variables of type double, updated at end of doStep(). */
	std::vector<double>			m_doubleOutputs;

	/*! Holds definitions all variables of this slave. */
	std::vector<FMIVariable>	m_variables;

private:
	/*! Pointer to the FMU object that instantiated this slave. */
	FMU			*m_fmu;

	/*! Current time the FMU is at. */
	double		m_t;

	/*! Component pointer returned by instantiation function of FMU. */
	void		*m_component;

	/*! Structure with function pointers to required call back functions. */
	static	fmiCallbackFunctions	m_fmiCallBackFunctions;
	/*! Structure with function pointers to required call back functions. */
	static	fmi2CallbackFunctions	m_fmi2CallBackFunctions;
};

} // namespace MASTER_SIM


#endif // MSIM_FMUSLAVE_H
