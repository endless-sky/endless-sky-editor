/* System.cpp
Copyright (c) 2014 by Michael Zahniser

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#include "System.h"

#include "DataNode.h"
#include "DataWriter.h"
#include "Planet.h"

#include <algorithm>
#include <limits>

using namespace std;



// Load a system's description.
void System::Load(const DataNode &node)
{
    if(node.Size() < 2)
        return;
    name = node.Token(1);

    habitable = numeric_limits<double>::quiet_NaN();

    for(const DataNode &child : node)
    {
        if(child.Token(0) == "pos" && child.Size() >= 3)
            position = QVector2D(child.Value(1), child.Value(2));
        else if(child.Token(0) == "government" && child.Size() >= 2)
            government = child.Token(1);
        else if(child.Token(0) == "habitable" && child.Size() >= 2)
            habitable = child.Value(1);
        else if(child.Token(0) == "link" && child.Size() >= 2)
            links.push_back(child.Token(1));
        else if(child.Token(0) == "asteroids" && child.Size() >= 4)
            asteroids.push_back({child.Token(1), static_cast<int>(child.Value(2)), child.Value(3)});
        else if(child.Token(0) == "trade" && child.Size() >= 3)
            trade[child.Token(1)] = child.Value(2);
        else if(child.Token(0) == "fleet" && child.Size() >= 3)
            fleets.push_back({child.Token(1), static_cast<int>(child.Value(2))});
        else if(child.Token(0) == "object")
            LoadObject(child);
        else
            unparsed.push_back(child);
    }
}



void System::Save(DataWriter &file) const
{
    file.Write("system", name);
    file.BeginChild();
    {
        file.Write("pos", position.x(), position.y());
        if(!government.isEmpty())
            file.Write("government", government);
        file.Write("habitable", habitable);
        for(const QString &it : links)
            file.Write("link", it);
        for(const Asteroid &it : asteroids)
            file.Write("asteroids", it.type, it.count, it.energy);
        for(const auto &it : trade)
            file.Write("trade", it.first, it.second);
        for(const Fleet &it : fleets)
            file.Write("fleet", it.name, it.period);
        for(const DataNode &node : unparsed)
            file.Write(node);
        for(const StellarObject &object : objects)
            SaveObject(file, object);
    }
    file.EndChild();
}



// Get this system's name and position (in the star map).
const QString &System::Name() const
{
    return name;
}



const QVector2D &System::Position() const
{
    return position;
}



// Get this system's government.
const QString &System::Government() const
{
    return government;
}



// Get a list of systems you can travel to through hyperspace from here.
const vector<QString> &System::Links() const
{
    return links;
}



// Get the stellar object locations on the most recently set date.
vector<StellarObject> &System::Objects()
{
    return objects;
}



// Get the stellar object locations on the most recently set date.
const vector<StellarObject> &System::Objects() const
{
    return objects;
}



// Get the habitable zone's center.
double System::HabitableZone() const
{
    return habitable;
}



// Get the radius of the zone occupied by the given stellar object. This
// zone includes the object and anything that orbits around it. If this
// object is in orbit around something else, this function returns 0.
double System::OccupiedRadius(const StellarObject &object)
{
    // Make sure the object is part of this system and is a primary object.
    if(&object < &objects.front() || &object > &objects.back() || object.Parent() >= 0)
        return 0.;

    double radius = object.Radius();
    int index = &object - &objects.front();
    for(const StellarObject &other : objects)
        if(other.Parent() == index)
            radius = max(radius, other.Distance() + other.Radius());

    return radius;
}



// Get the specification of how many asteroids of each type there are.
const vector<System::Asteroid> &System::Asteroids() const
{
    return asteroids;
}



// Get the price of the given commodity in this system.
int System::Trade(const QString &commodity) const
{
    auto it = trade.find(commodity);
    return (it == trade.end()) ? 0 : it->second;
}



// Get the probabilities of various fleets entering this system.
const vector<System::Fleet> &System::Fleets() const
{
    return fleets;
}



// Position the planets, etc.
void System::SetDay(double day)
{
    for(StellarObject &object : objects)
    {
        double angle = !object.period ? 0. : day * 360. / object.period + object.offset;
        angle *= M_PI / 180.;

        object.position = object.distance * QVector2D(sin(angle), -cos(angle));

        // Because of the order of the vector, the parent's position has always
        // been updated before this loop reaches any of its children, so:
        if(object.parent >= 0)
            object.position += objects[object.parent].position;
    }
}



void System::LoadObject(const DataNode &node, int parent)
{
    int index = objects.size();
    objects.push_back(StellarObject());
    StellarObject &object = objects.back();
    object.parent = parent;

    if(node.Size() >= 2)
        object.planet = node.Token(1);

    for(const DataNode &child : node)
    {
        if(child.Token(0) == "sprite" && child.Size() >= 2)
            object.sprite = child.Token(1);
        else if(child.Token(0) == "distance" && child.Size() >= 2)
            object.distance = child.Value(1);
        else if(child.Token(0) == "period" && child.Size() >= 2)
            object.period = child.Value(1);
        else if(child.Token(0) == "offset" && child.Size() >= 2)
            object.offset = child.Value(1);
        else if(child.Token(0) == "object")
            LoadObject(child, index);
        else
            object.unparsed.push_back(child);
    }
}



void System::SaveObject(DataWriter &file, const StellarObject &object) const
{
    int level = 0;
    int parent = object.parent;
    while(parent >= 0)
    {
        file.BeginChild();
        ++level;
        parent = objects[parent].parent;
    }
    if(!object.planet.isEmpty())
        file.Write("object", object.planet);
    else
        file.Write("object");
    file.BeginChild();
    {
        if(!object.sprite.isEmpty())
            file.Write("sprite", object.sprite);
        if(object.distance)
            file.Write("distance", object.distance);
        if(object.period)
            file.Write("period", object.period);
        if(object.offset)
            file.Write("offset", object.offset);
        for(const DataNode &node : unparsed)
            file.Write(node);
    }
    file.EndChild();
    while(level--)
        file.EndChild();
}



void System::Init(const QString &name, const QVector2D &position)
{
    this->name = name;
    this->position = position;

    // TODO: initialize with random planets, asteroids, etc.
}



void System::SetName(const QString &name)
{
    this->name = name;
}



void System::SetPosition(const QVector2D &pos)
{
    position = pos;
}



void System::SetGovernment(const QString &gov)
{
    government = gov;
}



void System::ToggleLink(System *other)
{
    if(!other || other == this)
        return;

    auto it = find(links.begin(), links.end(), other->name);
    auto oit = find(other->links.begin(), other->links.end(), name);
    if(it == links.end())
    {
        links.push_back(other->name);
        if(oit == other->links.end())
            other->links.push_back(name);
    }
    else
    {
        links.erase(it);
        if(oit != other->links.end())
            other->links.erase(oit);
    }
}



void System::ChangeLink(const QString &from, const QString &to)
{
    auto it = find(links.begin(), links.end(), from);
    if(it != links.end())
        *it = to;
}
