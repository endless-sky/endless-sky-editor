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
#include <QTextEdit>
#include <QGridLayout>



PlanetView::PlanetView(Map &mapData, QWidget *parent) :
    QWidget(parent), mapData(mapData)
{
    name = new QLineEdit;
    landscape = new LandscapeView(this);
    landscape->setMinimumHeight(360);
    landscape->setMaximumHeight(360);
    description = new QTextEdit;
    description->setTabStopWidth(20);
    spaceport = new QTextEdit;
    spaceport->setTabStopWidth(20);

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
        description->setText(it->second.Description());
        spaceport->setText(it->second.SpaceportDescription());
    }
}
