#ifndef MSIM_FMUSLAVE_H
#define MSIM_FMUSLAVE_H

#include <string>

#include "fmi/fmi2Functions.h"

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

private:
	/*! Pointer to the FMU object that instantiated this slave. */
	FMU			*m_fmu;

	/*! Simulator/slave ID name. */
	std::string	m_name;

	/*! Component pointer returned by instantiation function of FMU. */
	void		*m_component;

	/*! Structure with function pointers to required call back functions. */
	static	fmi2CallbackFunctions	m_fmi2CallBackFunctions;
};

} // namespace MASTER_SIM


#endif // MSIM_FMUSLAVE_H
