/* Planet.cpp
Copyright (c) 2014 by Michael Zahniser

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#include "Planet.h"

#include "DataNode.h"
#include "DataWriter.h"

#include <QString>
#include <QStringList>

#include <algorithm>
#include <cmath>

using namespace std;



// Load a planet's description from a file.
void Planet::Load(const DataNode &node)
{
    if(node.Size() < 2)
        return;
    name = node.Token(1);

    for(const DataNode &child : node)
    {
        if(child.Token(0) == "attributes")
        {
            for(int i = 1; i < child.Size(); ++i)
                attributes.push_back(child.Token(i));
        }
        else if(child.Token(0) == "landscape" && child.Size() >= 2)
            landscape = child.Token(1);
        else if(child.Token(0) == "music" && child.Size() >= 2)
            music = child.Token(1);
        else if(child.Token(0) == "description" && child.Size() >= 2)
        {
            if(!description.isEmpty() && !child.Token(1).isEmpty() && child.Token(1)[0] > ' ')
                description += '\t';
            description += child.Token(1);
            description += '\n';
        }
        else if(child.Token(0) == "spaceport" && child.Size() >= 2)
        {
            if(!spaceport.isEmpty() && !child.Token(1).isEmpty() && child.Token(1)[0] > ' ')
                spaceport += '\t';
            spaceport += child.Token(1);
            spaceport += '\n';
        }
        else if(child.Token(0) == "shipyard" && child.Size() >= 2)
            shipyard.push_back(child.Token(1));
        else if(child.Token(0) == "outfitter" && child.Size() >= 2)
            outfitter.push_back(child.Token(1));
        else if(child.Token(0) == "government" && child.Size() >= 2)
            government = child.Token(1);
        else if(child.Token(0) == "required reputation" && child.Size() >= 2)
            requiredReputation = child.Value(1);
        else if(child.Token(0) == "bribe" && child.Size() >= 2)
            bribe = child.Value(1);
        else if(child.Token(0) == "security" && child.Size() >= 2)
            security = child.Value(1);
        else if(child.Token(0) == "tribute" && child.Size() >= 2)
            LoadTribute(child);
        else
            unparsed.push_back(child);
    }
}



void  Planet::LoadTribute(const DataNode &node)
{
    if(node.Size() >= 2)
        tribute = node.Value(1);

    for(const DataNode &child : node)
    {
        if(child.Token(0) == "threshold" && child.Size() >= 2)
            tributeThreshold = child.Value(1);
        else if(child.Token(0) == "fleet" && child.Size() >= 3)
        {
            tributeFleetName = child.Token(1);
            tributeFleetQuantity = child.Value(2);
        }
        else
            tributeUnparsed.push_back(child);
    }
}



void Planet::Save(DataWriter &file) const
{
    file.Write("planet", name);
    file.BeginChild();
    {
        if(!attributes.empty())
        {
            file.WriteToken("attributes");
            for(const QString &it : attributes)
                file.WriteToken(it);
            file.Write();
        }
        if(!landscape.isEmpty())
            file.Write("landscape", landscape);
        if(!music.isEmpty())
            file.Write("music", music);

        // Break the descriptions into paragraphs.
        for(const QString &str : description.split('\n', Qt::SkipEmptyParts))
        {
            file.WriteToken("description");
            file.WriteToken(str, '`');
            file.Write();
        }
        for(const QString &str : spaceport.split('\n', Qt::SkipEmptyParts))
        {
            file.WriteToken("spaceport");
            file.WriteToken(str, '`');
            file.Write();
        }

        for(const QString &it : shipyard)
            file.Write("shipyard", it);
        for(const QString &it : outfitter)
            file.Write("outfitter", it);

        if(!government.isEmpty())
            file.Write("government", government);
        if(!std::isnan(requiredReputation))
            file.Write("required reputation", requiredReputation);
        if(!std::isnan(bribe))
            file.Write("bribe", bribe);
        if(!std::isnan(security))
            file.Write("security", security);
        if(!std::isnan(tribute))
        {
            file.Write("tribute", tribute);
            file.BeginChild();
            {
                file.Write("threshold", tributeThreshold);
                file.Write("fleet", tributeFleetName, tributeFleetQuantity);
                for(const DataNode &node : tributeUnparsed)
                    file.Write(node);
            }
            file.EndChild();
        }
        for(const DataNode &node : unparsed)
            file.Write(node);
    }
    file.EndChild();
}



// Get the name of the planet.
const QString &Planet::Name() const
{
    return name;
}



// Get the planet's descriptive text.
const QString &Planet::Description() const
{
    return description;
}



// Get the landscape sprite.
const QString &Planet::Landscape() const
{
    return landscape;
}



// Get the list of "attributes" of the planet.
const vector<QString> &Planet::Attributes() const
{
    return attributes;
}



// Check whether there is a spaceport (which implies there is also trading,
// jobs, banking, and hiring).
bool Planet::HasSpaceport() const
{
    return !spaceport.isEmpty();
}



// Get the spaceport's descriptive text.
const QString &Planet::SpaceportDescription() const
{
    return spaceport;
}



// Check if this planet has a shipyard.
bool Planet::HasShipyard() const
{
    return !shipyard.empty();
}



// Get the list of ships in the shipyard.
const vector<QString> &Planet::Shipyard() const
{
    return shipyard;
}



// Check if this planet has an outfitter.
bool Planet::HasOutfitter() const
{
    return !outfitter.empty();
}



// Get the list of outfits available from the outfitter.
const vector<QString> &Planet::Outfitter() const
{
    return outfitter;
}



// You need this good a reputation with this system's government to land here.
double Planet::RequiredReputation() const
{
    return requiredReputation;
}



// This is what fraction of your fleet's value you must pay as a bribe in
// order to land on this planet. (If zero, you cannot bribe it.)
double Planet::Bribe() const
{
    return bribe;
}



// This is how likely the planet's authorities are to notice if you are
// doing something illegal.
double Planet::Security() const
{
    return security;
}



double Planet::Tribute() const
{
    return tribute;
}



double Planet::TributeThreshold() const
{
    return tributeThreshold;
}



double Planet::TributeFleetQuantity() const
{
    return tributeFleetQuantity;
}



const QString &Planet::TributeFleetName() const

{
    return tributeFleetName;
}

void Planet::SetName(const QString &name)
{
    this->name = name;
}



void Planet::SetLandscape(const QString &sprite)
{
    landscape = sprite;
}



void Planet::SetDescription(const QString &text)
{
    description = text;
}



void Planet::SetSpaceportDescription(const QString &text)
{
    spaceport = text;
}



vector<QString> &Planet::Attributes()
{
    return attributes;
}



vector<QString> &Planet::Shipyard()
{
    return shipyard;
}



vector<QString> &Planet::Outfitter()
{
    return outfitter;
}



void Planet::SetRequiredReputation(double value)
{
    requiredReputation = value;
}



void Planet::SetBribe(double value)
{
    bribe = value;
}



void Planet::SetSecurity(double value)
{
    security = value;
}



void Planet::SetTribute(double value)
{
    tribute = value;
}



void Planet::SetTributeThreshold(double value)
{
    tributeThreshold = value;
}



void Planet::SetTributeFleetName(QString &value)
{
    tributeFleetName = value;
}



void Planet::SetTributeFleetQuantity(double value)
{
    tributeFleetQuantity = value;
}
