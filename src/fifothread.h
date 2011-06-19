#ifndef _FIFOTHREAD_H_
#define _FIFOTHREAD_H_


#include <QThread>
//#include <QFileSystemWatcher>
#include <basedir.h>

class FifoThread: public QThread {
    Q_OBJECT
    private:
        QString spath;
        int fifofd;
        bool running;
        //QFileSystemWatcher fsw;
        //
        void ExecCommand(QString);

    public:
        FifoThread(xdgHandle);
        ~FifoThread();
        void stop();


    protected:
        void run();

    signals:
        void PopupWord(QString);
        void InWindowSearch(QString);
        void HidePopup();


    //public slots:
    //    void FifoChanged(QString);
        

};

































#endif /* _FIFOTHREAD_H_ */
