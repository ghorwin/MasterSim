#ifndef MSIM_ABSTRACTSLAVE_H
#define MSIM_ABSTRACTSLAVE_H

#include <string>
#include <vector>

#include <IBK_Path.h>

#include "fmi/fmiFunctions.h"
#include "fmi/fmi2Functions.h"

#include "MSIM_FMIVariable.h"

namespace MASTER_SIM {

/*! Declares the interface that MasterSim and its algorithms use to access variables.
	Basically, this interface regulates the way that variables are read and written, and
	the progress of the integration via doStep().

	This interface is currently implemented by FMUSlave and FileReaderSlave.
*/
class AbstractSlave {
public:
	/*! Constructor. */
	AbstractSlave(const std::string & name) : m_name(name), m_t(0) {}

	/*! Destructor. */
	virtual ~AbstractSlave() {}

	/*! Calls FMU instantiation function to create this simulation slave.
		This function essentially populates the m_components pointer.
	*/
	virtual void instantiate() = 0;

	/*! Wraps the call to setup experiment. */
	virtual void setupExperiment(double relTol, double tStart, double tEnd) = 0;

	virtual void enterInitializationMode() = 0;
	virtual void exitInitializationMode() = 0;

	/*! Tells the slave to integrate up the tEnd.
		At end of function, all output quantities are updated.
		\param stepSize Step size to take for integration.
		\param noSetFMUStatePriorToCurrentPoint Flag is passed on in call to slave.
		\return Returns either fmiStatus or fmi2Status value (depending on FMU type) to be handled by the master algorithm.
	*/
	virtual int doStep(double stepSize, bool noSetFMUStatePriorToCurrentPoint) = 0;

	/*! Call getFMUState() function in fmu and retrieves current state as pointer. */
	virtual void currentState(fmi2FMUstate * state) const = 0;

	/*! Sets the state of the FMU (roll-back to recorded state). */
	virtual void setState(double t, fmi2FMUstate slaveState) = 0;

	/*! Retrieve all output quantities from slave and store in local vectors. */
	virtual void cacheOutputs() = 0;

	/*! Sets an input variable of type real in the slave.
		This is essentially a wrapper function around fmiSetReal or fmi2SetReal, depending on the
		standard supported by the FMU.
	*/
	virtual void setReal(unsigned int valueReference, double value) = 0;

	/*! Sets an input variable of type int in the slave.
		This is essentially a wrapper function around fmiSetInteger or fmi2SetInteger, depending on the
		standard supported by the FMU.
	*/
	virtual void setInteger(unsigned int valueReference, int value) = 0;

	/*! Sets an input variable of type bool in the slave.
		This is essentially a wrapper function around fmiSetBoolean or fmi2SetBoolean, depending on the
		standard supported by the FMU.
	*/
	virtual void setBoolean(unsigned int valueReference, fmi2Boolean value) = 0;

	/*! Sets an input variable of type string in the slave.
		This is essentially a wrapper function around fmiSetString or fmi2SetString, depending on the
		standard supported by the FMU.
	*/
	virtual void setString(unsigned int valueReference, const std::string & str) = 0;

	/*! Convenience function for setting a variable of type defined by FMIVariable.
		\param var The variable holding type and value reference.
		\param value The value as string, will be decoded into the corresponding type.
	*/
	virtual void setValue(const FMIVariable & var, const std::string & value) = 0;

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

	/*! Variables names for boolean quantities (used for outputs). */
	std::vector<std::string>	m_boolVarNames;
	/*! Variables names for integer quantities (used for outputs). */
	std::vector<std::string>	m_intVarNames;
	/*! Variables names for string quantities (used for outputs). */
	std::vector<std::string>	m_stringVarNames;
	/*! Variables names for double quantities (used for outputs), already with appended ' [unit]' text. */
	std::vector<std::string>	m_doubleVarNames;

	/*! Holds definitions all variables of this slave. */
	std::vector<FMIVariable>	m_variables;

	/*! The full path to the fmu or data file. */
	IBK::Path m_filepath;

protected:
	/*! Current time the FMU is at.
		This value is initialized in setupExperiment() and updated in each successfuly call to
		doStep() and setState().
	*/
	double		m_t;


private:
	/*! Copy-constructor disabled. */
	AbstractSlave(const AbstractSlave&) = delete;
	/*! Assignment operator disabled. */
	AbstractSlave& operator=(const AbstractSlave& other) = delete;

};

} // namespace MASTER_SIM


#endif // MSIM_ABSTRACTSLAVE_H
