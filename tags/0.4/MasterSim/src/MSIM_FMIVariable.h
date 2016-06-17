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
		m_type(VT_DOUBLE)
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

	/*! Variable index from model description (1-based). */
	unsigned int m_varIdx;

	/*! Start value for string input variables/parameters. */
	std::string m_startValue;

	/*! Unit of this quantity, if empty - undefined (only type Real). */
	std::string m_unit;

	/*! Type name of type definition for this quantity (only type Real). */
	std::string m_declaredType;


	/*! Returns a string constant that describes the type. */
	static const char * varType2String(VarType t);
};

} // namespace MASTER_SIM

#endif // MSIM_FMIVARIABLE_H
