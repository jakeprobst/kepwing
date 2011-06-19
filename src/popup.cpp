#include "popup.h"

#include <stdio.h>

#include <QAction>
#include <QBoxLayout>
#include <QTextBlock>

PopupWindow::PopupWindow(Preferences* p)
            :QFrame(NULL)
{
    prefs = p;
    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::Popup);
    setFocusPolicy(Qt::NoFocus);
    setAttribute(Qt::WA_ShowWithoutActivating);
    setObjectName("popupwindow");

    text = new QTextEdit;
    text->setReadOnly(true);

    QAction* pophide = new QAction(this);
    pophide->setShortcut(QKeySequence("Ctrl+w"));
    pophide->setShortcutContext(Qt::ApplicationShortcut);
    connect(pophide, SIGNAL(triggered()), this, SLOT(esc()));
    addAction(pophide);

    QVBoxLayout* l = new QVBoxLayout;
    l->addWidget(text);
    l->setContentsMargins(0,0,0,0);
    setLayout(l);

    setFixedSize(prefs->pw, prefs->ph);
    move(prefs->px, prefs->py);
}


PopupWindow::~PopupWindow()
{
}

void PopupWindow::Clear()
{
    text->clear();
}


void PopupWindow::mousePressEvent(QMouseEvent* event)
{
    if (!geometry().contains(event->globalPos())) {
        hide();
    }
}

void PopupWindow::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape) {
        hide();
    }
}

void PopupWindow::esc()
{
    hide();
}


void PopupWindow::SetCSS(QString css)
{
    text->document()->setDefaultStyleSheet(css);
}

void PopupWindow::GetResult(EBDictResult result)
{
    QString dic = result.dict;
    QString str = result.text;

    QString out;
    out = "<div align=\"center\" class=\"popupdictname\">" ;
    out += dic + "</div> ";
    out += "<div class=\"popupdicttext\">" + str + "</div><br /><br />";

    out.replace("\n", "<br />");

    text->append(out);
    text->moveCursor(QTextCursor::Start);
}

void PopupWindow::PopupPrefsChanged()
{
    setFixedSize(prefs->pw, prefs->ph);
    move(prefs->px, prefs->py);
}
