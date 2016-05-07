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

	void enterInitializationMode();
	void exitInitializationMode();

	/*! Tells the slave to integrate up the tEnd.
		At end of function, all output quantities are updated.
		\param tEnd A time larger than m_tCurrent
		\param noSetFMUStatePriorToCurrentPoint Flag is passed on in call to slave.
		\return Returns either fmiStatus or fmi2Status value (depending on FMU type) to be handled by the master algorithm.
	*/
	int doStep(double tEnd, bool noSetFMUStatePriorToCurrentPoint);

	/*! Call getFMUState() function in fmu and retrieves current state as pointer.
	*/
	void currentState(fmi2FMUstate * state) const;

	/*! Sets the state of the FMU (roll-back to recorded state). */
	void setState(fmi2FMUstate slaveState);

	/*! Retrieve all output quantities from slave and store in local vectors. */
	void cacheOutputs();

	/*! Pointer to the FMU object that instantiated this slave. */
	const FMU			* fmu() const { return m_fmu; }

	/*! Sets an input variable of type real in the slave.
		This is essentially a wrapper function around fmiSetReal or fmi2SetReal, depending on the
		standard supported by the FMU.
	*/
	void setReal(unsigned int valueReference, double value);

	/*! Sets an input variable of type int in the slave.
		This is essentially a wrapper function around fmiSetInteger or fmi2SetInteger, depending on the
		standard supported by the FMU.
	*/
	void setInteger(unsigned int valueReference, int value);

	/*! Sets an input variable of type bool in the slave.
		This is essentially a wrapper function around fmiSetBoolean or fmi2SetBoolean, depending on the
		standard supported by the FMU.
	*/
	void setBoolean(unsigned int valueReference, fmi2Boolean value);

	/*! Sets an input variable of type string in the slave.
		This is essentially a wrapper function around fmiSetString or fmi2SetString, depending on the
		standard supported by the FMU.
	*/
	void setString(unsigned int valueReference, const std::string & str);

	/*! Convenience function for setting a variable of type defined by FMIVariable.
		\param var The variable holding type and value reference.
		\param value The value as string, will be decoded into the corresponding type.
	*/
	void setValue(const FMIVariable & var, const std::string & value);

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

	/*! Determines whether debug logging shall be enabled in FMU or not.
		Defaults to false, should be set by MasterSim after reading project.
	*/
	static bool					m_useDebugLogging;

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
