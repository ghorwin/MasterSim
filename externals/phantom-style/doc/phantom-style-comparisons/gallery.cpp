// Widget gallery for comparing QStyle variants.
// Usage: gallery <light|dark> <output-dir>
// Renders each section of the gallery into <output-dir>/<section>.png

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDateEdit>
#include <QDir>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QPainter>
#include <QPlainTextEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QRadioButton>
#include <QScrollBar>
#include <QSlider>
#include <QSpinBox>
#include <QStyleFactory>
#include <QTabWidget>
#include <QTableWidget>
#include <QThread>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>
#include <QTreeWidget>
#include <QVBoxLayout>

#ifndef USE_FUSION
#include "phantomstyle.h"
#endif

static QIcon makeIcon(QColor c, QChar ch) {
	QPixmap pm(64, 64);
	pm.fill(Qt::transparent);
	QPainter p(&pm);
	p.setRenderHint(QPainter::Antialiasing);
	p.setBrush(c);
	p.setPen(Qt::NoPen);
	p.drawRoundedRect(QRectF(4, 4, 56, 56), 12, 12);
	p.setPen(Qt::white);
	QFont f; f.setPixelSize(36); f.setBold(true);
	p.setFont(f);
	p.drawText(QRectF(0, 0, 64, 64), Qt::AlignCenter, QString(ch));
	return QIcon(pm);
}

static QPalette darkPalette() {
	QPalette p;
	const QColor window(53, 53, 53), base(35, 35, 35), text(224, 224, 224), disabled(127, 127, 127);
	p.setColor(QPalette::Window, window);
	p.setColor(QPalette::WindowText, text);
	p.setColor(QPalette::Base, base);
	p.setColor(QPalette::AlternateBase, QColor(45, 45, 45));
	p.setColor(QPalette::ToolTipBase, window);
	p.setColor(QPalette::ToolTipText, text);
	p.setColor(QPalette::PlaceholderText, disabled);
	p.setColor(QPalette::Text, text);
	p.setColor(QPalette::Button, QColor(60, 60, 60));
	p.setColor(QPalette::ButtonText, text);
	p.setColor(QPalette::BrightText, Qt::red);
	p.setColor(QPalette::Light, QColor(80, 80, 80));
	p.setColor(QPalette::Midlight, QColor(70, 70, 70));
	p.setColor(QPalette::Mid, QColor(40, 40, 40));
	p.setColor(QPalette::Dark, QColor(30, 30, 30));
	p.setColor(QPalette::Shadow, QColor(15, 15, 15));
	p.setColor(QPalette::Link, QColor(42, 130, 218));
	p.setColor(QPalette::Highlight, QColor(42, 130, 218));
	p.setColor(QPalette::HighlightedText, Qt::white);
	p.setColor(QPalette::Disabled, QPalette::Text, disabled);
	p.setColor(QPalette::Disabled, QPalette::WindowText, disabled);
	p.setColor(QPalette::Disabled, QPalette::ButtonText, disabled);
	p.setColor(QPalette::Disabled, QPalette::Highlight, QColor(80, 80, 80));
	p.setColor(QPalette::Disabled, QPalette::HighlightedText, disabled);
	return p;
}

static QPalette lightPalette() {
	// Fusion's standard palette - a neutral light palette independent of the desktop
	std::unique_ptr<QStyle> fusion(QStyleFactory::create("Fusion"));
	QPalette p = fusion->standardPalette();
	p.setColor(QPalette::Highlight, QColor(48, 140, 198));
	return p;
}

static QWidget * section(QWidget * parent, const QString & name) {
	QWidget * w = new QWidget(parent);
	w->setObjectName(name);
	w->setFixedWidth(460);
	return w;
}

int main(int argc, char * argv[]) {
	if (argc < 3) {
		qWarning("Usage: gallery <light|dark> <output-dir>");
		return 1;
	}
	QApplication app(argc, argv);
#ifdef USE_FUSION
	QApplication::setStyle(QStyleFactory::create("Fusion"));
#else
	QApplication::setStyle(new PhantomStyle);
#endif
	QFont f("Noto Sans");
	f.setPointSize(9);
	app.setFont(f);
	const bool dark = QString(argv[1]) == "dark";
	QApplication::setPalette(dark ? darkPalette() : lightPalette());
	QString outDir = argv[2];
	QDir().mkpath(outDir);

	QMainWindow mw;
	QWidget * central = new QWidget;
	QVBoxLayout * root = new QVBoxLayout(central);
	root->setSpacing(0);
	root->setContentsMargins(0, 0, 0, 0);
	mw.setCentralWidget(central);

	QIcon iconOpen = makeIcon(QColor(52, 152, 219), 'O');
	QIcon iconSave = makeIcon(QColor(39, 174, 96), 'S');
	QIcon iconRun  = makeIcon(QColor(230, 126, 34), 'R');
	QIcon iconStop = makeIcon(QColor(192, 57, 43), 'X');

	// --- menu bar + tool bar ---
	QWidget * secBars = section(central, "menubar_toolbar");
	{
		QVBoxLayout * l = new QVBoxLayout(secBars);
		l->setContentsMargins(0, 0, 0, 8);
		l->setSpacing(0);
		QMenuBar * mb = new QMenuBar;
		mb->addMenu("&File");
		mb->addMenu("&Edit");
		mb->addMenu("&View");
		mb->addMenu("&Simulation");
		mb->addMenu("&Help");
		l->addWidget(mb);
		QToolBar * tb = new QToolBar;
		tb->setIconSize(QSize(20, 20));
		tb->addAction(iconOpen, "Open");
		tb->addAction(iconSave, "Save");
		tb->addSeparator();
		QAction * a = tb->addAction(iconRun, "Run");
		a->setCheckable(true);
		a->setChecked(true);
		QAction * s = tb->addAction(iconStop, "Stop");
		s->setEnabled(false);
		tb->addSeparator();
		QToolButton * tbtn = new QToolButton;
		tbtn->setText("Mode");
		tbtn->setIcon(iconOpen);
		tbtn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
		tbtn->setPopupMode(QToolButton::MenuButtonPopup);
		tbtn->setMenu(new QMenu(tbtn));
		tb->addWidget(tbtn);
		l->addWidget(tb);
	}
	root->addWidget(secBars);

	// --- buttons ---
	QWidget * secButtons = section(central, "buttons");
	{
		QVBoxLayout * l = new QVBoxLayout(secButtons);
		QHBoxLayout * row1 = new QHBoxLayout;
		QPushButton * def = new QPushButton("Default");
		def->setDefault(true);
		row1->addWidget(def);
		row1->addWidget(new QPushButton("Normal"));
		QPushButton * pressed = new QPushButton("Pressed");
		pressed->setDown(true);
		row1->addWidget(pressed);
		QPushButton * tog = new QPushButton("Toggled");
		tog->setCheckable(true);
		tog->setChecked(true);
		row1->addWidget(tog);
		QPushButton * dis = new QPushButton("Disabled");
		dis->setEnabled(false);
		row1->addWidget(dis);
		l->addLayout(row1);

		QHBoxLayout * row2 = new QHBoxLayout;
		QPushButton * ib = new QPushButton(iconSave, "With icon");
		row2->addWidget(ib);
		QPushButton * mbtn = new QPushButton("Menu");
		mbtn->setMenu(new QMenu(mbtn));
		row2->addWidget(mbtn);
		QToolButton * t1 = new QToolButton; t1->setIcon(iconOpen); t1->setAutoRaise(false);
		QToolButton * t2 = new QToolButton; t2->setIcon(iconRun); t2->setCheckable(true); t2->setChecked(true);
		QToolButton * t3 = new QToolButton; t3->setArrowType(Qt::DownArrow);
		QToolButton * t4 = new QToolButton; t4->setText("...");
		row2->addWidget(t1); row2->addWidget(t2); row2->addWidget(t3); row2->addWidget(t4);
		row2->addStretch();
		l->addLayout(row2);

		QHBoxLayout * row3 = new QHBoxLayout;
		QCheckBox * c1 = new QCheckBox("Off");
		QCheckBox * c2 = new QCheckBox("On"); c2->setChecked(true);
		QCheckBox * c3 = new QCheckBox("Partial"); c3->setTristate(true); c3->setCheckState(Qt::PartiallyChecked);
		QCheckBox * c4 = new QCheckBox("Disabled"); c4->setChecked(true); c4->setEnabled(false);
		row3->addWidget(c1); row3->addWidget(c2); row3->addWidget(c3); row3->addWidget(c4);
		row3->addStretch();
		l->addLayout(row3);

		QHBoxLayout * row4 = new QHBoxLayout;
		QRadioButton * r1 = new QRadioButton("Gauss-Jacobi"); r1->setChecked(true);
		QRadioButton * r2 = new QRadioButton("Gauss-Seidel");
		QRadioButton * r3 = new QRadioButton("Newton"); r3->setEnabled(false);
		row4->addWidget(r1); row4->addWidget(r2); row4->addWidget(r3);
		row4->addStretch();
		l->addLayout(row4);
	}
	root->addWidget(secButtons);

	// --- input widgets ---
	QWidget * secInputs = section(central, "inputs");
	{
		QGridLayout * g = new QGridLayout(secInputs);
		g->addWidget(new QLabel("Line edit:"), 0, 0);
		g->addWidget(new QLineEdit("Math003_GaussSeidel.msim"), 0, 1);
		QLineEdit * ph = new QLineEdit; ph->setPlaceholderText("Placeholder text");
		g->addWidget(ph, 0, 2);
		g->addWidget(new QLabel("Combo box:"), 1, 0);
		QComboBox * cb = new QComboBox; cb->addItems({"Gauss-Jacobi", "Gauss-Seidel", "Newton"});
		g->addWidget(cb, 1, 1);
		QComboBox * ecb = new QComboBox; ecb->setEditable(true); ecb->addItems({"Editable", "B"});
		g->addWidget(ecb, 1, 2);
		g->addWidget(new QLabel("Spin boxes:"), 2, 0);
		QSpinBox * sb = new QSpinBox; sb->setRange(0, 1000); sb->setValue(42); sb->setSuffix(" iters");
		g->addWidget(sb, 2, 1);
		QDoubleSpinBox * dsb = new QDoubleSpinBox; dsb->setValue(0.25); dsb->setSuffix(" s");
		g->addWidget(dsb, 2, 2);
		g->addWidget(new QLabel("Disabled:"), 3, 0);
		QLineEdit * dle = new QLineEdit("read only / disabled"); dle->setEnabled(false);
		g->addWidget(dle, 3, 1);
		QComboBox * dcb = new QComboBox; dcb->addItem("Disabled combo"); dcb->setEnabled(false);
		g->addWidget(dcb, 3, 2);
	}
	root->addWidget(secInputs);

	// --- sliders, progress, scrollbars ---
	QWidget * secRanges = section(central, "sliders_progress");
	QProgressBar * busy = nullptr;
	{
		QGridLayout * g = new QGridLayout(secRanges);
		QSlider * s1 = new QSlider(Qt::Horizontal); s1->setValue(35);
		QSlider * s2 = new QSlider(Qt::Horizontal); s2->setValue(70);
		s2->setTickPosition(QSlider::TicksBelow); s2->setTickInterval(10);
		g->addWidget(s1, 0, 0);
		g->addWidget(s2, 1, 0);
		QProgressBar * p1 = new QProgressBar; p1->setValue(64);
		g->addWidget(p1, 2, 0);
		busy = new QProgressBar; busy->setRange(0, 0);
		g->addWidget(busy, 3, 0);
		QScrollBar * hs = new QScrollBar(Qt::Horizontal); hs->setRange(0, 100); hs->setPageStep(30); hs->setValue(30);
		g->addWidget(hs, 4, 0);
		QSlider * vs = new QSlider(Qt::Vertical); vs->setValue(60);
		g->addWidget(vs, 0, 1, 5, 1);
		QScrollBar * vsb = new QScrollBar(Qt::Vertical); vsb->setRange(0, 100); vsb->setPageStep(20); vsb->setValue(50);
		g->addWidget(vsb, 0, 2, 5, 1);
	}
	root->addWidget(secRanges);

	// --- tabs + group boxes ---
	QWidget * secTabs = section(central, "tabs_groupbox");
	{
		QVBoxLayout * l = new QVBoxLayout(secTabs);
		QTabWidget * tw = new QTabWidget;
		QWidget * page = new QWidget;
		QHBoxLayout * pl = new QHBoxLayout(page);
		QGroupBox * gb1 = new QGroupBox("Error control");
		QVBoxLayout * gl1 = new QVBoxLayout(gb1);
		gl1->addWidget(new QCheckBox("Adaptive steps"));
		gl1->addWidget(new QLineEdit("1e-5"));
		QGroupBox * gb2 = new QGroupBox("Output (checkable)");
		gb2->setCheckable(true);
		QVBoxLayout * gl2 = new QVBoxLayout(gb2);
		gl2->addWidget(new QRadioButton("Binary"));
		gl2->addWidget(new QRadioButton("Text"));
		pl->addWidget(gb1); pl->addWidget(gb2);
		tw->addTab(page, iconOpen, "Slaves");
		tw->addTab(new QWidget, "Connections");
		tw->addTab(new QWidget, "Simulation");
		tw->setTabEnabled(2, false);
		l->addWidget(tw);
	}
	root->addWidget(secTabs);

	// --- group box variants ---
	QWidget * secGroups = section(central, "groupboxes");
	{
		QGridLayout * g = new QGridLayout(secGroups);
		// fills a group box with a check box and a spin box, so that the (disabled) state of children is visible
		auto fill = [](QGroupBox * gb) {
			QVBoxLayout * gl = new QVBoxLayout(gb);
			QCheckBox * c = new QCheckBox("Write log file");
			c->setChecked(true);
			gl->addWidget(c);
			QSpinBox * s = new QSpinBox; s->setValue(10); s->setSuffix(" steps");
			gl->addWidget(s);
			return gb;
		};
		QGroupBox * gbNormal = fill(new QGroupBox("Normal"));
		QGroupBox * gbFlat = fill(new QGroupBox("Flat"));
		gbFlat->setFlat(true);
		QGroupBox * gbOn = fill(new QGroupBox("Checkable, checked"));
		gbOn->setCheckable(true);
		gbOn->setChecked(true);
		QGroupBox * gbOff = fill(new QGroupBox("Checkable, unchecked"));
		gbOff->setCheckable(true);
		gbOff->setChecked(false);
		QGroupBox * gbDis = fill(new QGroupBox("Disabled"));
		gbDis->setEnabled(false);
		QGroupBox * gbNoTitle = fill(new QGroupBox);
		g->addWidget(gbNormal, 0, 0);
		g->addWidget(gbFlat, 0, 1);
		g->addWidget(gbOn, 1, 0);
		g->addWidget(gbOff, 1, 1);
		g->addWidget(gbDis, 2, 0);
		g->addWidget(gbNoTitle, 2, 1);

		// nested group boxes with a centered title
		QGroupBox * gbOuter = new QGroupBox("Outer (centered title)");
		gbOuter->setAlignment(Qt::AlignHCenter);
		QHBoxLayout * ol = new QHBoxLayout(gbOuter);
		ol->addWidget(fill(new QGroupBox("Inner")));
		QGroupBox * gbInnerChk = fill(new QGroupBox("Inner, checkable"));
		gbInnerChk->setCheckable(true);
		ol->addWidget(gbInnerChk);
		g->addWidget(gbOuter, 3, 0, 1, 2);
	}
	root->addWidget(secGroups);

	// --- item views ---
	QWidget * secViews = section(central, "itemviews");
	{
		QVBoxLayout * l = new QVBoxLayout(secViews);
		QTreeWidget * tree = new QTreeWidget;
		tree->setHeaderLabels({"Variable", "Type", "Causality"});
		tree->setAlternatingRowColors(true);
		const char * slaves[] = {"Part1", "Part2", "Part3"};
		for (int i = 0; i < 3; ++i) {
			QTreeWidgetItem * it = new QTreeWidgetItem(tree, {slaves[i], "FMU 2.0", ""});
			it->setIcon(0, i == 0 ? iconOpen : (i == 1 ? iconSave : iconRun));
			for (int j = 0; j < 3; ++j) {
				QTreeWidgetItem * c = new QTreeWidgetItem(it, {QString("x%1").arg(j), "Real", j == 0 ? "input" : "output"});
				c->setCheckState(0, j == 1 ? Qt::Checked : Qt::Unchecked);
			}
		}
		tree->topLevelItem(0)->setExpanded(true);
		tree->topLevelItem(2)->setExpanded(false);
		tree->setCurrentItem(tree->topLevelItem(0)->child(1));
		tree->setFixedHeight(170);
		tree->header()->resizeSection(0, 180);
		l->addWidget(tree);

		QTableWidget * table = new QTableWidget(4, 3);
		table->setHorizontalHeaderLabels({"Output", "Input", "Offset"});
		const char * rows[][3] = {{"Part1.y1", "Part2.u1", "0"}, {"Part2.y1", "Part3.u1", "0"},
								  {"Part3.y1", "Part1.u1", "1.5"}, {"Part3.y2", "Part2.u2", "0"}};
		for (int r = 0; r < 4; ++r)
			for (int c = 0; c < 3; ++c)
				table->setItem(r, c, new QTableWidgetItem(rows[r][c]));
		table->setSortingEnabled(true);
		table->sortByColumn(0, Qt::AscendingOrder);
		table->selectRow(1);
		table->setAlternatingRowColors(true);
		table->horizontalHeader()->setStretchLastSection(true);
		table->setFixedHeight(150);
		l->addWidget(table);
	}
	root->addWidget(secViews);

	mw.show();
	mw.activateWindow();
	mw.raise();
	for (int i = 0; i < 10; ++i) {
		QApplication::processEvents();
		QThread::msleep(20);
	}

	const QStringList names = {"menubar_toolbar", "buttons", "inputs", "sliders_progress", "tabs_groupbox", "groupboxes", "itemviews"};
	for (const QString & n : names) {
		QWidget * w = central->findChild<QWidget *>(n);
		w->grab().save(outDir + "/" + n + ".png");
	}

	// --- popup menu with highlighted item ---
	QMenu menu;
	menu.addAction(iconOpen, "Open project...", QKeySequence("Ctrl+O"));
	menu.addAction(iconSave, "Save project", QKeySequence("Ctrl+S"));
	QAction * chk = menu.addAction("Show grid");
	chk->setCheckable(true); chk->setChecked(true);
	QAction * chkIcon = menu.addAction(iconRun, "Auto-run (checked + icon)");
	chkIcon->setCheckable(true); chkIcon->setChecked(true);
	menu.addSeparator();
	QMenu * sub = menu.addMenu("Recent projects");
	sub->addAction("Math003.msim");
	QAction * dis = menu.addAction(iconStop, "Disabled entry");
	dis->setEnabled(false);
	menu.addSeparator();
	QAction * hl = menu.addAction("Highlighted entry", QKeySequence("F5"));
	menu.popup(QPoint(0, 0));
	menu.setActiveAction(hl);
	for (int i = 0; i < 10; ++i) {
		QApplication::processEvents();
		QThread::msleep(20);
	}
	menu.grab().save(outDir + "/menu.png");
	return 0;
}
