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
#include <QSpinBox>
#include <QTreeWidget>
#include <QVBoxLayout>

using namespace std;



DetailView::DetailView(Map &mapData, GalaxyView *galaxyView, QWidget *parent) :
    QWidget(parent), mapData(mapData), galaxyView(galaxyView)
{
    name = new QLineEdit(this);
    connect(name, SIGNAL(editingFinished()), this, SLOT(NameChanged()));
    government = new QLineEdit(this);
    connect(government, SIGNAL(editingFinished()), this, SLOT(GovernmentChanged()));

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel("System Name:", this));
    layout->addWidget(name);
    layout->addWidget(new QLabel("Government:", this));
    layout->addWidget(government);
    government->installEventFilter(this);

    tradeWidget = new QTreeWidget(this);
    tradeWidget->setIndentation(0);
    tradeWidget->setColumnCount(3);
    QStringList tradeLabels;
    tradeLabels.append("Commodity");
    tradeLabels.append("Price");
    tradeLabels.append("");
    tradeWidget->setHeaderLabels(tradeLabels);
    connect(tradeWidget, SIGNAL(itemClicked(QTreeWidgetItem *, int)),
        this, SLOT(CommodityClicked(QTreeWidgetItem *, int)));
    layout->addWidget(tradeWidget);

    fleets = new QTreeWidget(this);
    fleets->setIndentation(0);
    fleets->setColumnCount(2);
    QStringList fleetLabels;
    fleetLabels.append("Fleet Type");
    fleetLabels.append("Period");
    fleets->setHeaderLabels(fleetLabels);
    fleets->setColumnWidth(0, 200);
    layout->addWidget(fleets);

    minables = new QTreeWidget(this);
    minables->setIndentation(0);
    minables->setColumnCount(3);
    QStringList minableLabels;
    minableLabels.append("Minable Type");
    minableLabels.append("count");
    minableLabels.append("energy");
    minables->setHeaderLabels(minableLabels);
    minables->setColumnWidth(0, 120);
    layout->addWidget(minables);

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

        spinMap.clear();
        tradeWidget->clear();
        tradeWidget->setColumnWidth(1, 70);
        for(const auto &it : mapData.Commodities())
        {
            QTreeWidgetItem *item = new QTreeWidgetItem(tradeWidget);
            int price = system->Trade(it.name);
            item->setText(0, it.name);
            item->setText(2, mapData.PriceLevel(it.name, price));

            QSpinBox *spin = new QSpinBox(tradeWidget);
            spin->setMinimum(0);
            spin->setMaximum(9999);
            spin->setValue(price);
            spin->setSingleStep(10);
            spinMap[spin] = item;
            connect(spin, SIGNAL(valueChanged(int)), this, SLOT(CommodityChanged(int)));
            tradeWidget->insertTopLevelItem(tradeWidget->topLevelItemCount(), item);
            tradeWidget->setItemWidget(item, 1, spin);
        }
        UpdateFleets();
        UpdateMinables();
    }
    else
    {
        name->clear();
        government->clear();
        fleets->clear();
    }
    update();
}



bool DetailView::eventFilter(QObject *object, QEvent *event)
{
    if(object == government && event->type() == QEvent::FocusIn)
        galaxyView->SetGovernment(government->text());
    return false;
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
        mapData.SetChanged();
        galaxyView->update();
    }
}



void DetailView::GovernmentChanged()
{
    if(!system || system->Government() == government->text() || government->text().isEmpty())
        return;

    system->SetGovernment(government->text());
    galaxyView->SetGovernment(government->text());
    mapData.SetChanged();
    galaxyView->update();
}



void DetailView::CommodityClicked(QTreeWidgetItem *item, int /*column*/)
{
    if(galaxyView)
        galaxyView->SetCommodity(item->text(0));
}



void DetailView::CommodityChanged(int value)
{
    auto it = spinMap.find(sender());
    if(it == spinMap.end() || !system)
        return;

    tradeWidget->setCurrentItem(it->second);
    CommodityClicked(it->second, 0);
    system->SetTrade(it->second->text(0), value);
    it->second->setText(2, mapData.PriceLevel(it->second->text(0), value));
    mapData.SetChanged();
    galaxyView->update();
}



void DetailView::FleetChanged(QTreeWidgetItem *item, int column)
{
    if(!system)
        return;

    unsigned row = item->text(2).toInt();
    if(row == system->Fleets().size())
        system->Fleets().push_back({item->text(0), item->text(1).toInt()});
    else if(item->text(0).isEmpty() && item->text(1).isEmpty())
        system->Fleets().erase(system->Fleets().begin() + row);
    else if(column == 0)
        system->Fleets()[row].name = item->text(0);
    else if(column == 1)
        system->Fleets()[row].period = item->text(1).toInt();
    else
        return;

    mapData.SetChanged();

    UpdateFleets();
}



void DetailView::MinableChanged(QTreeWidgetItem *item, int column)
{
    if(!system)
        return;

    unsigned row = item->text(2).toInt();
    if(row == system->Minables().size())
        system->Minables().push_back({item->text(0), item->text(1).toInt(), item->text(2).toDouble()});
    else if(item->text(0).isEmpty() && item->text(1).isEmpty())
        system->Minables().erase(system->Minables().begin() + row);
    else if(column == 0)
        system->Minables()[row].type = item->text(0);
    else if(column == 1)
        system->Minables()[row].count = item->text(1).toInt();
    else if(column == 2)
        system->Minables()[row].energy = item->text(2).toDouble();
    else
        return;

    mapData.SetChanged();

    UpdateMinables();
}



void DetailView::UpdateFleets()
{
    if(!system || !fleets)
        return;

    disconnect(fleets, SIGNAL(itemChanged(QTreeWidgetItem *, int)),
        this, SLOT(FleetChanged(QTreeWidgetItem *, int)));
    fleets->clear();

    for(const System::Fleet &fleet : system->Fleets())
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(fleets);
        item->setText(0, fleet.name);
        item->setText(1, QString::number(fleet.period));
        item->setText(2, QString::number(&fleet - &system->Fleets().front()));
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        fleets->addTopLevelItem(item);
    }
    {
        // Add one last item, which is empty, but can be edited to add a row.
        QTreeWidgetItem *item = new QTreeWidgetItem(fleets);
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        item->setText(2, QString::number(system->Fleets().size()));
        fleets->addTopLevelItem(item);
    }
    fleets->setColumnWidth(0, 200);
    connect(fleets, SIGNAL(itemChanged(QTreeWidgetItem *, int)),
        this, SLOT(FleetChanged(QTreeWidgetItem *, int)));
}



void DetailView::UpdateMinables()
{
    if(!system || !minables)
        return;

    disconnect(minables, SIGNAL(itemChanged(QTreeWidgetItem *, int)),
        this, SLOT(MinableChanged(QTreeWidgetItem *, int)));
    minables->clear();

    for(const System::Minable &minable : system->Minables())
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(minables);
        item->setText(0, minable.type);
        item->setText(1, QString::number(minable.count));
        item->setText(2, QString::number(minable.energy));
        item->setText(3, QString::number(&minable - &system->Minables().front()));

        item->setFlags(item->flags() | Qt::ItemIsEditable);
        minables->addTopLevelItem(item);
    }
    {
        // Add one last item, which is empty, but can be edited to add a row.
        QTreeWidgetItem *item = new QTreeWidgetItem(minables);
        item->setFlags(item->flags() | Qt::ItemIsEditable);

        item->setText(3, QString::number(system->Minables().size()));
        minables->addTopLevelItem(item);
    }
    minables->setColumnWidth(0, 120);
    connect(minables, SIGNAL(itemChanged(QTreeWidgetItem *, int)),
        this, SLOT(MinableChanged(QTreeWidgetItem *, int)));

}
