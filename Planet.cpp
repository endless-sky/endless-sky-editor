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

#include <algorithm>
#include <limits>

using namespace std;



// Load a planet's description from a file.
void Planet::Load(const DataNode &node)
{
    if(node.Size() < 2)
        return;
    name = node.Token(1);

    // Use NaN to mark optional fields.
    requiredReputation = numeric_limits<double>::quiet_NaN();
    bribe = numeric_limits<double>::quiet_NaN();
    security = numeric_limits<double>::quiet_NaN();

    for(const DataNode &child : node)
    {
        if(child.Token(0) == "landscape" && child.Size() >= 2)
            landscape = child.Token(1);
        else if(child.Token(0) == "attributes")
        {
            for(int i = 1; i < child.Size(); ++i)
                attributes.push_back(child.Token(i));
        }
        else if(child.Token(0) == "description" && child.Size() >= 2)
        {
            if(!description.empty() && !child.Token(1).empty() && child.Token(1)[0] > ' ')
                description += '\t';
            description += child.Token(1);
            description += '\n';
        }
        else if(child.Token(0) == "spaceport" && child.Size() >= 2)
        {
            if(!spaceport.empty() && !child.Token(1).empty() && child.Token(1)[0] > ' ')
                spaceport += '\t';
            spaceport += child.Token(1);
            spaceport += '\n';
        }
        else if(child.Token(0) == "shipyard" && child.Size() >= 2)
            shipyard.push_back(child.Token(1));
        else if(child.Token(0) == "outfitter" && child.Size() >= 2)
            outfitter.push_back(child.Token(1));
        else if(child.Token(0) == "required reputation" && child.Size() >= 2)
            requiredReputation = child.Value(1);
        else if(child.Token(0) == "bribe" && child.Size() >= 2)
            bribe = child.Value(1);
        else if(child.Token(0) == "security" && child.Size() >= 2)
            security = child.Value(1);
    }
}



// Get the name of the planet.
const string &Planet::Name() const
{
    return name;
}



// Get the planet's descriptive text.
const string &Planet::Description() const
{
    return description;
}



// Get the landscape sprite.
const string &Planet::Landscape() const
{
    return landscape;
}



// Get the list of "attributes" of the planet.
const vector<string> &Planet::Attributes() const
{
    return attributes;
}



// Check whether there is a spaceport (which implies there is also trading,
// jobs, banking, and hiring).
bool Planet::HasSpaceport() const
{
    return !spaceport.empty();
}



// Get the spaceport's descriptive text.
const string &Planet::SpaceportDescription() const
{
    return spaceport;
}



// Check if this planet has a shipyard.
bool Planet::HasShipyard() const
{
    return !shipyard.empty();
}



// Get the list of ships in the shipyard.
const vector<string> &Planet::Shipyard() const
{
    return shipyard;
}



// Check if this planet has an outfitter.
bool Planet::HasOutfitter() const
{
    return !outfitter.empty();
}



// Get the list of outfits available from the outfitter.
const vector<string> &Planet::Outfitter() const
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
