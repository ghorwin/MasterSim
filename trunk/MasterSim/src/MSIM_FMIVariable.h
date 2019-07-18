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
		C_INTERNAL,	/* only CS1 */
		C_OTHER		/* for local (CS1), calculatedParameter (CS2), independent (CS2) */
	};

	/*! Constructor. */
	FMIVariable() :
		m_causality(C_OTHER),
		m_type(VT_DOUBLE)
	{
	}

	/*! Reads ScalarVariable element. */
	void read(const TiXmlElement * element);


	/*! The variable name. */
	std::string	m_name;

	/*! Description. */
	std::string	m_description;

	/*! The causality of the variable - needed for connection logic: input, output or parameter - rest is not important (in GUI or solver). */
	Causality	m_causality;
	/*! The string identifier of the causality, only used for logging/output purposes. */
	std::string m_causalityString;

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

	/*! The variability of this variable. */
	std::string m_variability;

	/*! Creates a one-line variable info text. */
	std::string toString() const;

	/*! Returns a string constant that describes the type. */
	static const char * varType2String(VarType t);
};

} // namespace MASTER_SIM

#endif // MSIM_FMIVARIABLE_H
