#include "ebdict.h"

#include <unistd.h>
#include <stdlib.h>
#include <eb/error.h>
#include <eb/text.h>

#include <QMetaType>
#include <QDir>
#include <QTextCodec>

const int MAX_HITS = 50;
const int MAXLEN_HEADING = 127;


const char* toeuc(QString str)
{
    QTextCodec* enc = QTextCodec::codecForName("euc-jp");
    return enc->fromUnicode(str).constData();
}

QString fromeuc(char* str)
{
    QTextCodec* dec = QTextCodec::codecForName("euc-jp");
    return dec->toUnicode(str, strlen(str));
}

QString fromeuc(QByteArray str)
{
    QTextCodec* dec = QTextCodec::codecForName("euc-jp");
    return dec->toUnicode(str);
}


EBDict::EBDict(QString folder)
{
    run = true;
    qRegisterMetaType<EBDictResult>("EBDictResult");

    eb_initialize_library();

    folder.replace("~", getenv("HOME"));
    LoadFolder(folder);

    searchtype = "Forward";
    mutex = new QMutex;
}


void EBDict::LoadFolder(QString folder)
{
    //EB_Subbook_Code subbook_list[EB_MAX_SUBBOOKS];

    QDir dir(folder);
    dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);

    QFileInfoList list = dir.entryInfoList();
    for(int i = 0; i < list.size(); i++) {
        LoadFolder(list.at(i).absoluteFilePath());
    }


    EB_Book book;
    EB_Error_Code error;

    eb_initialize_book(&book);

    error = eb_bind(&book, folder.toLocal8Bit().constData());
    if (error != EB_SUCCESS) {


        eb_finalize_book(&book);
        return;
    }
        
    EB_Subbook_Code subbook_list[EB_MAX_SUBBOOKS];
    int subbook_count;
    char title[EB_MAX_TITLE_LENGTH + 1];

    //QTextCodec* dec = QTextCodec::codecForName("euc-jp");

    eb_subbook_list(&book, subbook_list, &subbook_count);
    for (int i = 0; i < subbook_count; i++) {
        eb_subbook_title2(&book, subbook_list[i], title);

        Dict d;
        d.name = fromeuc(title); //dec->toUnicode(title, strlen(title));
        d.book = book;
        d.subbook = subbook_list[i];
        d.enabled = true;

        for(int b = 0; b < books.size(); b++) {
            if (d.name == books[b].name) {
                d.name += "[1]";
            }
        }





        books.append(d);
    }
}


EBDict::~EBDict()
{
}


void EBDict::SetEnable(QString n, bool e)
{
    for(int i = 0; i < books.size(); i++) {
        if (books[i].name == n) {
            books[i].enabled = e;
        }
    }
}


void EBDict::stop()
{
    run = false;
}


void EBDict::DictReorder(QList<QString> reorder)
{
    mutex->lock();
    QList<Dict> out;
    for(int i = 0; i < reorder.count(); i++) {
        for(int n = 0; n < books.count(); n++) {
            if (reorder[i] == books[n].name) {
                out.append(books[n]);
                books.removeAt(n);
                break;
            }
        }
    }
    out += books;
    books = out;
    mutex->unlock();
}




void EBDict::Search(QString searchstr)
{
    mutex->lock();

    int lsize = 0;
    run = true;
    int hitcount = 0;
    EB_Hit hits[MAX_HITS];
    char heading[MAXLEN_HEADING + 1];
    for (int i = 0; run && i < books.size(); i++) {
        QString str = searchstr;
        Dict d = books[i];
        if (!d.enabled) {
            continue;
        }
        eb_set_subbook(&d.book, d.subbook);
       
        // this gets the size of the first word in the search str
        // for when highlighting sentences
        while (true) {
            if (searchtype == "Forward") {
                eb_search_word(&d.book, toeuc(str));
            }
            else if (searchtype == "Backward") {
                eb_search_endword(&d.book, toeuc(str));
            }
            else if (searchtype == "Exactword") {
                eb_search_exactword(&d.book, toeuc(str));
            }
            else if (searchtype == "Keyword") {
                //ehhh, eventually
            }
            else {
                // huh this shouldnt happen
                return;
            }
            
            eb_hit_list(&d.book, MAX_HITS, hits, &hitcount);
            if (hitcount > 0) {
                if (str.size() > lsize)
                    lsize = str.size();
            }
            if (str.size() == 0)
                break;
            str.truncate(str.size()-1);
        }

        if (lsize == 0) {
            mutex->unlock();
            return;
        }

        str = searchstr;
        str.truncate(lsize);

        if (searchtype == "Forward") {
            eb_search_word(&d.book, toeuc(str));
        }
        else if (searchtype == "Backward") {
            eb_search_endword(&d.book, toeuc(str));
        }
        else if (searchtype == "Exactword") {
            eb_search_exactword(&d.book, toeuc(str));
        }
        else if (searchtype == "Keyword") {
            //ehhh, eventually
        }
        else {
            // huh this shouldnt happen
            return;
        }
        eb_hit_list(&d.book, MAX_HITS, hits, &hitcount);

        for(int n = 0; n < hitcount; n++) {
            ssize_t l;
            eb_seek_text(&d.book, &(hits[n].heading));
            eb_read_heading(&d.book, NULL,NULL,NULL, MAXLEN_HEADING, heading, &l);

            char text[1024];
            QByteArray atext;
            eb_seek_text(&d.book, &(hits[n].text));
            while(!eb_is_text_stopped(&d.book)) {
                eb_read_text(&d.book, NULL, NULL, NULL, 1023, text, &l);
                atext += text;

            }

            EBDictResult res;
            res.dict = d.name;
            res.header = fromeuc(heading);
            res.text = fromeuc(atext);

            // clean up stuff cause I`m too lazy to implement binary data
            res.header.replace("<?>", "");
            emit SearchResult(res);
        }
    }
    mutex->unlock();
}




























