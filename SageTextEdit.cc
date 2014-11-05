#include "SageTextEdit.hh"

////////////////////////////////////////////////////////////////
// SageTextEdit
//   Overwrites QTextEdit to provide additional
//   functionality for multi-line editing.
////////////////////////////////////////////////////////////////

void SageTextEdit::keyPressEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_Return)
		emit sageReturnPressed();
	else
		QTextEdit::keyPressEvent(event);
}
