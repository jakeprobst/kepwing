#ifndef _POPUP_H_
#define _POPUP_H_

#include <QWidget>
#include <QString>
#include <QTextEdit>
#include <QPushButton>
#include <QFrame>
#include <QMouseEvent>
#include "ebdict.h"
#include "preferences.h"

class PopupWindow: public QFrame {
    Q_OBJECT
    private:
        QTextEdit* text;
        int x,y,w,h;
        Preferences* prefs;

    public:
        PopupWindow(Preferences*);
        ~PopupWindow();

        void SetCSS(QString);
        void Clear();
        void mousePressEvent(QMouseEvent*);
        void keyPressEvent(QKeyEvent*);

    public slots:
        void GetResult(EBDictResult);
        void esc();
        void PopupPrefsChanged();
};




























#endif /* _POPUP_H_ */
