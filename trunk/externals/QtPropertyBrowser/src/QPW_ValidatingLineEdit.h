#ifndef QtExt_ValidatingLineEditH
#define QtExt_ValidatingLineEditH

#include <memory>

#include <QLineEdit>
#include <QString>
#include <QLocale>

#include "QPW_ValidatingInputBase.h"

namespace QPW {

/*!	\brief Class ValidatingLineEdit provides validation functionality for entered double numbers and
			changes color of line edit when invalid content is entered.

	This line edit checks the numeric input for valid values and colors the line if an error occurs. In case
	of invalid content a tooltip is set that shows why the input is invalid.

	Currently set QLocale() will be used for converting numbers to/from the line edit.

	\code
	// ** Setup **
	// 1. First set limits of the acceptable value range, the last two arguments are optional and
	//	  can be used to specify whether the min and max values themselves are part of the acceptable range.
	lineEdit->setup(minVal, maxVal, toolTip, false, true);

	// 2. Connect to the editingFinishedSuccessfully() signal, which is only emitted when
	// the user has changed the line edit and/or the combo box and the result is a valid entry
	// with respect to the min/max values.
	connect(lineEdit, SIGNAL(editingFinishedSuccessfully()), this SLOT(onEditingFinishedSuccessfully()));
	\endcode

	The following code illustrates the usual work flow with the line edit.
	\code
	// Set any value either by using
	lineEdit->setValue(15);

	// or by setting the value directly via setText()
	lineEdit->setText(QString("%L1").arg(15, 0, 'f', 2));

	// when user has finished, the value can be retrieved
	if (lineEdit->isValid()) {
		double value = lineEdit->value();
		// ...
	}
	\endcode
*/
class ValidatingLineEdit : public QLineEdit {
	Q_OBJECT
public:
	/*! Default c'tor. */
	explicit ValidatingLineEdit(QWidget * parent = 0);

	/*! Set the range for validating and the tool tip string.
		\param minVal Minimum Value.
		\param maxVal Maximum Value.
		\param toolTip Tool tip which is shown if the value isn't valid.
		\param minValueAllowed Minimum value itself is part of accepted value range.
		\param maxValueAllowed Maximum value itself is part of accepted value range.
	*/
	void setup(double minVal, double maxVal, const QString & toolTip, bool minValueAllowed = false, bool maxValueAllowed = false);

	/*! Sets a new validator. Class will take ownership of this.
		Any existing validator will be deleted.
		\param validator New validator as derivation of ValidatorBase.
	*/
	void setValidator(ValidatorBase* validator);

	/*! Returns the current validator or NULL if no validator exist.*/
	ValidatorBase* validator() const;

	/*! Set the internal format string that is used by setValue().
		\param format specifies the number format. It can be:
			\li e format as [-]9.9e[+|-]999
			\li E format as [-]9.9E[+|-]999
			\li f format as [-]9.9
			\li g use e or f format, whichever is the most concise
			\li G use E or f format, whichever is the most concise
			\li a no format (default formatting)

		\param precision specifies for 'e', 'E' and 'f' the number of digits after decimal point
				and for 'g' and 'G' the maximum number of significant digits, if format 'a' is set, precision has no meaning.
		Format will be applied in next call to setValue().
		\note Users may still enter text in their own fashion, user input is not corrected to match this format.
	*/
	void setFormat(char format, int precision);

	/*! Sets a new formatter which will be used instead of built-in formatting rules in calls to setValue().
		Line edit will take ownership of this. Previously assigned formatter will be deleted.
		\param formatter New validator as derivation of ValidatorBase.
	*/
	void setFormatter(FormatterBase* formatter);

	/*! Returns the current formater or NULL if no formater exist.*/
	FormatterBase* formatter() const;


	/*! Returns true if the input is a valid number and the number matches the validation rule.*/
	bool isValid() const;

	/*! Checks if the line edit contains a valid double number (regardless of range test).
		\param val The value is stored herein if the line edit's text can be parsed into a number.
		\return Returns true, if the line edit's text can be parsed into a number.
	*/
	bool isValidNumber(double & val) const;

	/*! Set the enabled state (overloaded to change background color).
		\param enabled Enabled state of widget.
	*/
	void setEnabled(bool enabled);

	/*! Set whether line edit is read only (overloaded to change background color).
		\param readOnly Read-only state of widget.
	*/
	void setReadOnly(bool readOnly);

	/*! Configure the line edit to only accept integers (if argument is true).
		The test of integer is done prior to checking ranges or calling the validator.
	*/
	void setAcceptOnlyInteger(bool acceptOnlyInteger) { m_acceptOnlyInteger = acceptOnlyInteger; }

	/*! Configures line edit to allow an empty field.
		\param allowEmpty If this is set to true, the line edit will be valid even if the line edit is empty.
		\note When retrieving values from such an input, first check for empty line before
		calling value().
		\code
		if (!lineEdit->text().trimmed().isEmpty())
			val = lineEdit->value();
		\endcode
		The placeholder text can alternatively be set/changed with a call to setPlaceholderText();
	*/
	void setEmptyAllowed(bool allowEmpty, const QString & placeholderText);

	/*! Sets a double value as text for the edit field using the current format or formatter.
		\param value Value to be set.
		\sa setFormat(), setFormatter(), FormatterBase
	*/
	void setValue(double value);

	/*! Returns the current value of the line edit.
		Returns the last valid number that was entered in the line edit. If the line edit currently contains
		an invalid number, the last number that was accepted is returned.
		\note You should only call this function after isValid() returned true.
	*/
	double value() const { return m_value; }

	/*! Make setText function private in order to prevent setting text outside setValue.
		Function will be used internally.
	*/
	void setText(const QString& text);

	/*! Performs a new validating check without changing the entry.
		Useful for special validators with dependent edits.
	*/
	void check() {
		QString t = text();
		setText(t);
	}


signals:
	/*! Emits the result of the editing, but only if a result was entered correctly. */
	void editingFinishedSuccessfully();

	/*! Emits the result of the editing, but only if a result was entered correctly. */
	void valueChanged(double);

private slots:
	/*! Evaluates input and colors line edit accordingly. */
	void onEditingFinished();

	/*! Evaluates input and colors line edit accordingly. */
	void onTextChanged ( const QString & text );

private:

	// *** PRIVATE MEMBER FUNCTIONS ***

	/*! Test function for the given value.
		It returns true if the given value fits for all tests given in the current instance.
		\param value Value to be set.
	*/
	bool isValidImpl(double val) const;

	// *** PRIVATE MEMBER VARIABLES ***

#if __cplusplus <= 199711L
	/*! Holds validator object if provided. */
	std::auto_ptr<ValidatorBase>	m_validator;
	/*! Holds formatter object if provided. */
	std::auto_ptr<FormatterBase>	m_formatter;
#else
	/*! Holds validator object if provided. */
	std::unique_ptr<ValidatorBase>	m_validator;
	/*! Holds formatter object if provided. */
	std::unique_ptr<FormatterBase>	m_formatter;
#endif //__cplusplus <= 199711L
	/*! Lower limit of values that can be entered. */
	double							m_minVal;
	/*! Upper limit of values that can be entered. */
	double							m_maxVal;
	/*! Tool tip to be shown when invalid input is entered. */
	QString							m_toolTip;
	/*! If true, the lower limit is included in the range. */
	bool							m_minValueAllowed;
	/*! If true, the upper limit is included in the range. */
	bool							m_maxValueAllowed;
	/*! Standard number format if no formatter is provided. */
	char							m_format;
	/*! Standard precision if no formatter is provided. */
	int								m_precision;
	/*! If true, only integer values are accepted. */
	bool							m_acceptOnlyInteger;
	/*! If true, line edit may be empty and still valid (default is false). */
	bool							m_allowEmpty;

	/*! Caches value that was last accepted, updated in isValid() (therefore mutable). */
	mutable double					m_value;
};

} // namespace QPW

#endif // QPW_ValidatingLineEditH
