#ifndef MSIM_FMIVARIABLE_H
#define MSIM_FMIVARIABLE_H

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

	/*! Constructor. */
	FMIVariable() :
		m_type(VT_DOUBLE),
		m_localVectorIdx(0),
		m_globalVectorIdx(0)
	{
	}

	/*! The variable type. */
	VarType	m_type;

	/*! Index in data vector stored locally in each slave. */
	unsigned int m_localVectorIdx;
	/*! Index in global data vector from algorithm. */
	unsigned int m_globalVectorIdx;
};

} // namespace MASTER_SIM

#endif // MSIM_FMIVARIABLE_H
