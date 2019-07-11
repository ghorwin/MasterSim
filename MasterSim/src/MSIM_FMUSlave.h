#ifndef MSIM_FMUSLAVE_H
#define MSIM_FMUSLAVE_H

#include "MSIM_AbstractSlave.h"

namespace MASTER_SIM {

class FMU;

/*! Holds data for a simulation slave, that is a single instance of an FMU.
	Mind that there may be several slaves instantiated by a single FMU.
*/
class FMUSlave : public AbstractSlave {
public:
	/*! Initializing constructor. */
	FMUSlave(FMU * fmu, const std::string & name);

	/*! Destructor, frees instance and cleans up memory. */
	~FMUSlave();

	/*! Calls FMU instantiation function to create this simulation slave.
		This function essentially populates the m_components pointer.
	*/
	void instantiate() override;

	/*! Wraps the call to setup experiment. */
	void setupExperiment(double relTol, double tStart, double tEnd) override;

	void enterInitializationMode() override;
	void exitInitializationMode() override;

	/*! Tells the slave to integrate up the tEnd.
		At end of function, all output quantities are updated.
		\param stepSize Step size to take for integration.
		\param noSetFMUStatePriorToCurrentPoint Flag is passed on in call to slave.
		\return Returns either fmiStatus or fmi2Status value (depending on FMU type) to be handled by the master algorithm.
	*/
	int doStep(double stepSize, bool noSetFMUStatePriorToCurrentPoint) override;

	/*! Call getFMUState() function in fmu and retrieves current state as pointer.
	*/
	void currentState(fmi2FMUstate * state) const override;

	/*! Sets the state of the FMU (roll-back to recorded state). */
	void setState(double t, fmi2FMUstate slaveState) override;

	/*! Retrieve all output quantities from slave and store in local vectors. */
	void cacheOutputs() override;

	/*! Pointer to the FMU object that instantiated this slave. */
	const FMU			* fmu() const { return m_fmu; }

	/*! Sets an input variable of type real in the slave.
		This is essentially a wrapper function around fmiSetReal or fmi2SetReal, depending on the
		standard supported by the FMU.
	*/
	void setReal(unsigned int valueReference, double value) override;

	/*! Sets an input variable of type int in the slave.
		This is essentially a wrapper function around fmiSetInteger or fmi2SetInteger, depending on the
		standard supported by the FMU.
	*/
	void setInteger(unsigned int valueReference, int value) override;

	/*! Sets an input variable of type bool in the slave.
		This is essentially a wrapper function around fmiSetBoolean or fmi2SetBoolean, depending on the
		standard supported by the FMU.
	*/
	void setBoolean(unsigned int valueReference, fmi2Boolean value) override;

	/*! Sets an input variable of type string in the slave.
		This is essentially a wrapper function around fmiSetString or fmi2SetString, depending on the
		standard supported by the FMU.
	*/
	void setString(unsigned int valueReference, const std::string & str) override;

	/*! Convenience function for setting a variable of type defined by FMIVariable.
		\param var The variable holding type and value reference.
		\param value The value as string, will be decoded into the corresponding type.
	*/
	void setValue(const FMIVariable & var, const std::string & value) override;


	/*! Determines whether debug logging shall be enabled in FMU or not.
		Defaults to false, should be set by MasterSim after reading project.
	*/
	static bool					m_useDebugLogging;

private:

	/*! Pointer to the FMU object that instantiated this slave. */
	FMU			*m_fmu;

	/*! Component pointer returned by instantiation function of FMU. */
	void		*m_component;

	/*! Structure with function pointers to required call back functions. */
	static	fmiCallbackFunctions	m_fmiCallBackFunctions;
	/*! Structure with function pointers to required call back functions. */
	static	fmi2CallbackFunctions	m_fmi2CallBackFunctions;
};

} // namespace MASTER_SIM


#endif // MSIM_FMUSLAVE_H
