#include "searchthread.h"

SearchThread::SearchThread(EBDict* d, QString s)
{
    dict = d;
    lookup = s;
}

SearchThread::~SearchThread()
{

}

void SearchThread::stop()
{
    dict->stop();
}


void SearchThread::run()
{
    connect(dict, SIGNAL(SearchResult(EBDictResult)), this, SLOT(GetResult(EBDictResult)));
    dict->Search(lookup);
}

void SearchThread::GetResult(EBDictResult res)
{
    emit SearchResult(res);
}



