#include "QPW_CoordinateIndexEdit.h"
#include "ui_QPW_CoordinateIndexEdit.h"

#include <algorithm>
#include <stdexcept>
#include <cmath>

namespace QPW {

CoordinateIndexEdit::CoordinateIndexEdit(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::CoordinateIndexEdit),
	m_ascending(true),
	m_maxCoordinateIndex(-1),
	m_minCoordinateIndex(-1),
	m_spaceCoordinate(true),
	m_timeUnitBase(TU_s),
	m_timeUnitDisplay(TU_d)
{
	ui->setupUi(this);
	ui->labelUnit->setText("m");
	connect(ui->lineEdit, SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
	connect(ui->spinBox, SIGNAL(valueChanged(int)), this, SLOT(onIndexChanged(int)));
#ifdef Q_OS_UNIX
	setMaximumHeight(18);
	setMinimumHeight(18);
	ui->lineEdit->setMaximumHeight(20);
	ui->lineEdit->setMinimumHeight(20);
#endif
}


CoordinateIndexEdit::~CoordinateIndexEdit() {
	delete ui;
}


void CoordinateIndexEdit::changeEvent(QEvent *e) {
	QWidget::changeEvent(e);
	switch (e->type()) {
		case QEvent::LanguageChange:
			ui->retranslateUi(this);
			break;
		default:
			break;
	}
}


void CoordinateIndexEdit::set(const QVector<double>& coordinates) {
	m_coordinates = coordinates;
	m_maxCoordinateIndex = -1;
	m_minCoordinateIndex = -1;
	m_ascending = true;
	if(m_coordinates.empty()) {
		setDisabled(true);
		return;
	}
	ui->spinBox->setRange(0, m_coordinates.size()-1);
	if(m_coordinates.size() == 1) {
		m_maxCoordinateIndex = 0;
		m_minCoordinateIndex = 0;
	}
	else {
		m_ascending = m_coordinates[0] < m_coordinates[1];
		// check sorting
		for(QVector<double>::size_type i=2; i<m_coordinates.size(); ++i) {
			if(m_ascending) {
				if(m_coordinates[i-1] >= m_coordinates[i])
					throw std::invalid_argument("Coordinate vector not sorted   [CoordinateIndexEdit::set]");
			}
			else {
				if(m_coordinates[i-1] >= m_coordinates[i])
					throw std::invalid_argument("Coordinate vector not sorted   [CoordinateIndexEdit::set]");
			}
		}
		m_maxCoordinateIndex = m_ascending ? m_coordinates.size()-1 : 0;
		m_minCoordinateIndex = m_ascending ? 0 : m_coordinates.size()-1;
	}
	// initial setting
	blockSignals(true);
	ui->spinBox->setValue(0);
	if(m_spaceCoordinate)
		ui->lineEdit->setValue(m_coordinates[0]);
	else
		ui->lineEdit->setValue(convertBaseToDisplay(m_coordinates[0]));
	blockSignals(false);
}

bool CoordinateIndexEdit::validCoordinates() const {
	if(m_coordinates.empty()) {
		return false;
	}
	if(m_maxCoordinateIndex == -1 || m_minCoordinateIndex == -1)
		return false;

	return true;
}

bool CoordinateIndexEdit::isValid() const {
	if(!validCoordinates())
		return false;

	return ui->lineEdit->isValid();
}


void CoordinateIndexEdit::setValidator(ValidatorBase* validator) {
	ui->lineEdit->setValidator(validator);
}

bool CoordinateIndexEdit::isValidNumber(double & val) const {
	bool res = ui->lineEdit->isValidNumber(val);
	if(!res)
		return false;

	double baseValue = m_spaceCoordinate ? val : convertDisplayToBase(val);

	if(validCoordinates()) {
		if(baseValue > m_coordinates[m_maxCoordinateIndex])
			return false;
		if(baseValue < m_coordinates[m_minCoordinateIndex])
			return false;
	}

	return true;
}

void CoordinateIndexEdit::setEnabled(bool enabled) {
	ui->lineEdit->setEnabled(enabled);
	ui->spinBox->setEnabled(enabled);
}


void CoordinateIndexEdit::setReadOnly(bool readOnly) {
	ui->lineEdit->setReadOnly(readOnly);
	ui->spinBox->setReadOnly(readOnly);
}

double CoordinateIndexEdit::value() const {
	int index = ui->spinBox->value();
	return m_coordinates[index];
}

void CoordinateIndexEdit::setValue(double value) {
	int index = -1;
	// do nothing in case of empty vector
	if(m_coordinates.size() <= 1)
		return;

	// now find index of closest element
	if(m_coordinates.size() == 2) {
		double diff1 = std::abs(m_coordinates[0] - value);
		double diff2 = std::abs(value - m_coordinates[1]);
		index = (diff2 < diff1) ? 1 : 0;
	}
	else {
		if(m_ascending) {
			auto it = std::lower_bound(m_coordinates.begin(), m_coordinates.end(), value);
			// all values in vector are smaller
			if(it == m_coordinates.end())
				index = m_coordinates.size() - 1;
			else {
				index = it - m_coordinates.begin();
			}
			if(index > 0) {
				double diff1 = m_coordinates[index] - value;
				double diff2 = value - m_coordinates[index-1];
				if(diff2 < diff1)
					index = index - 1;
			}
		}
		else {
			auto it = std::lower_bound(m_coordinates.rbegin(), m_coordinates.rend(), value);
			// all values in vector are smaller
			if(it == m_coordinates.rend())
				index = 0;
			else {
				index = m_coordinates.size() - 1 - (it - m_coordinates.rbegin());
			}
			if(index < m_coordinates.size() - 1) {
				double diff1 = m_coordinates[index] - value;
				double diff2 = value - m_coordinates[index+1];
				if(diff2 < diff1)
					index = index + 1;
			}
		}
	}
	setIndex(index);
}


int CoordinateIndexEdit::index() const {
	return ui->spinBox->value();
}


void CoordinateIndexEdit::setIndex(unsigned int index) {
	if(m_coordinates.empty())
		return;

	if(index >= (unsigned int)m_coordinates.size())
		index = m_coordinates.size() - 1;

	blockSignals(true);
	ui->spinBox->setValue(index);
	if(m_spaceCoordinate)
		ui->lineEdit->setValue(m_coordinates[index]);
	else
		ui->lineEdit->setValue(convertBaseToDisplay(m_coordinates[index]));
	blockSignals(false);
	if((int)index != ui->spinBox->value() || ui->lineEdit->text().isEmpty()) {
		emit editingFinishedSuccessfully();
		emit valueChanged(index);
	}
}


void CoordinateIndexEdit::onEditingFinished() {
	if (isValid()) {
		// use setValue function for checking
		double val = ui->lineEdit->value();
		double baseValue = m_spaceCoordinate ? val : convertDisplayToBase(val);
		setValue(baseValue);
	}
}


void CoordinateIndexEdit::onIndexChanged(int index) {
	blockSignals(true);
	if(m_spaceCoordinate)
		ui->lineEdit->setValue(m_coordinates[index]);
	else
		ui->lineEdit->setValue(convertBaseToDisplay(m_coordinates[index]));
	blockSignals(false);
	emit editingFinishedSuccessfully();
	emit valueChanged(index);
}


void CoordinateIndexEdit::setTimeUnits(TimeUnit base, TimeUnit display) {
	if(base != TU_Unknown)
		m_timeUnitBase = base;
	if(m_timeUnitDisplay != display && display != TU_Unknown) {
		m_timeUnitDisplay = display;
		if(!m_spaceCoordinate) {
			QString timeUnit;
			switch(m_timeUnitDisplay) {
				case TU_s: timeUnit = "s"; break;
				case TU_min: timeUnit = "min"; break;
				case TU_h: timeUnit = "h"; break;
				case TU_d: timeUnit = "d"; break;
				case TU_y: timeUnit = "a"; break;
				default: timeUnit = ""; break;
			}
			ui->labelUnit->setText(timeUnit);
		}
	}
}


void CoordinateIndexEdit::setCoordinateType(bool spaceCoordinates) {
	m_spaceCoordinate = spaceCoordinates;
	if(m_spaceCoordinate) {
		ui->labelUnit->setText("m");
	}
	else {
		QString timeUnit;
		switch(m_timeUnitDisplay) {
			case TU_s: timeUnit = "s"; break;
			case TU_min: timeUnit = "min"; break;
			case TU_h: timeUnit = "h"; break;
			case TU_d: timeUnit = "d"; break;
			case TU_y: timeUnit = "a"; break;
			default: timeUnit = ""; break;
		}
		ui->labelUnit->setText(timeUnit);
	}
}


double CoordinateIndexEdit::convertBaseToDisplay(double timeBase) const {
	return timeConvert(m_timeUnitBase, m_timeUnitDisplay, timeBase);
}


double CoordinateIndexEdit::convertDisplayToBase(double timeDisplay) const {
	return timeConvert(m_timeUnitDisplay, m_timeUnitBase, timeDisplay);
}


double CoordinateIndexEdit::timeConvert(TimeUnit base, TimeUnit display, double value) {
	switch(base) {
		case TU_s: {
			switch(display) {
				case TU_s:   return value;
				case TU_min: return value / 60.0;
				case TU_h:   return value / (60.0 * 60.0);
				case TU_d:   return value / (60.0 * 60.0 * 24.0);
				case TU_y:   return value / (60.0 * 60.0 * 24.0 *365.0);
				default:     return value;
			}
		}
		case TU_min: {
			switch(display) {
				case TU_s:   return value * 60.0;
				case TU_min: return value;
				case TU_h:   return value / (60.0);
				case TU_d:   return value / (60.0 * 24.0);
				case TU_y:   return value / (60.0 * 24.0 *365.0);
				default:     return value;
			}
		}
		case TU_h: {
			switch(display) {
				case TU_s:   return value * 60.0 * 60.0;
				case TU_min: return value * 60.0;
				case TU_h:   return value;
				case TU_d:   return value / (24.0);
				case TU_y:   return value / (24.0 *365.0);
				default:     return value;
			}
		}
		case TU_d: {
			switch(display) {
				case TU_s:   return value * 60.0 * 60.0 * 24.0;
				case TU_min: return value * 60.0 * 24.0;
				case TU_h:   return value * 24.0;
				case TU_d:   return value;
				case TU_y:   return value / 365.0;
				default:     return value;
			}
		}
		case TU_y: {
			switch(display) {
				case TU_s:   return value * 60.0 * 60.0 * 24.0 * 365.0;
				case TU_min: return value * 60.0 * 60.0 * 24.0;
				case TU_h:   return value * 60.0 * 60.0;
				case TU_d:   return value * 60.0;
				case TU_y:   return value;
				default:     return value;
			}
		}
		default: return value;
	}
	return value;
}

} // namespace QPW
