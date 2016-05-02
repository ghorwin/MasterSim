#ifndef MSIM_FMIVARIABLE_H
#define MSIM_FMIVARIABLE_H

#include <string>

class TiXmlElement;

namespace MASTER_SIM {

/*! Wraps an FMU variable and all access to this variable. */
class FMIVariable {
public:
	/*! Variable types. */
	enum VarType {
		VT_BOOL,
		VT_INT,
		VT_STRING,
		VT_DOUBLE
	};

	enum Causality {
		C_OUTPUT,
		C_INPUT,
		C_PARAMETER,
		C_OTHER
	};

	/*! Constructor. */
	FMIVariable() :
		m_type(VT_DOUBLE),
		m_localVectorIdx(0),
		m_globalVectorIdx(0)
	{
	}

	/*! Reads ScalarVariable element. */
	void read(const TiXmlElement * element);


	/*! The variable name. */
	std::string	m_name;

	/*! Description. */
	std::string	m_description;

	Causality	m_causality;

	/*! The variable type. */
	VarType		m_type;

	/*! The value reference. */
	unsigned int m_valueReference;

	/*! Variable index from model description. */
	unsigned int m_varIdx;

	/*! Index in data vector stored locally in each slave. */
	unsigned int m_localVectorIdx;

	/*! Index in global data vector from algorithm. */
	unsigned int m_globalVectorIdx;
};

} // namespace MASTER_SIM

#endif // MSIM_FMIVARIABLE_H
