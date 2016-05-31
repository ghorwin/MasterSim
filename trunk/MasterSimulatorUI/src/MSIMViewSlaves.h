#ifndef MSIMVIEWSLAVES_H
#define MSIMVIEWSLAVES_H

#include <QWidget>

namespace Ui {
class MSIMViewSlaves;
}

class MSIMViewSlaves : public QWidget
{
	Q_OBJECT

public:
	explicit MSIMViewSlaves(QWidget *parent = 0);
	~MSIMViewSlaves();

private:
	Ui::MSIMViewSlaves *m_ui;
};

#endif // MSIMVIEWSLAVES_H
