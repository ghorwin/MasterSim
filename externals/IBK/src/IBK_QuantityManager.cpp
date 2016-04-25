#include "IBK_QuantityManager.h"

#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <fstream>

#include "IBK_algorithm.h"
#include "IBK_StringUtils.h"
#include "IBK_FormatString.h"
#include "IBK_Exception.h"
#include "IBK_assert.h"

#include "IBK_messages.h"

namespace IBK {

QuantityManager::QuantityManager() {
}


void QuantityManager::read(const std::string & data) {
	std::stringstream strm(data);
	std::string line;
	while (getline(strm,line)) {
		IBK::trim(line);
		if (line.empty() || line[0] == '#') continue;
		Quantity qd;
		try {
			qd.read(line);
		}
		catch (IBK::Exception & ex) {
			throw IBK::Exception(ex, IBK::FormatString("Cannot read quantity definition from line '%1'.")
				.arg(line), "[QuantityManager::read]");
		}
		// check if we already have such a quantity
		int idx = index(qd);
		if (idx != -1) {
			// replace quantity
			IBK::IBK_Message(IBK::FormatString("Replacing quantity '%1::%2' with new definition.\n")
							 .arg(Quantity::type2string(qd.m_type))
							 .arg(qd.m_name), IBK::MSG_PROGRESS, "QuantityManager::read" ,1);
			m_quantities[idx] = qd;
		}
		else {
			// add quantity to list
			m_quantities.push_back(qd);
			// add new reference to global index map
			m_globalIndexMap[qd.m_name]= m_quantities.size()-1;
		}
	}
}


void QuantityManager::write(std::ostream & out) {
	for (unsigned int i=0; i<m_quantities.size(); ++i)
		m_quantities[i].write(out,0);
}


void QuantityManager::readFromFile(const IBK::Path & fname) {
	const char * const FUNC_ID = "[QuantityManager::readFromFile]";
#ifdef _MSC_VER
	std::ifstream in(fname.wstr().c_str());
#else // _WIN32
	std::ifstream in(fname.str().c_str());
#endif // _WIN32

	if (!in) {
		throw IBK::Exception(IBK::FormatString("Cannot open quantity file '%1'.").arg(fname), FUNC_ID);
	}
	std::stringstream strm;
	strm << in.rdbuf();
	try {
		read(strm.str());
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, IBK::FormatString("Error reading quantity file '%1'.").arg(fname), FUNC_ID);
	}
}


void QuantityManager::writeToFile(const IBK::Path & fname) {
	const char * const FUNC_ID = "[QuantityManager::writeToFile]";
#ifdef _MSC_VER
	std::ofstream out(fname.wstr().c_str());
#else // _WIN32
	std::ofstream out(fname.str().c_str());
#endif // _WIN32
	if (!out) {
		throw IBK::Exception(IBK::FormatString("Cannot open quantity file '%1' for writing.").arg(fname), FUNC_ID);
	}
	write(out);
}


void QuantityManager::clear() {
	m_quantities.clear();
	m_globalIndexMap.clear();
}


int QuantityManager::index(const std::string & quantityName) const {
	std::map<std::string, unsigned int>::const_iterator it = m_globalIndexMap.find(quantityName);
	if (it == m_globalIndexMap.end())
		return -1;
	else
		return it->second;
}


const Quantity & QuantityManager::quantity(unsigned int globalIndex) const {
	if (globalIndex > m_quantities.size())
		throw IBK::Exception( IBK::FormatString("Index %1 out of range (must be < %2).")
			.arg(globalIndex)
			.arg((unsigned int)m_quantities.size()), "[Quantity::quantity]" );
	return m_quantities[globalIndex];
}


std::set<Quantity> QuantityManager::quantitiesOfType(IBK::Quantity::type_t t) const {
	std::set<Quantity> selectedQuantities;
	for (std::vector<Quantity>::const_iterator it = m_quantities.begin(); it != m_quantities.end(); ++it) {
		if (it->m_type == t)
			selectedQuantities.insert(*it);
	}
	return selectedQuantities;
}

//void QuantityManager::setUsedQuantities(const std::vector<unsigned int> & quantityList) {
//	const char * const FUNC_ID = "[QuantityManager::setUsedQuantities]";
//	// update the index maps
//	m_selectedIndexMap.clear();
//	IBK::clear(m_globalIndexForSelectedIndexMap.begin(), m_globalIndexForSelectedIndexMap.end());
//	// now process all quantities
//	for (unsigned int i=0; i<quantityList.size(); ++i) {
//		unsigned int globIndex = quantityList[i];
//		if (globIndex >= m_quantities.size()) {
//			throw IBK::Exception( IBK::FormatString("Invalid index %1 in selected quantity vector (must be < %2).")
//				.arg(globIndex).arg((unsigned int)m_quantities.size()), FUNC_ID);
//		}
//		// determine type
//		Quantity::type_t t = m_quantities[globIndex].m_type;
//		if (t >= Quantity::NUM_TYPES) {
//			throw IBK::Exception( IBK::FormatString("Quantity '%1' does not have a valid type!.")
//				.arg(m_quantities[globIndex].m_name), FUNC_ID);
//		}
//		// insert global index in respective index vector
//		m_globalIndexForSelectedIndexMap[t].push_back(globIndex);
//		// insert selected index into map
//		m_selectedIndexMap[globIndex] = m_globalIndexForSelectedIndexMap[t].size()-1;
//	}
//}


//void QuantityManager::setUsedQuantities(const std::vector<std::pair<Quantity::type_t, std::string> > & quantityList) {
//	// first compute all global indicies
//	std::vector<unsigned int> globIndices;
//	for (unsigned int i=0; i<quantityList.size(); ++i) {
//		int idx = index(quantityList[i].first, quantityList[i].second);
//		if (idx == -1) {
//			throw IBK::Exception( IBK::FormatString("Quantity '%1::%2' unknown in quantity list.")
//								  .arg(quantityList[i].first)
//								  .arg(quantityList[i].second), "[QuantityManager::setUsedQuantities]");
//		}
//		globIndices.push_back(idx);
//	}
//	setUsedQuantities(globIndices);
//}


//int QuantityManager::selectedIndex(unsigned int globalIndex) const {
//	std::map<unsigned int, unsigned int>::const_iterator it = m_selectedIndexMap.find(globalIndex);
//	if (it == m_selectedIndexMap.end())
//		return -1;
//	else
//		return it->second;
//}


//int QuantityManager::selectedIndex(const Quantity::type_t type, const std::string & quantityName) const {
//	// first get global index, then recycle function above
//	int idx = index(type, quantityName);
//	if (idx == -1) {
//		throw IBK::Exception( IBK::FormatString("Quantity '%1::%2' unknown in quantity list.")
//			.arg(Quantity::type2string(type))
//			.arg(quantityName), "[QuantityManager::selectedIndex]");
//	}
//	return selectedIndex(idx);
//}


//const Quantity &QuantityManager::selectedQuantity(Quantity::type_t t, unsigned int selIndex) const {
//	const char * const FUNC_ID = "[QuantityManager::selectedQuantity]";
//	if ((unsigned int)t >= Quantity::NUM_TYPES) {
//		throw IBK::Exception( IBK::FormatString("Invalid quantity type enumeration value %1.").arg((int)t), FUNC_ID);
//	}
//	IBK_ASSERT(m_globalIndexForSelectedIndexMap.size() == Quantity::NUM_TYPES);

//	if (selIndex >= m_globalIndexForSelectedIndexMap[t].size()) {
//		throw IBK::Exception( IBK::FormatString("Invalid quantity index %1 for type '%2'.")
//			.arg(selIndex).arg(Quantity::type2string(t)), FUNC_ID);
//	}
//	// convenience function, first we get the global index, then we return a const reference to the entry
//	unsigned int globIndex = m_globalIndexForSelectedIndexMap[t][selIndex];
//	if (globIndex >= m_quantities.size()) {
//		throw IBK::Exception( IBK::FormatString("Error in selected quantity index -> global index map."), FUNC_ID);
//	}
//	return m_quantities[globIndex];
//}


} // namespace IBK

