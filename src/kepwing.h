#ifndef _KEPWING_H_
#define _KEPWING_H_

//#include <QtGui>
#include <QObject>
#include <QMainWindow>
#include <QLineEdit>
#include <QListWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QComboBox>
#include <QList>
#include <QKeyEvent>
#include <QEvent>
#include <basedir.h>

#include "searchthread.h"
#include "preferences.h"
#include "ebdict.h"
#include "popup.h"
#include "fifothread.h"

struct KepwingGui {
    QLineEdit* searchbar;
    QListWidget* dictlist;
    QTextEdit* textarea;
    QComboBox* stype;
};

class Kepwing :public QMainWindow {
    Q_OBJECT
    private:
        xdgHandle xdg;
        KepwingGui gui;
        PreferenceWindow* prefw;
        Preferences prefs;
        SearchThread* searchthread;
        SearchThread* popupthread;
        FifoThread* fifothread;
        EBDict* dict;
        QList<EBDictResult> results;

        QString popsearch;
        PopupWindow* popup;

    public:
        Kepwing(QWidget* parent = 0);
        ~Kepwing();
        
        void LoadPreferences();
        void InitGui();

        //bool eventFilter(QObject*, QEvent*);



    public slots:
        void ListClicked(/*QListWidgetItem**/);
        void DisplaySearchResult(EBDictResult);
        void Search(/*QString str = ""*/);
        void DeleteThread();
        void ShowPopup(QString str);

        void DictPathChanged(QString);
    
        void InWindowSearch(QString);

};




























#endif /* _KEPWING_H_ */


