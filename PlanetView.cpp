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

#include <limits>

using namespace std;



PlanetView::PlanetView(Map &mapData, QWidget *parent) :
    QWidget(parent), mapData(mapData)
{
    name = new QLineEdit;
    connect(name, SIGNAL(editingFinished()), this, SLOT(NameChanged()));
    attributes = new QLineEdit;
    connect(attributes, SIGNAL(editingFinished()), this, SLOT(AttributesChanged()));

    landscape = new LandscapeView(this);
    landscape->setMinimumHeight(360);
    landscape->setMaximumHeight(360);

    description = new QPlainTextEdit;
    description->setTabStopWidth(20);
    connect(description, SIGNAL(textChanged()), this, SLOT(DescriptionChanged()));
    spaceport = new QPlainTextEdit;
    spaceport->setTabStopWidth(20);
    connect(spaceport, SIGNAL(textChanged()), this, SLOT(SpaceportDescriptionChanged()));

    shipyard = new QLineEdit;
    connect(shipyard, SIGNAL(editingFinished()), this, SLOT(ShipyardChanged()));
    outfitter = new QLineEdit;
    connect(outfitter, SIGNAL(editingFinished()), this, SLOT(OutfitterChanged()));

    reputation = new QLineEdit;
    reputation->setValidator(new QRegExpValidator(QRegExp("-?\\d*\\.?\\d*"), reputation));
    connect(reputation, SIGNAL(editingFinished()), this, SLOT(ReputationChanged()));
    bribe = new QLineEdit;
    bribe->setValidator(new QRegExpValidator(QRegExp("0||0?\\.\\d*"), bribe));
    connect(bribe, SIGNAL(editingFinished()), this, SLOT(BribeChanged()));
    security = new QLineEdit;
    security->setValidator(new QRegExpValidator(QRegExp("0||0?\\.\\d*"), security));
    connect(security, SIGNAL(editingFinished()), this, SLOT(SecurityChanged()));

    QGridLayout *layout = new QGridLayout;
    int row = 0;

    layout->addWidget(new QLabel("Planet:"), row, 0);
    layout->addWidget(name, row++, 1);
    layout->addWidget(new QLabel("Attributes:"), row, 0);
    layout->addWidget(attributes, row++, 1);

    layout->addWidget(landscape, row++, 0, 1, 2);

    layout->addWidget(description, row++, 0, 1, 2);
    layout->addWidget(new QLabel("Spaceport description:"), row++, 0, 1, 2);
    layout->addWidget(spaceport, row++, 0, 1, 2);

    layout->addWidget(new QLabel("Shipyard:"), row, 0);
    layout->addWidget(shipyard, row++, 1);
    layout->addWidget(new QLabel("Outfitter:"), row, 0);
    layout->addWidget(outfitter, row++, 1);

    QWidget *box = new QWidget;
    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->addWidget(new QLabel("Required reputation:"));
    hLayout->addWidget(reputation);
    hLayout->addWidget(new QLabel("Bribe:"));
    hLayout->addWidget(bribe);
    hLayout->addWidget(new QLabel("Security:"));
    hLayout->addWidget(security);
    hLayout->addStretch();
    box->setLayout(hLayout);
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
        description->setPlainText(it->second.Description());
        spaceport->setPlainText(it->second.SpaceportDescription());
        shipyard->setText(ToString(it->second.Shipyard()));
        outfitter->setText(ToString(it->second.Outfitter()));
        reputation->setText(QString::number(it->second.RequiredReputation()));
        bribe->setText(QString::number(it->second.Bribe()));
        security->setText(QString::number(it->second.Security()));
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
        planet.Attributes() = ToList(attributes->text());
        planet.SetLandscape(landscape->Landscape());
        planet.SetDescription(description->toPlainText());
        planet.SetSpaceportDescription(spaceport->toPlainText());
        if(!reputation->text().isEmpty())
            planet.SetRequiredReputation(reputation->text().toDouble());
        if(!bribe->text().isEmpty())
            planet.SetBribe(bribe->text().toDouble());
        if(!security->text().isEmpty())
            planet.SetSecurity(security->text().toDouble());
    }
}



void PlanetView::AttributesChanged()
{
    if(object && !object->GetPlanet().isEmpty())
        mapData.Planets()[object->GetPlanet()].Attributes() = ToList(attributes->text());
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



void PlanetView::ShipyardChanged()
{
    if(object && !object->GetPlanet().isEmpty())
        mapData.Planets()[object->GetPlanet()].Shipyard() = ToList(shipyard->text());
}



void PlanetView::OutfitterChanged()
{
    if(object && !object->GetPlanet().isEmpty())
        mapData.Planets()[object->GetPlanet()].Outfitter() = ToList(outfitter->text());
}



void PlanetView::ReputationChanged()
{
    if(object && !object->GetPlanet().isEmpty())
        mapData.Planets()[object->GetPlanet()].SetRequiredReputation(
            reputation->text().isEmpty() ? numeric_limits<double>::quiet_NaN() : reputation->text().toDouble());
}



void PlanetView::BribeChanged()
{
    if(object && !object->GetPlanet().isEmpty())
        mapData.Planets()[object->GetPlanet()].SetBribe(
            bribe->text().isEmpty() ? numeric_limits<double>::quiet_NaN() : bribe->text().toDouble());
}



void PlanetView::SecurityChanged()
{
    if(object && !object->GetPlanet().isEmpty())
        mapData.Planets()[object->GetPlanet()].SetSecurity(
            security->text().isEmpty() ? numeric_limits<double>::quiet_NaN() : security->text().toDouble());
}



QString PlanetView::ToString(const std::vector<QString> &list)
{
    if(list.empty())
        return "";

    auto it = list.begin();
    QString result = *it;

    for(++it; it != list.end(); ++it)
        result += ", " + *it;

    return result;
}



std::vector<QString> PlanetView::ToList(const QString &str)
{
    vector<QString> result;

    QStringList strings = str.split(",", QString::SkipEmptyParts);
    for(const QString &token : strings)
        result.push_back(token.trimmed());

    return result;
}
