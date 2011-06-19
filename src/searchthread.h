#ifndef _SEARCHTHREAD_H_
#define _SEARCHTHREAD_H_

#include <QThread>

#include <stdio.h>
#include <unistd.h>

#include "ebdict.h"

class SearchThread: public QThread {
    Q_OBJECT
    private:
        EBDict* dict;
        QString lookup;

    public:
        SearchThread(EBDict*, QString);
        ~SearchThread();
        void stop();

    protected:
        void run();

    signals:
        void SearchResult(EBDictResult);

    public slots:
        void GetResult(EBDictResult);
};




























#endif /* _SEARCHTHREAD_H_ */
