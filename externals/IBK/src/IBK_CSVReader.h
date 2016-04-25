#ifndef IBK_CSVReaderH
#define IBK_CSVReaderH

#include <vector>
#include <string>

#include "IBK_Path.h"

namespace IBK {

/*! A class for simplified reading of tab/csv separated values in a column format.
	\todo Speed up reading through use of IBK::FileReader
*/
class CSVReader {
public:
	CSVReader() : m_separationCharacter('\t'), m_nColumns(0), m_nRows(0) {}

	/*! Reads table from a file.
		\param filename Input file name.
	*/
	void read(const IBK::Path & filename);

	/*! Separation character for different tabulator columns. */
	char								m_separationCharacter;
	/*! Tabulator captions: columns of the first line. */
	std::vector<std::string>			m_captions;
	/*! Data values sorted by row and column. */
	std::vector<std::vector<double> >	m_values;
	/*! Number of tabulator columns. */
	unsigned int						m_nColumns;
	/*! Number of tabulator rows. */
	unsigned int						m_nRows;
};

} // namespace IBK

/*! \file IBK_CSVReader.h
	\brief A simple CSV reader class.
*/

#endif // IBK_CSVReaderH
