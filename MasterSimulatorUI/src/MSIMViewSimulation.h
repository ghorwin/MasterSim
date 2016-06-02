#ifndef MSIMVIEWSIMULATION_H
#define MSIMVIEWSIMULATION_H

#include <QWidget>

namespace Ui {
class MSIMViewSimulation;
}

/*! The view containing FMU and slave definition tables. */
class MSIMViewSimulation : public QWidget {
	Q_OBJECT
public:
	explicit MSIMViewSimulation(QWidget *parent = 0);
	~MSIMViewSimulation();

public slots:
	/*! Connected to MSIMProjectHandler::modified() */
	void onModified( int modificationType, void * data );

private:
	Ui::MSIMViewSimulation *m_ui;
};

#endif // MSIMVIEWSIMULATION_H
