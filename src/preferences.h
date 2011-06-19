#ifndef _PREFERENCES_H_
#define _PREFERENCES_H_

#include <QString>
#include <QBoxLayout>
#include <QList>
#include <QCheckBox>
#include <QListWidget>
#include <QApplication>

//#include "ebdict.h"
struct Dict;

struct Preferences {
    QString path;
    QStringList ignoredict;
    QStringList dictorder;
    QByteArray geometry;
    QByteArray state;
    int stype;
    int px,py,pw,ph;


};


class PrefListBox: public QListWidget {
    Q_OBJECT
    private:


    public:
        PrefListBox();
        ~PrefListBox() {};

        bool eventFilter(QObject*, QEvent*);

    signals:
        void ItemMoved();
};


class PreferenceWindow: public QWidget {
    Q_OBJECT
    private:
        PrefListBox* l;
        Preferences* prefs;

        QLineEdit* dictpath;
        QLineEdit* xline;
        QLineEdit* yline;
        QLineEdit* wline;
        QLineEdit* hline;


    public:
        PreferenceWindow(Preferences*);
        ~PreferenceWindow() {};
        void SetDictList(QList<Dict>);

    public slots:
        void CheckChanged(QListWidgetItem*);
        void ItemMoved();
        void DictPathChanged();
        void PopupChanged();

    signals:
        void SetEnable(QString, bool);
        void DictReorder(QList<QString>);
        void DictPathChanged(QString);
        void PopupPrefsChanged();

};



























#endif /* _PREFERENCES_H_ */
