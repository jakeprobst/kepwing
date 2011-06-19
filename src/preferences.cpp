#include "preferences.h"
#include "ebdict.h"

#include <stdio.h>

#include <QBoxLayout>
#include <QScrollArea>
#include <QGroupBox>
#include <QLineEdit>
#include <QLabel>

PrefListBox::PrefListBox()
            :QListWidget()
{
    qApp->installEventFilter(this);

}

bool PrefListBox::eventFilter(QObject*, QEvent* event)
{
    if (event->type() == QEvent::ChildRemoved) {
        emit ItemMoved();
    }
    return false;
}


PreferenceWindow::PreferenceWindow(Preferences* p)
                 :QWidget(NULL)
{
    prefs = p;
    l = new PrefListBox;
    l->setDragDropMode(QAbstractItemView::InternalMove);

    QGroupBox* dictlistbox = new QGroupBox("dictionaries");
    QVBoxLayout* dictlistlayout = new QVBoxLayout;
    dictlistlayout->addWidget(l);
    dictlistbox->setLayout(dictlistlayout);
    connect(l, SIGNAL(itemChanged(QListWidgetItem*)), 
            this, SLOT(CheckChanged(QListWidgetItem*)));
    connect(l, SIGNAL(ItemMoved()), 
            this, SLOT(ItemMoved()));

    QGroupBox* dictbox = new QGroupBox("dictionary path");
    dictpath = new QLineEdit;
    connect(dictpath, SIGNAL(returnPressed()),
            this, SLOT(DictPathChanged()));
    dictpath->setText(prefs->path);
    QVBoxLayout* dictlayout = new QVBoxLayout;
    dictlayout->addWidget(dictpath);
    dictbox->setLayout(dictlayout);
    
    QGroupBox* popupbox = new QGroupBox("popup window");
    QGridLayout* popuplayout = new QGridLayout;
    popuplayout->addWidget(new QLabel("x"), 0, 0);
    popuplayout->addWidget(new QLabel("y"), 1, 0);
    popuplayout->addWidget(new QLabel("w"), 2, 0);
    popuplayout->addWidget(new QLabel("h"), 3, 0);

    xline = new QLineEdit;
    xline->setText(QString("%1").arg(prefs->px));
    yline = new QLineEdit;
    yline->setText(QString("%1").arg(prefs->py));
    wline = new QLineEdit;
    wline->setText(QString("%1").arg(prefs->pw));
    hline = new QLineEdit;
    hline->setText(QString("%1").arg(prefs->ph));

    connect(xline, SIGNAL(textChanged(QString)),
            this, SLOT(PopupChanged()));
    connect(yline, SIGNAL(textChanged(QString)),
            this, SLOT(PopupChanged()));
    connect(wline, SIGNAL(textChanged(QString)),
            this, SLOT(PopupChanged()));
    connect(hline, SIGNAL(textChanged(QString)),
            this, SLOT(PopupChanged()));

    popuplayout->addWidget(xline, 0, 1);
    popuplayout->addWidget(yline, 1, 1);
    popuplayout->addWidget(wline, 2, 1);
    popuplayout->addWidget(hline, 3, 1);
    popupbox->setLayout(popuplayout);

    QVBoxLayout* tboxes = new QVBoxLayout;
    tboxes->addWidget(dictbox);
    tboxes->addWidget(popupbox);
    tboxes->addStretch(0);

    QHBoxLayout* ll = new QHBoxLayout;
    ll->addWidget(dictlistbox);
    ll->addLayout(tboxes);
    setLayout(ll);
}

void PreferenceWindow::SetDictList(QList<Dict> dicts)
{
    l->clear();
    for(int i = 0; i < dicts.size(); i++) {
        QListWidgetItem* item = new QListWidgetItem(dicts[i].name);
        if (dicts[i].enabled)
            item->setCheckState(Qt::Checked);
        else
            item->setCheckState(Qt::Unchecked);
        l->addItem(item);
    }
}


void PreferenceWindow::CheckChanged(QListWidgetItem* item)
{
    QString t = item->text();
    bool e = false;
    if (item->checkState() == Qt::Checked) {
        e = true;
    }

    emit SetEnable(t, e);
}

void PreferenceWindow::DictPathChanged()
{
    QString str = dictpath->text();
    emit DictPathChanged(str);
}

void PreferenceWindow::PopupChanged()
{
    prefs->px = xline->text().toInt();
    prefs->py = yline->text().toInt();
    prefs->pw = wline->text().toInt();
    prefs->ph = hline->text().toInt();

    emit PopupPrefsChanged();
}

void PreferenceWindow::ItemMoved()
{
    QList<QString> dictorder;
    for(int i = 0; i < l->count(); i++) {
        dictorder.append(l->item(i)->text());
    }

    emit DictReorder(dictorder);

}





