#ifndef MSIMVIEWCONNECTIONS_H
#define MSIMVIEWCONNECTIONS_H

#include <QWidget>

namespace Ui {
class MSIMViewConnections;
}

class MSIMViewConnections : public QWidget
{
	Q_OBJECT

public:
	explicit MSIMViewConnections(QWidget *parent = 0);
	~MSIMViewConnections();

private:
	Ui::MSIMViewConnections *m_ui;
};

#endif // MSIMVIEWCONNECTIONS_H
