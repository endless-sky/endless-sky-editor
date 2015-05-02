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
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTreeWidget>

using namespace std;



DetailView::DetailView(Map &mapData, GalaxyView *galaxyView, QWidget *parent) :
    QWidget(parent), mapData(mapData), galaxyView(galaxyView)
{
    name = new QLineEdit;
    government = new QLineEdit;

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
        item->setText(0, QString::fromStdString(it.name));
        item->setText(1, QString::number((it.low + it.high) / 2));
        item->setText(2, "medium");
        //item->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        trade.append(item);
        connect(tradeWidget, SIGNAL(itemClicked(QTreeWidgetItem *, int)),
            this, SLOT(CommodityClicked(QTreeWidgetItem *, int)));
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
        name->setText(QString::fromStdString(system->Name()));
        government->setText(QString::fromStdString(system->Government()));

        QList<QTreeWidgetItem *>::iterator item = trade.begin();
        for(const auto &it : mapData.Commodities())
        {
            static const QString LEVEL[] = {
                "(very low)",
                "(low)",
                "(medium)",
                "(high)",
                "(very high)"
            };
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
        galaxyView->SetGovernment(string(government->text().toUtf8()));
    return false;
}



void DetailView::CommodityClicked(QTreeWidgetItem *item, int /*column*/)
{
    if(galaxyView)
    {
        galaxyView->SetCommodity(string(item->text(0).toUtf8()));
    }
}
