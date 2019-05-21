#ifndef QPW_CoordinateIndexEditH
#define QPW_CoordinateIndexEditH

#include <QWidget>
#include <QVector>

#include "QPW_ValidatingInputBase.h"

namespace QPW {

namespace Ui {
class CoordinateIndexEdit;
}

/*! \brief CoordinateIndexEdit allows to edit a coordinate vector by value and/or index.
	The ccordinate vector must be set before use. The vector must be sorted.
	The edit widget consists of two combined edit fields.
	One spin edit for the indices and one ValidatingLineEdit for the coordinate values. For properties of this edit field \sa QPW::ValidatingLineEdit .
*/
class CoordinateIndexEdit : public QWidget {
	Q_OBJECT

public:

	enum TimeUnit {
		TU_s,
		TU_min,
		TU_h,
		TU_d,
		TU_y,
		TU_Unknown
	};

	/*! Default c'tor. */
	explicit CoordinateIndexEdit(QWidget *parent = nullptr);

	/*! Standard destructor. Delete the ui.*/
	~CoordinateIndexEdit();

	/*! Initialises the internal coordinate vector.*/
	void set(const QVector<double>& coordinates);

	/*! Returns true if the input is a valid number and the number matches the validation rule.*/
	bool isValid() const;

	/*! Sets a new validator. Class will take ownership of this.
		Any existing validator will be deleted.
		Calls same function from base class ValidatingInputBase. Necessary in order to avoid abiguity with QLineEdit.
		\param validator New validator as derivation of ValidatorBase.
	*/
	void setValidator(ValidatorBase* validator);

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

	/*! Returns the current value of the line edit.
		Returns the last valid number that was entered in the line edit. If the line edit currently contains
		an invalid number, the last number that was accepted is returned.
		\note You should only call this function after isValid() returned true.
	*/
	double value() const;

	/*! Return the current index of the coordinate vector.*/
	int index() const;

	/*! Set the index to the given one and updates coordinate with value from internal vector at given position.
	   \param index Index in coordinate vector.
	*/
	void setIndex(unsigned int index);

	/*! Set time units for base (used in coordinate vector) and display (used in edit field).*/
	void setTimeUnits(TimeUnit base, TimeUnit display);

	/*! Return base time unit used in coordinate vector in case of time coordinates \sa spaceCoordinate.*/
	TimeUnit timeUniteBase() const { return m_timeUnitBase; }

	/*! Return base time unit used in coordinate vector in case of time coordinates \sa spaceCoordinate.*/
	TimeUnit timeUniteDisplay() const { return m_timeUnitDisplay; }

	/*! Set type of coordinates. If true these are are space coordinates otherwise time coordinates.*/
	void setCoordinateType(bool spaceCoordinates);

	/*! Return true if coordinates are space coordinates otherwise these are time coordinates.*/
	bool spaceCoordinate() const { return m_spaceCoordinate; }

	static double timeConvert(TimeUnit base, TimeUnit display, double value);

protected:
	void changeEvent(QEvent *e);

signals:
	/*! Emits the result of the editing, but only if a result was entered correctly. */
	void editingFinishedSuccessfully();

	/*! Emits the result of the editing as index, but only if a result was entered correctly. */
	void valueChanged(int);

private slots:
	/*! Is called then a change in the edit field is finished in order to check the values. */
	void onEditingFinished();

	/*! Is called then index in spin box is changed.*/
	void onIndexChanged(int index);

private:
	Ui::CoordinateIndexEdit *ui;
	QVector<double>	m_coordinates;
	bool			m_ascending;
	int				m_maxCoordinateIndex;
	int				m_minCoordinateIndex;
	bool			m_spaceCoordinate;
	TimeUnit		m_timeUnitBase;
	TimeUnit		m_timeUnitDisplay;

	/*! Return if the internal coordinate vector is valid.*/
	bool			validCoordinates() const;

	/*! Sets a double value as text for the edit field using the current format or formatter.
	   Only internal use. Use setIndex in order to set new values.
		\param value Value to be set in base unit.
		\sa setFormat(), setFormatter(), FormatterBase
	*/
	void setValue(double value);

	double convertBaseToDisplay(double timeBase) const;

	double convertDisplayToBase(double timeDisplay) const;
};


} // namespace QPW
#endif // QPW_CoordinateIndexEditH
