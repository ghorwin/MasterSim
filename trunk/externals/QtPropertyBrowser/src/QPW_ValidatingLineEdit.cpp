#include "QPW_ValidatingLineEdit.h"

#include <limits>

#include <QPalette>

#include "QPW_Style.h"

namespace QPW {

ValidatingLineEdit::ValidatingLineEdit(QWidget * parent) :
	QLineEdit(parent),
	m_minVal(-std::numeric_limits<double>::max()),
	m_maxVal(std::numeric_limits<double>::max()),
	m_minValueAllowed(false),
	m_maxValueAllowed(false),
	m_format('a'),
	m_precision(0),
	m_acceptOnlyInteger(false),
	m_allowEmpty(false)
{
	// customize appearance
	QPalette palEdit;
	palEdit.setColor(QPalette::Base, Style::EditFieldBackground);
	setPalette(palEdit);
	setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	connect(this, SIGNAL(textChanged(const QString&)), this, SLOT(onTextChanged(const QString&)));
	connect(this, SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
}


void ValidatingLineEdit::setup(double minVal, double maxVal, const QString &toolTip, bool minValueAllowed, bool maxValueAllowed) {
	m_minVal = minVal;
	m_maxVal = maxVal;
	m_toolTip = toolTip;
	m_minValueAllowed = minValueAllowed;
	m_maxValueAllowed = maxValueAllowed;
}


void ValidatingLineEdit::setValidator(ValidatorBase* validator) {
	m_validator.reset(validator);
}


ValidatorBase* ValidatingLineEdit::validator() const {
	return m_validator.get();
}


void ValidatingLineEdit::setFormatter(FormatterBase *formatter) {
	m_formatter.reset(formatter);
}


FormatterBase* ValidatingLineEdit::formatter() const {
	return m_formatter.get();
}


void ValidatingLineEdit::setFormat(char format, int precision) {
	m_format = format;
	if (m_format != 'e' && m_format != 'E' && m_format != 'f' && m_format != 'g' && m_format != 'G')
		m_format = 'a';
	m_precision = precision;
}


bool ValidatingLineEdit::isValid() const {
	if (m_allowEmpty && text().trimmed().isEmpty())
		return true;
	double val;
	if (!isValidNumber(val))
		return false;

	return isValidImpl(val);
}


bool ValidatingLineEdit::isValidNumber(double & val) const {
	bool ok;
	QString textTemp = text();
	if (m_acceptOnlyInteger) {
		val = QLocale().toInt(textTemp, &ok);
	}
	else {
		val = QLocale().toDouble(textTemp, &ok);
	}
	return ok;
}


void ValidatingLineEdit::setValue(double value) {
	blockSignals(true);

	QString textTemp;
	if (m_formatter.get() != NULL) {
		textTemp = m_formatter->formatted(value);
	}
	else if(m_format != 'a' ) {
		textTemp = QLocale().toString(value, m_format, m_precision);
	}
	else {
		textTemp = QLocale().toString(value);
	}
	QLineEdit::setText(textTemp);

	blockSignals(false);
	onTextChanged(text()); // this will update the value and state of the line edit
}


void ValidatingLineEdit::setText(const QString& str) {
	blockSignals(true);
	QLineEdit::setText(str);
	blockSignals(false);
	onTextChanged(text()); // this will update the value and state of the line edit
}


void ValidatingLineEdit::setEnabled(bool enabled) {
	QLineEdit::setEnabled(enabled);
	onTextChanged(text());
}


void ValidatingLineEdit::setReadOnly(bool readOnly) {
	QLineEdit::setReadOnly(readOnly);
	onTextChanged(text());
}


void ValidatingLineEdit::setEmptyAllowed(bool allowEmpty, const QString & placeholderText) {
	m_allowEmpty = allowEmpty;
	setPlaceholderText(placeholderText);
}

void ValidatingLineEdit::onEditingFinished() {
	if (isValid()) {
		emit editingFinishedSuccessfully();
		emit valueChanged(m_value);
	}
}


void ValidatingLineEdit::onTextChanged ( const QString& ) {
	if (!isEnabled()) {
		QPalette palEdit;
		setPalette(palEdit);
		setToolTip("");
		return;
	}
	if (isReadOnly()) {
		QPalette palEdit;
		palEdit.setColor(QPalette::Base, Style::ReadOnlyEditFieldBackground);
		setPalette(palEdit);
		setToolTip("");
		return;
	}

	if (!isValid()) {
		QPalette palEdit;
		palEdit.setColor(QPalette::Base, Style::ErrorEditFieldBackground);
		setPalette(palEdit);
		if( m_validator.get() != NULL && !m_validator->toolTip().isEmpty()) {
			setToolTip(m_validator->toolTip());
		}
		else {
			setToolTip(m_toolTip);
		}
	}
	else {
		QPalette palEdit;
		palEdit.setColor(QPalette::Base, Style::EditFieldBackground);
		setPalette(palEdit);
		setToolTip("");
	}
}


bool ValidatingLineEdit::isValidImpl(double val) const {
	if (m_validator.get() != NULL) {
		bool res = m_validator->isValid(val);
		if (res)
			m_value = val;
		return res;
	}
	else {
		if (m_minValueAllowed) {
			if (val < m_minVal)
				return false;
		}
		else {
			if (val <= m_minVal)
				return false;
		}
		if (m_maxValueAllowed) {
			if (val > m_maxVal)
				return false;
		}
		else {
			if (val >= m_maxVal)
				return false;
		}
		m_value = val;
		return true;
	}
}


} // namespace QPW

#include "moc_QPW_ValidatingLineEdit.cpp"
