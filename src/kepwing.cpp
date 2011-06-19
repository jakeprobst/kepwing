#include "kepwing.h"
#include <stdio.h>
#include <basedir.h>

#include <QBoxLayout>
#include <QIcon>
#include <QSettings>
#include <QFile>
#include <QSplitter>
#include <QAction>
#include <QX11Info>
#include <QApplication>
#include <QDesktopWidget>
#include <QClipboard>
/*
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
*/
#include "searchthread.h"

Kepwing::Kepwing(QWidget* parent)
        :QMainWindow(parent)
{
    xdgInitHandle(&xdg);
    LoadPreferences();

    searchthread = NULL;
    popupthread = NULL;
    dict = new EBDict(prefs.path);
    dict->DictReorder(prefs.dictorder);
    InitGui();

    fifothread = new FifoThread(xdg);
    connect(fifothread, SIGNAL(PopupWord(QString)),
            this, SLOT(ShowPopup(QString)));
    connect(fifothread, SIGNAL(InWindowSearch(QString)),
            this, SLOT(InWindowSearch(QString)));
    connect(fifothread, SIGNAL(HidePopup()),
            popup, SLOT(hide()));
    fifothread->start();

    restoreGeometry(prefs.geometry);
    restoreState(prefs.state);
}

Kepwing::~Kepwing()
{
    QString file = QString(xdgConfigHome(&xdg)) + "/kepwing/config";
    QSettings config(file, QSettings::IniFormat);
    file = QString(xdgConfigHome(&xdg)) + "/kepwing/state";
    QSettings state(file, QSettings::IniFormat);

    config.setValue("dict/path", prefs.path);
    config.setValue("popup/x", prefs.px);
    config.setValue("popup/y", prefs.py);
    config.setValue("popup/w", prefs.pw);
    config.setValue("popup/h", prefs.ph);

    QList<Dict> dl = dict->DictList();
    state.beginWriteArray("dict/ignore");
    for(int i = 0, n = 0; i < dl.size(); i++) {
        if (!dl[i].enabled) {
            state.setArrayIndex(n++);
            state.setValue("ignore", dl[i].name);
        }
    }
    state.endArray();
    
    state.beginWriteArray("dict/order");
    for(int i = 0, n = 0; i < dl.size(); i++) {
        state.setArrayIndex(n++);
        state.setValue("order", dl[i].name);
    }
    state.endArray();

    state.setValue("gui/geometry", saveGeometry());
    state.setValue("gui/state", saveState());
    state.setValue("gui/stype", gui.stype->currentIndex());
    state.sync();

    fifothread->stop();
    fifothread->wait();
    delete fifothread;
}

void Kepwing::LoadPreferences()
{
    QString file = QString(xdgConfigHome(&xdg)) + "/kepwing/state";
    QSettings state(file, QSettings::IniFormat);
    file = QString(xdgConfigHome(&xdg)) + "/kepwing/config";
    QSettings config(file, QSettings::IniFormat);

    prefs.path = config.value("dict/path").toString();
    prefs.px = config.value("popup/x",0).toInt();
    prefs.py = config.value("popup/y",0).toInt();
    prefs.pw = config.value("popup/w", 300).toInt();
    prefs.ph = config.value("popup/h", 300).toInt();

    int s = state.beginReadArray("dict/ignore");
    for(int i = 0; i < s; i++) {
        state.setArrayIndex(i);
        prefs.ignoredict.append(state.value("ignore").toString());
    }
    state.endArray();
    
    s = state.beginReadArray("dict/order");
    for(int i = 0; i < s; i++) {
        state.setArrayIndex(i);
        prefs.dictorder.append(state.value("order").toString());
    }
    state.endArray();

    prefs.geometry = state.value("gui/geometry").toByteArray();
    prefs.state = state.value("gui/state").toByteArray();
    prefs.stype = state.value("gui/stype", 0).toInt();
}

void Kepwing::InitGui()
{
    gui.searchbar = new QLineEdit;
    gui.dictlist = new QListWidget;
    gui.textarea = new QTextEdit;
    gui.stype = new QComboBox;

    gui.textarea->setReadOnly(true);
    connect(gui.stype, SIGNAL(activated(QString)), 
            dict, SLOT(SetSearchType(QString)));

    QStringList l;
    l << "Forward" << "Backward" << "Exactword" << "Keyword";
    gui.stype->addItems(l);
    gui.stype->setCurrentIndex(prefs.stype);
    dict->SetSearchType(l[prefs.stype]);

    QHBoxLayout* sbartype = new QHBoxLayout;
    sbartype->addWidget(gui.searchbar, 1);
    sbartype->addWidget(gui.stype, 0);

    //QHBoxLayout* dicttext = new QHBoxLayout;
    QSplitter* dicttext = new QSplitter(Qt::Horizontal);
    dicttext->addWidget(gui.dictlist);
    dicttext->addWidget(gui.textarea);
    QList<int> il;
    il << 150 << 1000; 
    dicttext->setSizes(il);

    QVBoxLayout* searchdict = new QVBoxLayout;
    searchdict->addLayout(sbartype, 0);
    searchdict->addWidget(dicttext, 1);

    QWidget* layout = new QWidget;
    layout->setLayout(searchdict);
    setCentralWidget(layout);
    setWindowTitle("Kepwing");
    
    connect(gui.searchbar, SIGNAL(returnPressed()), 
            this, SLOT(Search()));
    connect(gui.dictlist, SIGNAL(itemSelectionChanged()),
            this, SLOT(ListClicked()));
    //connect(gui.dictlist, SIGNAL(selected()),
    //        this, SLOT(ListClicked()));

    // pref window...
    for(int i = 0; i < prefs.ignoredict.size(); i++) {
        dict->SetEnable(prefs.ignoredict[i], false);
    }

    prefw = new PreferenceWindow(&prefs);
    prefw->SetDictList(dict->DictList());
    connect(prefw, SIGNAL(SetEnable(QString, bool)), 
            dict, SLOT(SetEnable(QString, bool)));
    connect(prefw, SIGNAL(DictReorder(QList<QString>)), 
            dict, SLOT(DictReorder(QList<QString>)));
    connect(prefw, SIGNAL(DictPathChanged(QString)), 
            this, SLOT(DictPathChanged(QString)));
    
    QAction* selbar = new QAction(this);
    selbar->setShortcut(QKeySequence("Ctrl+L"));
    selbar->setShortcutContext(Qt::ApplicationShortcut);
    connect(selbar, SIGNAL(triggered()),
            gui.searchbar, SLOT(selectAll()));
    connect(selbar, SIGNAL(triggered()),
            gui.searchbar, SLOT(setFocus()));
    addAction(selbar);

    QAction* prefact = new QAction(this);
    prefact->setShortcut(QKeySequence("Ctrl+D"));
    prefact->setShortcutContext(Qt::ApplicationShortcut);
    connect(prefact, SIGNAL(triggered()),
            prefw, SLOT(show()));
    addAction(prefact);
    
    popup = new PopupWindow(&prefs);
    connect(prefw, SIGNAL(PopupPrefsChanged()),
            popup, SLOT(PopupPrefsChanged()));
}

void Kepwing::ListClicked()
{
    gui.textarea->clear();
    QString path = QString(xdgConfigHome(&xdg)) + "/kepwing/default.css";
    QFile file(path);
    file.open(QIODevice::ReadOnly);
    QString css = file.readAll();
    gui.textarea->document()->setDefaultStyleSheet(css);

    QString dic = results[gui.dictlist->currentRow()].dict;
    QString str = results[gui.dictlist->currentRow()].text;
    str.replace(" ", "&nbsp;");
    str.replace("\n", "<br />");
    str = "<div class=\"dicttext\">" + str + "</div>";
    str = "<div align=\"center\" class=\"dictname\">" + dic + "</div><br /><br /> " + str;

    gui.textarea->setHtml(str);
}


void Kepwing::DeleteThread()
{
    if (searchthread && searchthread->isFinished()) {
        delete searchthread;
        searchthread = NULL;
    }
    if (popupthread && popupthread->isFinished()) {
        delete popupthread;
        popupthread = NULL;
    }
}


void Kepwing::DisplaySearchResult(EBDictResult result)
{
    gui.dictlist->addItem(result.header);
    results.append(result);
}

void Kepwing::Search()
{
    if (searchthread) {
        searchthread->stop();
        searchthread->wait();
        delete searchthread;
    }

    gui.dictlist->clear();
    results.clear();

    searchthread = new SearchThread(dict, gui.searchbar->text());
    connect(searchthread, SIGNAL(SearchResult(EBDictResult)), 
            this, SLOT(DisplaySearchResult(EBDictResult)));
    connect(searchthread, SIGNAL(finished()), 
            this, SLOT(DeleteThread()));
    searchthread->start();
}


void Kepwing::ShowPopup(QString str)
{
    if (str.size() == 0) {
        QClipboard* clip = QApplication::clipboard();
        str = clip->text(QClipboard::Selection);
    }

    if (popup->isVisible() && !popup->underMouse() && 
        (str == popsearch || str.size() == 0)) {
        popup->hide();
        return;
    }

    if (str.size() == 0)
        return;

    if (str == popsearch) {
        popup->show();
        return;
    }

    if (popupthread) {
        popupthread->stop();
        popupthread->wait();
        delete popupthread;
    }

    popup->Clear();

    QString path = QString(xdgConfigHome(&xdg)) + "/kepwing/default.css";
    QFile file(path);
    file.open(QIODevice::ReadOnly);
    popup->SetCSS(file.readAll());

    popsearch = str;
    popupthread = new SearchThread(dict, str);
    connect(popupthread, SIGNAL(SearchResult(EBDictResult)), 
            popup, SLOT(GetResult(EBDictResult)));
    connect(popupthread, SIGNAL(finished()), 
            this, SLOT(DeleteThread()));
    popupthread->start();

    popup->show();
}


void Kepwing::InWindowSearch(QString str)
{
    gui.searchbar->setText(str);
    Search();
}


void Kepwing::DictPathChanged(QString str)
{
    prefs.path = str;
    if (searchthread) {
        searchthread->stop();
        searchthread->wait();
        delete searchthread;
    }
    delete dict;
    dict = new EBDict(prefs.path);
    dict->DictReorder(prefs.dictorder);
    for(int i = 0; i < prefs.ignoredict.size(); i++) {
        dict->SetEnable(prefs.ignoredict[i], false);
    }
    prefw->SetDictList(dict->DictList());
}







