#include "MSIMBrowseFilenameWidget.h"

#include <QLineEdit>
#include <QToolButton>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QFileInfo>

MSIMBrowseFilenameWidget::MSIMBrowseFilenameWidget(QWidget *parent) :
	QWidget(parent),
	m_filenameMode(true),
	m_fileMustExist(true)
{
	m_lineEdit = new QLineEdit(this);
	m_toolBtn = new QToolButton(this);
	m_toolBtn->setText("...");
	QHBoxLayout * lay = new QHBoxLayout;
	lay->addWidget(m_lineEdit);
	lay->addWidget(m_toolBtn);
	setLayout(lay);
	lay->setMargin(0);

	connect(m_toolBtn, SIGNAL(clicked()), this, SLOT(onToolBtnClicked()));
	connect(m_lineEdit, SIGNAL(editingFinished()), this, SIGNAL(editingFinished()));
}


void MSIMBrowseFilenameWidget::setup(const QString &filename, bool filenameMode, bool fileMustExist, const QString & filter) {
	m_filenameMode = filenameMode;
	m_fileMustExist = fileMustExist;
	m_filter = filter;
	m_lineEdit->setText(filename);
}


void MSIMBrowseFilenameWidget::setFilename(const QString & filename) {
	m_lineEdit->setText(filename);
}


QString MSIMBrowseFilenameWidget::filename() const {
	return m_lineEdit->text();
}


void MSIMBrowseFilenameWidget::onToolBtnClicked() {
	QString fn;
	if (m_filenameMode) {
		if (m_fileMustExist) {
			fn = QFileDialog::getOpenFileName(this, tr("Select filename"), filename(), m_filter);
		}
		else {
			fn = QFileDialog::getSaveFileName(this, tr("Select filename"), filename(), m_filter);
		}
	}
	else {
		if (m_fileMustExist) {
			fn = QFileDialog::getExistingDirectory(this, tr("Select filename"), filename());
		}
		else {
			fn = QFileDialog::getSaveFileName(this, tr("Select directory"), filename(), m_filter);
		}
	}
	if (!fn.isEmpty()) {
		m_lineEdit->setText(fn);
		emit editingFinished();
	}
}
