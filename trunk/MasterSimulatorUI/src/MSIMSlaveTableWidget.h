#ifndef MSIMSlaveTableWidgetH
#define MSIMSlaveTableWidgetH

#include <QTableWidget>

/*! The table widget in the slaves view that shows the simulators/slaves.
	Subclassed to provide custom resize operation.
*/
class MSIMSlaveTableWidget : public QTableWidget {
	Q_OBJECT
public:
	MSIMSlaveTableWidget(QWidget * parent);

protected:
	void resizeEvent(QResizeEvent *event) override;
};

#endif // MSIMSlaveTableWidgetH
