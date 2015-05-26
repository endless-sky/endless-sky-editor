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
#include "pi.h"
#include "Planet.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <set>

using namespace std;

namespace {
    static const double MIN_GAP = 50.;
    static const int RANDOM_GAP = 100;
    static const double MIN_MOON_GAP = 10.;
    static const int RANDOM_MOON_GAP = 50;
    static const double STAR_MASS_SCALE = .25;
    static const double PLANET_MASS_SCALE = .015;
    static const double HABITABLE_SCALE = 6.25;

    static const int RANDOM_STAR_DISTANCE = 40;
    static const double MIN_STAR_DISTANCE = 40.;
}



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
            if(!it.name.isEmpty() && it.period)
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
double System::OccupiedRadius(const StellarObject &object) const
{
    // Make sure the object is part of this system and is a primary object.
    if(&object < &objects.front() || &object > &objects.back() || object.Parent() >= 0 || object.IsStar())
        return 0.;

    double radius = object.Radius();
    int index = &object - &objects.front();
    for(const StellarObject &other : objects)
        if(other.Parent() == index)
            radius = max(radius, other.Distance() + other.Radius());

    return radius;
}



double System::OccupiedRadius() const
{
    if(objects.empty())
        return 0.;

    double radius = objects.back().Radius() + objects.back().Distance();
    if(objects.back().Parent() >= 0)
        radius += objects[objects.back().Parent()].Distance();

    return radius;
}



double System::StarRadius() const
{
    double radius = 0.;
    for(const StellarObject &other : objects)
    {
        if(!other.IsStar())
            break;
        radius = max(radius, other.Distance() + other.Radius());
    }
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
vector<System::Fleet> &System::Fleets()
{
    return fleets;
}



const vector<System::Fleet> &System::Fleets() const
{
    return fleets;
}



// Position the planets, etc.
void System::SetDay(double day)
{
    timeStep = day;
    for(StellarObject &object : objects)
    {
        double angle = (!object.period ? 0. : day * 360. / object.period) + object.offset;
        angle *= TO_RAD;

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

    Randomize(true, false);
    ChangeAsteroids();
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



void System::SetTrade(const QString &commodity, int value)
{
    trade[commodity] = value;
}



void System::Move(StellarObject *object, double dDistance, double dAngle)
{
    if(!object || !object->period || object->IsStar())
        return;

    // Find the next object in from this object. Determine what the orbital
    // radius of that object is. Don't allow objects too close together.
    auto it = objects.begin() + (object - &objects.front());
    auto root = (it->Parent() >= 0 ? objects.begin() + it->Parent() : it);
    if(root != objects.begin())
    {
        auto previous = root - 1;
        while(previous != objects.begin() && previous->Parent() >= 0)
            --previous;

        double previousOccupied = previous->IsStar() ?
            StarRadius() : (OccupiedRadius(*previous) + previous->Distance());
        double rootOccupied = root->Distance() - OccupiedRadius(*root);

        double gap = rootOccupied - previousOccupied;
        double sign = (it == root ? 1. : -1.);
        gap += sign * dDistance;
        if(gap < MIN_GAP)
            dDistance += sign * (MIN_GAP - gap);
    }
    // If this is a moon, we also have to make sure it does not collide with
    // whatever planet is in one space from it.
    if(object->Parent() >= 0)
    {
        double previousOccupied = it[-1].Radius();
        if(it[-1].Parent() >= 0)
            previousOccupied += it[-1].Distance();

        double thisOccupied = it->Distance() - it->Radius();
        double gap = thisOccupied + dDistance - previousOccupied;
        if(gap < MIN_MOON_GAP)
            dDistance += MIN_MOON_GAP - gap;
    }

    object->offset -= dAngle;

    // If a root object is moved, every root object beyond it must be moved in
    // or out by whatever amount its radius changed by. If a child object was
    // moved, all the children after it must be moved by that amount and then
    // all root objects after it must be moved by twice that amount.
    for( ; it != objects.end(); ++it)
        if(it->Parent() < 0 || it->Parent() == object->Parent())
        {
            it->distance += dDistance;
            Recompute(*it);
        }
}



void System::ChangeAsteroids()
{
    asteroids.clear();

    // Pick the total number of asteroids. Bias towards small numbers, with
    // a few systems with many more.
    int fullTotal = (rand() % 21) * (rand() % 21) + 1;
    double energy = (rand() % 21 + 10) * (rand() % 21 + 10) * .01;
    const QString suffix[2] = {" rock", " metal"};
    const QString prefix[3] = {"small", "medium", "large"};

    int total[2] = {rand() % fullTotal, 0};
    total[1] = fullTotal - total[0];

    for(int i = 0; i < 2; ++i)
    {
        if(!total[i])
            continue;

        int count[3] = {0, rand() % total[i], 0};
        int remaining = total[i] - count[1];
        if(remaining)
        {
            count[0] = rand() % remaining;
            count[2] = remaining - count[0];
        }

        for(int j = 0; j < 3; ++j)
            if(count[j])
                asteroids.push_back({
                    prefix[j] + suffix[i],
                    count[j],
                    energy * (rand() % 101 + 50) * .01});
    }
}



void System::ChangeStar()
{
    double oldStarRadius = StarRadius();
    unsigned oldStars = 0;
    while(!objects.empty() && objects.front().IsStar())
    {
        objects.erase(objects.begin());
        ++oldStars;
    }

    // If the number of stars is changing, all parent indices change.
    unsigned stars = 1 + !(rand() % 3);
    if(stars != oldStars)
        for(StellarObject &object : objects)
            if(object.parent >= 0)
                object.parent += stars - oldStars;

    double mass = 0.;
    if(stars == 1)
    {
        StellarObject star = StellarObject::Star();
        star.period = 10.;
        mass = pow(star.Radius(), 3.) * STAR_MASS_SCALE;

        objects.insert(objects.begin(), star);
    }
    else
    {
        StellarObject first = StellarObject::Star();
        StellarObject second = StellarObject::Star();
        first.offset = 0.;
        second.offset = 180.;

        double firstR = first.Radius();
        double secondR = second.Radius();
        double firstMass = pow(firstR, 3.) * STAR_MASS_SCALE;
        double secondMass = pow(secondR, 3.) * STAR_MASS_SCALE;
        mass = firstMass + secondMass;

        double distance = firstR + secondR + (rand() % RANDOM_STAR_DISTANCE) + MIN_STAR_DISTANCE;
        // m1 * d1 = m2 * d2
        // d1 + d2 = d;
        // m1 * d1 = m2 * (d - d1)
        // m1 * d1 = m2 * d - m2 * d1
        // (m1 + m2) * d1 = m2 * d
        double firstD = (secondMass * distance) / mass;
        double secondD = (firstMass * distance) / mass;
        first.distance = firstD;
        second.distance = secondD;

        double period = sqrt(pow(distance, 3.) / mass);
        first.period = period;
        second.period = period;

        objects.insert(objects.begin(), (firstD < secondD) ? second : first);
        objects.insert(objects.begin(), (firstD < secondD) ? first : second);
    }
    habitable = mass / HABITABLE_SCALE;

    if(objects.size() > stars)
    {
        double newStarRadius = StarRadius();
        Move(&objects[stars], newStarRadius - oldStarRadius);
    }
}



void System::ChangeSprite(StellarObject *object)
{
    if(!object || object < &objects.front() || object > &objects.back())
        return;

    StellarObject newObject;
    set<QString> used = Used();
    do {
        if(object->IsStation())
            newObject = StellarObject::Station();
        else if(object->IsMoon())
            newObject = StellarObject::Moon();
        else if(object->IsGiant())
            newObject = StellarObject::Giant();
        else
        {
            double distance = (object->Parent() >= 0 ? objects[object->Parent()].Distance() : object->Distance());
            if(distance >= .5 * habitable && distance < 2. * habitable)
                newObject = StellarObject::Planet();
            else
                newObject = StellarObject::Uninhabited();
        }
    } while(used.find(newObject.Sprite()) != used.end());

    // Check how much the radius will change by, then change the sprite.
    double radiusChange = newObject.Radius() - object->Radius();
    object->sprite = newObject.sprite;

    // If this object has a parent:
    // this distance += dRadius
    // children after distance += 2 * dRadius
    // parent distance += 2 * dRadius
    // after distance += 2 * dRadius
    // Otherwise:
    // this distance += dRadius
    // child distance += dRadius
    // after distance += 2 * dRadius

    // Get the index of this object, for checking if other objects are children.
    //int index = object - &objects.front();

    // Get an iterator to this object.
    auto it = objects.begin() + (object - &*objects.begin());
    // Move this object out by an amount equal to the radius change.
    it->distance += radiusChange;
    Recompute(*it);

    // If this object has a parent, the parent's occupied radius will be
    // expanding by twice the radius change, so it must move out by that amount.
    // In addition, root objects beyond this one's parent will be moving out by
    // four times the radius change, instead of two.
    if(object->Parent() >= 0)
    {
        radiusChange *= 2.;
        objects[object->Parent()].distance += radiusChange;
        Recompute(objects[object->Parent()]);
    }
    // The objects that will be affected are: children of this object, and all
    // "root" objects outside of this one.
    bool isChild = true;
    for(++it; it != objects.end(); ++it)
    {
        if(isChild && it->Parent() < 0)
        {
            isChild = false;
            radiusChange *= 2.;
        }
        if(isChild || it->Parent() < 0)
        {
            it->distance += radiusChange;
            Recompute(*it);
        }
    }
}



void System::AddPlanet()
{
    // The spacing between planets grows exponentially.
    int randomPlanetSpace = RANDOM_GAP;
    for(const StellarObject &object : objects)
        if(!object.IsStar() && object.Parent() < 0)
            randomPlanetSpace += randomPlanetSpace / 2;

    double distance = OccupiedRadius();
    int space = rand() % randomPlanetSpace;
    distance += (space * space) * .01 + MIN_GAP;

    set<QString> used = Used();

    StellarObject root;
    int rootIndex = objects.size();
    bool isHabitable = (distance > habitable * .5 && distance < habitable * 2. - 120.);
    bool isSmall = !(rand() % 10);
    bool isTerrestrial = !isSmall && (rand() % 2000 > distance);
    // Occasionally, moon-sized objects can be root objects. Otherwise, pick a
    // giant or a normal planet, with giants more frequent in the outer parts
    // of the solar system.
    do {
        if(isSmall)
            root = StellarObject::Moon();
        else if(isTerrestrial)
            root = isHabitable ? StellarObject::Planet() : StellarObject::Uninhabited();
        else
            root = StellarObject::Giant();
    } while(used.find(root.Sprite()) != used.end());
    objects.push_back(root);
    used.insert(root.Sprite());

    int moonCount = rand() % (isTerrestrial ? (rand() % 2 + 1) : (rand() % 3 + 3));
    if(root.Radius() < 70)
        moonCount = 0;

    double moonDistance = root.Radius();
    int randomMoonSpace = RANDOM_MOON_GAP;
    for(int i = 0; i < moonCount; ++i)
    {
        moonDistance += rand() % randomMoonSpace + MIN_MOON_GAP;
        // Each moon, on average, should be spaced more widely than the one before.
        randomMoonSpace += 20;

        StellarObject moon;
        do {
            moon = StellarObject::Moon();
        } while(used.find(moon.Sprite()) != used.end());
        used.insert(moon.Sprite());

        moon.distance = moonDistance + moon.Radius();
        moon.parent = rootIndex;
        Recompute(moon, false);
        objects.push_back(moon);
        moonDistance += 2. * moon.Radius();
    }
    objects[rootIndex].distance = distance + moonDistance;
    Recompute(objects[rootIndex], false);
}



void System::AddMoon(StellarObject *object, bool isStation)
{
    if(!object || object < &objects.front() || object > &objects.back())
        return;

    double originalMoonDistance = object->Radius();
    int randomMoonSpace = RANDOM_MOON_GAP;
    int rootIndex = object - &objects.front();
    auto it = objects.begin() + rootIndex + 1;
    while(it != objects.end() && it->Parent() == rootIndex)
    {
        randomMoonSpace += 20;
        originalMoonDistance = it->Distance() + it->Radius();
        ++it;
    }

    double moonDistance = originalMoonDistance + rand() % randomMoonSpace + MIN_MOON_GAP;
    set<QString> used = Used();
    StellarObject moon;
    do {
        moon = isStation ? StellarObject::Station() : StellarObject::Moon();
    } while(used.find(moon.Sprite()) != used.end());

    moon.distance = moonDistance + moon.Radius();
    moon.parent = rootIndex;
    Recompute(moon, false);

    // Move the next root planet out from this one farther out.
    double distanceIncrease = moonDistance + 2. * moon.Radius() - originalMoonDistance;
    if(it != objects.end())
        Move(&*it, 2. * distanceIncrease);

    // Move this root planet farther out.
    objects[rootIndex].distance += distanceIncrease;
    Recompute(objects[rootIndex]);

    // Insert the new moon, then update the parent indices of all moons farther
    // out than this one (because their parents' indices have changed).
    it = objects.insert(it, moon);
    for( ; it != objects.end(); ++it)
        if(it->parent > rootIndex)
            ++it->parent;
}



void System::Randomize(bool allowHabitable, bool requireHabitable)
{
    // Try to create a system satisfying the given parameters.
    for(int i = 0; i < 100; ++i)
    {
        objects.clear();
        ChangeStar();
        while(OccupiedRadius() < 2000.)
            AddPlanet();

        bool isInhabited = false;
        bool isHabitable = false;
        for(const StellarObject &object : objects)
        {
            isInhabited |= object.IsInhabited();
            double d = object.Distance();
            isHabitable |= object.Parent() < 0 && object.IsTerrestrial()
                && d > .5 * habitable && d < 2. * habitable;
        }
        if(isInhabited && !allowHabitable)
            continue;
        if(!isHabitable && requireHabitable)
            continue;
        break;
    }
}



void System::Delete(StellarObject *object)
{
    if(!object || objects.empty())
        return;

    int index = object - &objects.front();
    if(index < 0 || static_cast<unsigned>(index) >= objects.size())
        return;

    double shrink = object->Radius();

    auto it = objects.begin() + index;
    auto end = it + 1;
    while(end != objects.end() && end->Parent() == index)
    {
        shrink = max(shrink, end->Distance() + end->Radius());
        ++end;
    }
    int parentShift = end - it;
    objects.erase(it, end);

    it = objects.begin() + index;
    if(it == objects.end())
        return;

    Move(&*it, -2. * shrink);
    for( ; it != objects.end(); ++it)
        if(it->parent >= 0)
            it->parent -= parentShift;
}



void System::Recompute(StellarObject &object, bool updateOffset)
{
    double mass = habitable * HABITABLE_SCALE;
    if(object.Parent() >= 0.)
        mass = pow(objects[object.Parent()].Radius(), 3.) * PLANET_MASS_SCALE;

    double newPeriod = sqrt(pow(object.distance, 3) / mass);
    if(updateOffset)
    {
        double delta = timeStep / object.period - timeStep / newPeriod;
        object.offset += 360. * (delta - floor(delta));
        object.offset = fmod(object.offset, 360.);
    }
    object.period = newPeriod;
}



set<QString> System::Used() const
{
    set<QString> used;
    for(const StellarObject &object : objects)
        used.insert(object.Sprite());
    return used;
}
