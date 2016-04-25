#include "IBK_configuration.h"

#include <stdexcept>
#include <iostream>

#include "IBK_Unit.h"
#include "IBK_UnitData.h"
#include "IBK_UnitList.h"
#include "IBK_StringUtils.h"
#include "IBK_Exception.h"
#include "IBK_FormatString.h"

namespace IBK {

Unit::Unit(const std::string& name) {
	const UnitData* unit = UnitList::instance().retrieve(name);
	if (unit==NULL)  throw IBK::Exception( FormatString("Unit '%1' not found.").arg(name),"[Unit::Unit]");
	m_id=unit->id();
#ifdef IBK_ENABLE_UNIT_NAME
	m_name = unit->name();
#endif // IBK_ENABLE_UNIT_NAME
}

Unit::Unit(unsigned int unitid) {
	if (unitid>=UnitList::instance().size())
		throw IBK::Exception( FormatString("Unit index %1 out of range 0..%2.")
			.arg(unitid).arg((int)UnitList::instance().size()-1),"[Unit::Unit]");
	m_id=unitid;
#ifdef IBK_ENABLE_UNIT_NAME
	const UnitData* unit = UnitList::instance().retrieve(unitid);
	if (unit==NULL)  throw IBK::Exception( FormatString("Unit with ID '%1' not found.").arg(unitid),"[Unit::Unit]");
	m_name = unit->name();
#endif // IBK_ENABLE_UNIT_NAME
}

void Unit::set(unsigned int unitid) {
	if (unitid>=UnitList::instance().size())
		throw IBK::Exception( FormatString("Unit index %1 out of range 0..%2.")
			.arg(unitid).arg((int)UnitList::instance().size()-1),"[Unit::set]");
	m_id=unitid;
#ifdef IBK_ENABLE_UNIT_NAME
	m_name = UnitList::instance().retrieve(unitid)->name();
#endif // IBK_ENABLE_UNIT_NAME
}

void  Unit::set(const std::string& name) {
	const UnitData* unit = UnitList::instance().retrieve(name);
	if (unit==NULL)  throw IBK::Exception( FormatString("Unit '%1' not found.").arg(name),"[Unit::set]");
	m_id=unit->id();
#ifdef IBK_ENABLE_UNIT_NAME
	m_name = unit->name();
#endif // IBK_ENABLE_UNIT_NAME
};

const std::string& Unit::name() const {
	return UnitList::instance().retrieve(m_id)->name();
}

unsigned int Unit::base_id() const {
	return UnitList::instance().retrieve(m_id)->base_id();
}

void Unit::relate_to(unsigned int unitid, double& fact, unsigned int& op) {
	if (unitid==m_id) {
		fact=1;
		op=UnitList::OP_MUL;
		return;
	}
	const UnitData* from = UnitList::instance().retrieve( m_id );
	const UnitData* to = UnitList::instance().retrieve( unitid );
	UnitList::instance().relate_units(from, to, fact, op);
}

void Unit::relate_to(const std::string& name, double& fact, unsigned int& op) {
	unsigned int to_id = UnitList::instance().retrieve(name)->id();
	relate_to(to_id, fact, op);
}

void Unit::relate_to(Unit unit, double& fact, unsigned int& op) {
	relate_to(unit.m_id, fact, op);
}

std::istream& operator>>(std::istream& in, Unit& unit) {
	std::string uname;
	in >> uname;
	if (!in) return in;
	unit = Unit(uname);
	return in;
}

std::ostream& operator<<(std::ostream& out, const Unit& unit) {
	return out << unit.name();
}

}  // namespace IBK

