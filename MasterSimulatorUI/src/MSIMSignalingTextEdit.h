#ifndef MSIMSignalingTextEditH
#define MSIMSignalingTextEditH

#include <QPlainTextEdit>

class MSIMSignalingTextEdit : public QPlainTextEdit {
	Q_OBJECT
public:
	MSIMSignalingTextEdit(QWidget *parent) : QPlainTextEdit(parent) {}

protected:
	/*! Emits the editingFinished() signal. */
	virtual void focusOutEvent(QFocusEvent * /*event*/) {
		emit editingFinished();
	}

signals:
	void editingFinished();
};

#endif // MSIMSignalingTextEdit
