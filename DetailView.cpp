/* DetailView.cpp
Copyright (c) 2015 by Michael Zahniser

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#include "DetailView.h"

#include "GalaxyView.h"
#include "Map.h"
#include "System.h"

#include <QEvent>
#include <QMessageBox>
#include <QLabel>
#include <QLineEdit>
#include <QTreeWidget>
#include <QVBoxLayout>

using namespace std;

namespace {
    static const QString LEVEL[] = {
                "(very low)",
                "(low)",
                "(medium)",
                "(high)",
                "(very high)"
            };
}



DetailView::DetailView(Map &mapData, GalaxyView *galaxyView, QWidget *parent) :
    QWidget(parent), mapData(mapData), galaxyView(galaxyView)
{
    name = new QLineEdit;
    connect(name, SIGNAL(editingFinished()), this, SLOT(NameChanged()));
    government = new QLineEdit;
    connect(government, SIGNAL(editingFinished()), this, SLOT(GovernmentChanged()));

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(new QLabel("System Name:"));
    layout->addWidget(name);
    layout->addWidget(new QLabel("Government:"));
    layout->addWidget(government);
    government->installEventFilter(this);

    QTreeWidget *tradeWidget = new QTreeWidget;
    tradeWidget->setIndentation(0);
    tradeWidget->setColumnCount(3);
    QStringList tradeLabels;
    tradeLabels.append("Commodity");
    tradeLabels.append("Price");
    tradeLabels.append("");
    tradeWidget->setHeaderLabels(tradeLabels);
    for(const auto &it : mapData.Commodities())
    {
        QTreeWidgetItem *item = new QTreeWidgetItem;
        item->setText(0, it.name);
        item->setText(1, QString::number((it.low + it.high) / 2));
        item->setText(2, "medium");
        trade.append(item);
        connect(tradeWidget, SIGNAL(itemClicked(QTreeWidgetItem *, int)),
            this, SLOT(CommodityClicked(QTreeWidgetItem *, int)));
        connect(tradeWidget, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
            this, SLOT(CommodityChanged(QTreeWidgetItem *, int)));
    }
    tradeWidget->setColumnWidth(1, 50);
    tradeWidget->insertTopLevelItems(0, trade);
    layout->addWidget(tradeWidget);

    setLayout(layout);
}



void DetailView::SetSystem(System *system)
{
    if(system == this->system)
        return;

    this->system = system;
    if(system)
    {
        name->setText(system->Name());
        government->setText(system->Government());

        QList<QTreeWidgetItem *>::iterator item = trade.begin();
        for(const auto &it : mapData.Commodities())
        {
            int price = system->Trade(it.name);
            int level = max(0, min(4, ((price - it.low) * 5) / (it.high - it.low)));
            (*item)->setText(1, QString::number(price));
            (*item)->setText(2, LEVEL[level]);
            ++item;
        }
    }
    else
    {
        name->setText(QString());
        government->setText(QString());
    }
    update();
}



bool DetailView::eventFilter(QObject* object, QEvent* event)
{
    if(object == government && event->type() == QEvent::FocusIn)
        galaxyView->SetGovernment(government->text());
    return false;
}



void DetailView::CommodityClicked(QTreeWidgetItem *item, int column)
{
    if(galaxyView)
        galaxyView->SetCommodity(item->text(0));

    if(column == 1)
        item->setFlags(item->flags() | Qt::ItemIsEditable);
    else
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
}



void DetailView::CommodityChanged(QTreeWidgetItem *item, int column)
{
    if(system && column == 1)
        for(const auto &it : mapData.Commodities())
            if(it.name == item->text(0))
            {
                int price = item->text(1).toInt();
                system->SetTrade(it.name, price);
                int level = max(0, min(4, ((price - it.low) * 5) / (it.high - it.low)));
                item->setText(2, LEVEL[level]);
                galaxyView->update();
                break;
            }
}



void DetailView::NameChanged()
{
    if(!system || system->Name() == name->text() || name->text().isEmpty())
        return;

    auto it = mapData.Systems().find(name->text());
    if(it != mapData.Systems().end())
    {
        QMessageBox::warning(this, "Duplicate name",
            "A system named \"" + name->text() + "\" already exists.");
    }
    else
    {
        mapData.RenameSystem(system->Name(), name->text());
        galaxyView->update();
    }
}



void DetailView::GovernmentChanged()
{
    if(!system || system->Government() == government->text() || government->text().isEmpty())
        return;

    system->SetGovernment(government->text());
    galaxyView->SetGovernment(government->text());
}
