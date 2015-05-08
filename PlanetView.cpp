/* PlanetView.cpp
Copyright (c) 2015 by Michael Zahniser

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#include "PlanetView.h"

#include "LandscapeView.h"
#include "Map.h"
#include "StellarObject.h"

#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QGridLayout>



PlanetView::PlanetView(Map &mapData, QWidget *parent) :
    QWidget(parent), mapData(mapData)
{
    name = new QLineEdit;
    connect(name, SIGNAL(editingFinished()), this, SLOT(NameChanged()));
    landscape = new LandscapeView(this);
    landscape->setMinimumHeight(360);
    landscape->setMaximumHeight(360);
    description = new QPlainTextEdit;
    description->setTabStopWidth(20);
    connect(description, SIGNAL(textChanged()), this, SLOT(DescriptionChanged()));
    spaceport = new QPlainTextEdit;
    spaceport->setTabStopWidth(20);
    connect(spaceport, SIGNAL(textChanged()), this, SLOT(SpaceportDescriptionChanged()));

    QGridLayout *layout = new QGridLayout;
    layout->addWidget(new QLabel("Planet:"), 0, 0);
    layout->addWidget(name, 0, 1);
    layout->addWidget(landscape, 1, 0, 1, 2);
    layout->addWidget(description, 2, 0, 1, 2);
    layout->addWidget(new QLabel("Spaceport description:"), 3, 0, 1, 2);
    layout->addWidget(spaceport, 4, 0, 1, 2);

    setLayout(layout);
}



void PlanetView::SetPlanet(StellarObject *object)
{
    this->object = object;

    auto it = mapData.Planets().end();
    if(object && !object->GetPlanet().isEmpty())
        it = mapData.Planets().find(object->GetPlanet());

    if(it == mapData.Planets().end())
    {
        name->clear();
        landscape->SetPlanet(nullptr);
        description->clear();
        spaceport->clear();
    }
    else
    {
        name->setText(it->second.Name());
        landscape->SetPlanet(&it->second);
        description->setPlainText(it->second.Description());
        spaceport->setPlainText(it->second.SpaceportDescription());
    }
}



void PlanetView::NameChanged()
{
    if(!object || object->GetPlanet() == name->text() || name->text().isEmpty())
        return;

    auto it = mapData.Planets().find(name->text());
    if(it != mapData.Planets().end())
    {
        QMessageBox::warning(this, "Duplicate name",
            "A planet named \"" + name->text() + "\" already exists.");
    }
    else
    {
        mapData.RenamePlanet(object, name->text());
        Planet &planet = mapData.Planets()[name->text()];
        planet.SetLandscape(landscape->Landscape());
        planet.SetDescription(description->toPlainText());
        planet.SetSpaceportDescription(spaceport->toPlainText());
    }
}



void PlanetView::DescriptionChanged()
{
    if(object && !object->GetPlanet().isEmpty())
        mapData.Planets()[object->GetPlanet()].SetDescription(description->toPlainText());
}



void PlanetView::SpaceportDescriptionChanged()
{
    if(object && !object->GetPlanet().isEmpty())
        mapData.Planets()[object->GetPlanet()].SetSpaceportDescription(spaceport->toPlainText());
}
