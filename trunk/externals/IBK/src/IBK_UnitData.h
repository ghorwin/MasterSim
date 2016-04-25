#ifndef IBK_UnitDataH
#define IBK_UnitDataH

#include <string>
#include <iosfwd>

namespace IBK {

/*! The class UnitData holds the information for a single unit.
	This class is internally used by the UnitList and shouldn't be used directly. Normally, using IBK::Unit
	is sufficient as light-weight identification data member.
	\todo Make this class a private internal class of UnitList.
*/
class UnitData {
  public:
	/*! Default constructor, creates an undefined unit. */
	UnitData() : id_(0), name_("undefined"), base_id_(0), factor_(0), operation_(0)  {}

	/*! Initialisation constructor. */
	UnitData(const unsigned int unitid, const std::string& name,
			  const unsigned int baseunit, double fact, const unsigned int op)
	  :  id_(unitid), name_(name), base_id_(baseunit), factor_(fact), operation_(op) {}

	/*! Returns the ID number of the unit. */
	unsigned int  id()          const     { return id_; }
	/*! Returns the unit string (or name). */
	const std::string&  name()        const     { return name_; }
	/*! Returns the ID of the appropriate base unit. */
	unsigned int  base_id()     const     { return base_id_; }
	/*! Returns the factor or summand for conversion into the base unit. */
	double        factor()      const     { return factor_; }
	/*! Returns the operation for converting the unit into the base unit. */
	unsigned int  operation()   const     { return operation_; }

  private:
	unsigned int id_;           ///< ID number.
	std::string  name_;         ///< String constant, unit name.
	unsigned int base_id_;      ///< Assigned base unit it.
	double       factor_;       ///< Conversion factor/summand.
	unsigned int operation_;    ///< Conversion operation.

	/*! Returns true, if two units are equal (have the same ID number). */
	friend bool operator==(const UnitData& lhs, const UnitData& rhs);
	/*! Returns true, if two units are not equal (do not have the same ID number). */
	friend bool operator!=(const UnitData& lhs, const UnitData& rhs);
}; // class UnitData

inline bool operator==(const UnitData& lhs, const UnitData& rhs) {
						return (lhs.id_==rhs.id_); }
inline bool operator!=(const UnitData& lhs, const UnitData& rhs) {
						return (lhs.id_!=rhs.id_); }

}  // namespace IBK

/*! \file IBK_UnitData.h
	\brief Contains declaration of class UnitData.
*/

#endif // IBK_UnitDataH
