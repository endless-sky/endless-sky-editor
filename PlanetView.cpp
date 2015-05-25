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

#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QRegExp>
#include <QRegExpValidator>

#include <cmath>
#include <limits>

using namespace std;



PlanetView::PlanetView(Map &mapData, QWidget *parent) :
    QWidget(parent), mapData(mapData)
{
    name = new QLineEdit(this);
    connect(name, SIGNAL(editingFinished()), this, SLOT(NameChanged()));
    attributes = new QLineEdit(this);
    connect(attributes, SIGNAL(editingFinished()), this, SLOT(AttributesChanged()));

    landscape = new LandscapeView(mapData, this);
    landscape->setMinimumHeight(360);
    landscape->setMaximumHeight(360);

    description = new QPlainTextEdit(this);
    description->setTabStopWidth(20);
    connect(description, SIGNAL(textChanged()), this, SLOT(DescriptionChanged()));
    spaceport = new QPlainTextEdit(this);
    spaceport->setTabStopWidth(20);
    connect(spaceport, SIGNAL(textChanged()), this, SLOT(SpaceportDescriptionChanged()));

    shipyard = new QLineEdit(this);
    connect(shipyard, SIGNAL(editingFinished()), this, SLOT(ShipyardChanged()));
    outfitter = new QLineEdit(this);
    connect(outfitter, SIGNAL(editingFinished()), this, SLOT(OutfitterChanged()));

    reputation = new QLineEdit(this);
    reputation->setValidator(new QRegExpValidator(QRegExp("-?\\d*\\.?\\d*"), reputation));
    connect(reputation, SIGNAL(editingFinished()), this, SLOT(ReputationChanged()));
    bribe = new QLineEdit(this);
    bribe->setValidator(new QRegExpValidator(QRegExp("0||0?\\.\\d*"), bribe));
    connect(bribe, SIGNAL(editingFinished()), this, SLOT(BribeChanged()));
    security = new QLineEdit(this);
    security->setValidator(new QRegExpValidator(QRegExp("0||0?\\.\\d*"), security));
    connect(security, SIGNAL(editingFinished()), this, SLOT(SecurityChanged()));

    QGridLayout *layout = new QGridLayout(this);
    int row = 0;

    layout->addWidget(new QLabel("Planet:", this), row, 0);
    layout->addWidget(name, row++, 1);
    layout->addWidget(new QLabel("Attributes:", this), row, 0);
    layout->addWidget(attributes, row++, 1);

    layout->addWidget(landscape, row++, 0, 1, 2);

    layout->addWidget(description, row++, 0, 1, 2);
    layout->addWidget(new QLabel("Spaceport description:", this), row++, 0, 1, 2);
    layout->addWidget(spaceport, row++, 0, 1, 2);

    layout->addWidget(new QLabel("Shipyard:", this), row, 0);
    layout->addWidget(shipyard, row++, 1);
    layout->addWidget(new QLabel("Outfitter:", this), row, 0);
    layout->addWidget(outfitter, row++, 1);

    QWidget *box = new QWidget(this);
    QHBoxLayout *hLayout = new QHBoxLayout(box);
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->addWidget(new QLabel("Required reputation:", this));
    hLayout->addWidget(reputation);
    hLayout->addWidget(new QLabel("Bribe:", this));
    hLayout->addWidget(bribe);
    hLayout->addWidget(new QLabel("Security:", this));
    hLayout->addWidget(security);
    hLayout->addStretch();
    layout->addWidget(box, row++, 0, 1, 2);

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
        attributes->clear();
        landscape->SetPlanet(nullptr);
        description->clear();
        spaceport->clear();
        shipyard->clear();
        outfitter->clear();
        reputation->clear();
        bribe->clear();
        security->clear();
    }
    else
    {
        name->setText(it->second.Name());
        attributes->setText(ToString(it->second.Attributes()));
        landscape->SetPlanet(&it->second);

        disconnect(description, SIGNAL(textChanged()), this, SLOT(DescriptionChanged()));
        description->setPlainText(it->second.Description());
        connect(description, SIGNAL(textChanged()), this, SLOT(DescriptionChanged()));

        disconnect(spaceport, SIGNAL(textChanged()), this, SLOT(SpaceportDescriptionChanged()));
        spaceport->setPlainText(it->second.SpaceportDescription());
        connect(spaceport, SIGNAL(textChanged()), this, SLOT(SpaceportDescriptionChanged()));

        shipyard->setText(ToString(it->second.Shipyard()));
        outfitter->setText(ToString(it->second.Outfitter()));
        reputation->setText(std::isnan(it->second.RequiredReputation()) ?
            QString() : QString::number(it->second.RequiredReputation()));
        bribe->setText(std::isnan(it->second.Bribe()) ?
            QString() : QString::number(it->second.Bribe()));
        security->setText(std::isnan(it->second.Security()) ?
            QString() : QString::number(it->second.Security()));
    }
}


void PlanetView::Reinitialize()
{
    SetPlanet(nullptr);
    landscape->Reinitialize();
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
        planet.Attributes() = ToList(attributes->text());
        planet.SetLandscape(landscape->Landscape());
        landscape->SetPlanet(&planet);
        planet.SetDescription(description->toPlainText());
        planet.SetSpaceportDescription(spaceport->toPlainText());
        if(!reputation->text().isEmpty())
            planet.SetRequiredReputation(reputation->text().toDouble());
        if(!bribe->text().isEmpty())
            planet.SetBribe(bribe->text().toDouble());
        if(!security->text().isEmpty())
            planet.SetSecurity(security->text().toDouble());
        mapData.SetChanged();
    }
}



void PlanetView::AttributesChanged()
{
    if(object && !object->GetPlanet().isEmpty())
    {
        vector<QString> list = ToList(attributes->text());
        Planet &planet = mapData.Planets()[object->GetPlanet()];
        if(planet.Attributes() != list)
        {
            planet.Attributes() = list;
            mapData.SetChanged();
        }
    }
}



void PlanetView::DescriptionChanged()
{
    if(object && !object->GetPlanet().isEmpty())
    {
        QString newDescription = description->toPlainText();
        Planet &planet = mapData.Planets()[object->GetPlanet()];
        if(planet.Description() != newDescription)
        {
            planet.SetDescription(newDescription);
            mapData.SetChanged();
        }
    }
}



void PlanetView::SpaceportDescriptionChanged()
{
    if(object && !object->GetPlanet().isEmpty())
    {
        QString newDescription = spaceport->toPlainText();
        Planet &planet = mapData.Planets()[object->GetPlanet()];
        if(planet.SpaceportDescription() != newDescription)
        {
            planet.SetSpaceportDescription(newDescription);
            mapData.SetChanged();
        }
    }
}



void PlanetView::ShipyardChanged()
{
    if(object && !object->GetPlanet().isEmpty())
    {
        vector<QString> list = ToList(shipyard->text());
        Planet &planet = mapData.Planets()[object->GetPlanet()];
        if(planet.Shipyard() != list)
        {
            planet.Shipyard() = list;
            mapData.SetChanged();
        }
    }
}



void PlanetView::OutfitterChanged()
{
    if(object && !object->GetPlanet().isEmpty())
    {
        vector<QString> list = ToList(outfitter->text());
        Planet &planet = mapData.Planets()[object->GetPlanet()];
        if(planet.Outfitter() != list)
        {
            planet.Outfitter() = list;
            mapData.SetChanged();
        }
    }
}



void PlanetView::ReputationChanged()
{
    if(object && !object->GetPlanet().isEmpty())
    {
        double value = numeric_limits<double>::quiet_NaN();
        if(!reputation->text().isEmpty())
            value = reputation->text().toDouble();
        Planet &planet = mapData.Planets()[object->GetPlanet()];
        if(planet.RequiredReputation() != value)
        {
            planet.SetRequiredReputation(value);
            mapData.SetChanged();
        }
    }
}



void PlanetView::BribeChanged()
{
    if(object && !object->GetPlanet().isEmpty())
    {
        double value = numeric_limits<double>::quiet_NaN();
        if(!bribe->text().isEmpty())
            value = bribe->text().toDouble();
        Planet &planet = mapData.Planets()[object->GetPlanet()];
        if(planet.Bribe() != value)
        {
            planet.SetBribe(value);
            mapData.SetChanged();
        }
    }
}



void PlanetView::SecurityChanged()
{
    if(object && !object->GetPlanet().isEmpty())
    {
        double value = numeric_limits<double>::quiet_NaN();
        if(!security->text().isEmpty())
            value = security->text().toDouble();
        Planet &planet = mapData.Planets()[object->GetPlanet()];
        if(planet.Security() != value)
        {
            planet.SetSecurity(value);
            mapData.SetChanged();
        }
    }
}



QString PlanetView::ToString(const vector<QString> &list)
{
    if(list.empty())
        return "";

    auto it = list.begin();
    QString result = *it;

    for(++it; it != list.end(); ++it)
        result += ", " + *it;

    return result;
}



vector<QString> PlanetView::ToList(const QString &str)
{
    vector<QString> result;

    QStringList strings = str.split(",", QString::SkipEmptyParts);
    for(const QString &token : strings)
        result.push_back(token.trimmed());

    return result;
}
