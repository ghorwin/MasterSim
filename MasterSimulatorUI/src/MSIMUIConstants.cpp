#include  "MSIMUIConstants.h"

#include <QFont>
#include <QFontMetrics>
#include <QTableWidget>
#include <QHeaderView>

const char * const ORG_NAME				= "IBK";
const char * const DOT_FILE_EXTENSION	= ".msim";

#if defined(Q_OS_MAC) // Q_OS_UNIX

const int TABLE_FONT_SIZE = 10;
const char * const FIXED_FONT_FAMILY = "Monaco";

#elif defined(Q_OS_UNIX)

const int TABLE_FONT_SIZE = 10;
const char * const FIXED_FONT_FAMILY = "Monospace";

#else

const int TABLE_FONT_SIZE = 10;
const char * const FIXED_FONT_FAMILY = "Courier New";

#endif

void formatTable(QTableWidget * tab) {
	QFont f;
	f.setPointSize(TABLE_FONT_SIZE);
	tab->setFont(f);
	QFontMetrics fm(f);
	int pixels = fm.lineSpacing()+8;
	tab->verticalHeader()->setDefaultSectionSize(pixels);
}
