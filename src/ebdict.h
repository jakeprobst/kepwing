#ifndef _EBDICT_H_
#define _EBDICT_H_

#include <QObject>
#include <QString>
#include <QList>
#include <QMutex>

#include <eb/eb.h>

#include "preferences.h"



struct EBDictResult{
    QString dict;
    QString header;
    QString text;
};

struct Dict {
    QString name;
    EB_Book book;
    EB_Subbook_Code subbook;
    bool enabled;
};

class EBDict: public QObject {
    Q_OBJECT
    private:
        bool run;

        QString searchtype;
        QList<Dict> books;
        QMutex* mutex;
        //EB_Book book;

        void LoadFolder(QString);

    public:
        EBDict(QString);
        ~EBDict();

        void stop();
        void Search(QString);

        QList<Dict> DictList() {return books;};



    public slots:
        void SetSearchType(QString st) {searchtype = st;};
        void SetEnable(QString, bool);
        void DictReorder(QList<QString>);

    signals:
        void SearchResult(EBDictResult);

};


























#endif /*_EBDICT_H_ */
