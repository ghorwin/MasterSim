#ifndef MSIM_FILEREADERSLAVE_H
#define MSIM_FILEREADERSLAVE_H

#include "MSIM_AbstractSlave.h"

namespace IBK {
	class CSVReader;
	class LinearSpline;
}

namespace MASTER_SIM {

/*! FileReaderSlave instance that reads data from a linear spline and provides this data
	as linearly interpolated data.
*/
class FileReaderSlave : public AbstractSlave {
public:
	/*! Initializing constructor. */
	FileReaderSlave(const IBK::Path & filepath, const std::string & name);

	/*! Destructor, frees instance and cleans up memory. */
	~FileReaderSlave();

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

	/*! Sets an input variable of type real in the slave.
		This is essentially a wrapper function around fmiSetReal or fmi2SetReal, depending on the
		standard supported by the FMU.
	*/
	void setReal(unsigned int /*valueReference*/, double /*value*/) override {}

	/*! Sets an input variable of type int in the slave.
		This is essentially a wrapper function around fmiSetInteger or fmi2SetInteger, depending on the
		standard supported by the FMU.
	*/
	void setInteger(unsigned int /*valueReference*/, int /*value*/) override {}

	/*! Sets an input variable of type bool in the slave.
		This is essentially a wrapper function around fmiSetBoolean or fmi2SetBoolean, depending on the
		standard supported by the FMU.
	*/
	void setBoolean(unsigned int /*valueReference*/, fmi2Boolean /*value*/) override {}

	/*! Sets an input variable of type string in the slave.
		This is essentially a wrapper function around fmiSetString or fmi2SetString, depending on the
		standard supported by the FMU.
	*/
	void setString(unsigned int /*valueReference*/, const std::string & /*str*/) override {}

	/*! Convenience function for setting a variable of type defined by FMIVariable.
		\param var The variable holding type and value reference.
		\param value The value as string, will be decoded into the corresponding type.
	*/
	void setValue(const FMIVariable & /*var*/, const std::string & /*value*/) override {}

	/*! Variables names for quantities that are not yet assigned a type (done based on connection). */
	std::vector<std::string>	m_typelessVarNames;
	/*! Units for quantities, to be appended when writing output files as ' [unit]' text. */
	std::vector<std::string>	m_typelessVarUnits;

	/*! After reading the tsv file, this vector holds for each column the identified type.
		During connection of slaves and assigning variable mappings (see MasterSim::composeVariableVector()),
		this vector is filled with types based on the connected FMU slave variable type.
		Each tsv variable must have exactly one type associated, so it is not possible to
		connect a file reader variable to both double and int FMU variable slots.
		When evaluating the different spline data values, these types are used to identify which
		of the m_xxxOutputs vectors to copy the data to and which columns linear interpolation need.
		Any unconnected variable is assumed to be of type double (the vector is initially filled with
		type NUM_VT).
	*/
	std::vector<MASTER_SIM::FMIVariable::VarType>	m_columnVariableTypes;

	/*! For each of the variables holds index of this variable in the respective outputXXX vector.
		For example, if m_columnVariableTypes[5] == VT_DOUBLE, then the value should be stored
		in m_doubleOutputs[ m_columnVariableOutputVectorIndex[5] ].
		For example, if m_columnVariableTypes[2] == VT_INT, then the value should be stored
		in m_intOutputs[ m_columnVariableOutputVectorIndex[2] ].
	*/
	std::vector<unsigned int>						m_columnVariableOutputVectorIndex;

private:

	IBK::CSVReader					*m_fileReader;
	std::vector<IBK::LinearSpline*>	m_valueSplines;
};

} // namespace MASTER_SIM


#endif // MSIM_FILEREADERSLAVE_H
