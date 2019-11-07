#include  "MSIMUIConstants.h"

#include <QFont>
#include <QFontMetrics>
#include <QTableView>
#include <QTableWidget>
#include <QHeaderView>
#include <QApplication>

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

//void formatTable(QTableWidget * tab) {
//	QFont f;
//	f.setPointSize(TABLE_FONT_SIZE);
//	tab->setFont(f);
//	QFontMetrics fm(f);
//	int pixels = fm.lineSpacing()+8;
//	tab->verticalHeader()->setDefaultSectionSize(pixels);
//}


void formatTable(QTableView * tab, bool caption) {
	QFont f = qApp->font();
	//int ps = f.pointSize();
	//f.setPointSize(ps-1);
	QFontMetrics fm(f);
	int pixels = fm.lineSpacing();
	int additionalGap = pixels*0.2;
#ifdef Q_OS_LINUX
	if (additionalGap < 3)
		additionalGap = 3;
#elif defined(Q_OS_MAC)
	if (additionalGap < 2)
		additionalGap = 2;
#else
	additionalGap = pixels*0.4;
	if (additionalGap < 4)
		additionalGap = 4;
#endif
	if (caption)
		additionalGap *= 2;
	pixels += additionalGap;
	tab->verticalHeader()->setDefaultSectionSize(pixels);
}

