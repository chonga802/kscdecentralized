#ifndef SAGE_TEXT_EDIT_HH
#define SAGE_TEXT_EDIT_HH

#include <QTextEdit>
#include <QKeyEvent>

// Subclass to extend functionality of QLineEdit to QTextEdit
class SageTextEdit : public QTextEdit
{

	Q_OBJECT

public:
	void keyPressEvent(QKeyEvent *event);

signals:
	void sageReturnPressed();
};

#endif // SAGE_TEXT_EDIT_HH
