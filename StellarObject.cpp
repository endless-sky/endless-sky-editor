/* StellarObject.cpp
Copyright (c) 2014 by Michael Zahniser

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#include "StellarObject.h"

#include "Planet.h"

#include <algorithm>
#include <cstdint>
#include <map>
#include <string>

using namespace std;

namespace {
    static const int MOON_RADIUS = 50;
    static const int GIANT_RADIUS = 120;

    struct Info { int radius; int info; };
    const map<QString, Info> INFO = {
        {"planet/callisto", {47, 0}},
        {"planet/cloud0", {76, 0}},
        {"planet/cloud1", {100, 0}},
        {"planet/cloud2", {101, 0}},
        {"planet/cloud3", {82, 0}},
        {"planet/cloud4", {91, 0}},
        {"planet/cloud5", {116, 0}},
        {"planet/cloud6", {86, 0}},
        {"planet/cloud7", {70, 0}},
        {"planet/cloud8", {77, 0}},
        {"planet/desert0", {75, 0}},
        {"planet/desert1", {96, 0}},
        {"planet/desert2", {81, 0}},
        {"planet/desert3", {85, 0}},
        {"planet/desert4", {33, 0}},
        {"planet/desert5", {82, 0}},
        {"planet/desert6", {85, 0}},
        {"planet/desert7", {64, 0}},
        {"planet/desert8", {74, 0}},
        {"planet/desert9", {66, 0}},
        {"planet/desert10", {86, 0}},
        {"planet/dust0", {28, 0}},
        {"planet/dust1", {42, 0}},
        {"planet/dust2", {51, 0}},
        {"planet/dust3", {37, 0}},
        {"planet/dust4", {42, 0}},
        {"planet/dust5", {47, 0}},
        {"planet/dust6", {57, 0}},
        {"planet/dust7", {37, 0}},
        {"planet/earth", {86, 2}},
        {"planet/europa", {31, 0}},
        {"planet/fog0", {107, 0}},
        {"planet/forest0", {97, 2}},
        {"planet/forest1", {81, 2}},
        {"planet/forest2", {90, 2}},
        {"planet/forest3", {71, 2}},
        {"planet/forest4", {94, 2}},
        {"planet/forest5", {83, 2}},
        {"planet/forest6", {69, 2}},
        {"planet/ganymede", {52, 0}},
        {"planet/gas0", {198, 0}},
        {"planet/gas1", {161, 0}},
        {"planet/gas2", {122, 0}},
        {"planet/gas3", {217, 0}},
        {"planet/gas4", {175, 0}},
        {"planet/gas5", {182, 0}},
        {"planet/gas6", {217, 0}},
        {"planet/gas7", {174, 0}},
        {"planet/gas8", {133, 0}},
        {"planet/gas9", {168, 0}},
        {"planet/gas10", {134, 0}},
        {"planet/gas11", {183, 0}},
        {"planet/gas12", {213, 0}},
        {"planet/gas13", {203, 0}},
        {"planet/gas14", {159, 0}},
        {"planet/gas15", {134, 0}},
        {"planet/gas16", {134, 0}},
        {"planet/gas17", {154, 0}},
        {"planet/ice0", {37, 0}},
        {"planet/ice1", {97, 0}},
        {"planet/ice2", {75, 0}},
        {"planet/ice3", {90, 0}},
        {"planet/ice4", {75, 0}},
        {"planet/ice5", {88, 0}},
        {"planet/ice6", {75, 0}},
        {"planet/ice7", {47, 0}},
        {"planet/ice8", {37, 0}},
        {"planet/io", {36, 0}},
        {"planet/jupiter", {189, 0}},
        {"planet/lava0", {48, 0}},
        {"planet/lava1", {53, 0}},
        {"planet/lava2", {50, 0}},
        {"planet/lava3", {64, 0}},
        {"planet/lava4", {74, 0}},
        {"planet/lava5", {66, 0}},
        {"planet/lava6", {56, 0}},
        {"planet/lava7", {60, 0}},
        {"planet/luna", {38, 0}},
        {"planet/mars", {72, 0}},
        {"planet/mercury", {53, 0}},
        {"planet/miranda", {33, 0}},
        {"planet/neptune", {139, 0}},
        {"planet/oberon", {28, 0}},
        {"planet/ocean0", {77, 1}},
        {"planet/ocean1", {87, 1}},
        {"planet/ocean2", {96, 1}},
        {"planet/ocean3", {81, 1}},
        {"planet/ocean4", {93, 1}},
        {"planet/ocean5", {77, 1}},
        {"planet/ocean6", {101, 1}},
        {"planet/ocean7", {82, 1}},
        {"planet/ocean8", {95, 1}},
        {"planet/ocean9", {86, 1}},
        {"planet/rhea", {43, 0}},
        {"planet/rock0", {37, 0}},
        {"planet/rock1", {79, 0}},
        {"planet/rock2", {83, 0}},
        {"planet/rock3", {37, 0}},
        {"planet/rock4", {93, 0}},
        {"planet/rock5", {60, 0}},
        {"planet/rock6", {75, 0}},
        {"planet/rock7", {32, 0}},
        {"planet/rock8", {65, 0}},
        {"planet/rock9", {74, 0}},
        {"planet/rock10", {98, 0}},
        {"planet/rock11", {55, 0}},
        {"planet/rock12", {85, 0}},
        {"planet/rock13", {75, 0}},
        {"planet/rock14", {46, 0}},
        {"planet/rock15", {56, 0}},
        {"planet/rock16", {71, 0}},
        {"planet/rock17", {28, 0}},
        {"planet/rock18", {75, 0}},
        {"planet/rock19", {72, 0}},
        {"planet/tethys", {28, 0}},
        {"planet/titan", {54, 0}},
        {"planet/uranus", {154, 0}},
        {"planet/venus", {80, 0}},
        {"planet/water0", {56, 1}},
        {"planet/water1", {96, 1}},


        {"planet/station0", {30, 3}},
        {"planet/station1", {30, 3}},
        {"planet/station1k", {30, 3}},
        {"planet/station1kd", {30, 3}},
        {"planet/station2", {35, 3}},
        {"planet/station2k", {35, 3}},
        {"planet/station2kd", {35, 3}},
        {"planet/station3", {35, 3}},
        {"planet/station3k", {45, 3}},
        {"planet/station3kd", {45, 3}},
        {"planet/station4", {45, 3}},
        {"planet/station5", {55, 3}},
        {"planet/station6", {65, 3}},
        {"planet/station7", {65, 3}},

        {"planet/wisp", {96, 3}},
        {"planet/wormhole", {120, 3}},

        {"planet/ringworld", {20, 3}},
        {"planet/ringworld left", {20, 3}},
        {"planet/ringworld right", {20, 3}},

        {"star/b5", {60, 1}},
        {"star/a0", {50, 1}},
        {"star/a5", {45, 2}},
        {"star/f0", {39, 3}},
        {"star/f5", {35, 8}},
        {"star/g0", {30, 12}},
        {"star/g5", {25, 14}},
        {"star/k0", {23, 17}},
        {"star/k5", {22, 16}},
        {"star/m0", {20, 12}},
        {"star/m4", {18, 9}},
        {"star/m8", {15, 5}},
        {"star/wr", {25, 0}}
    };
}



// Some objects do not have sprites, because they are just an orbital
// center for two or more other objects.
const QString &StellarObject::Sprite() const
{
    return sprite;
}



// Get this object's position on the date most recently passed to this
// system's SetDate() function.
const QVector2D &StellarObject::Position() const
{
    return position;
}



// Get this object's distance from its parent.
double StellarObject::Distance() const
{
    return distance;
}



// Get the number of days it takes for this object to complete one orbit.
double StellarObject::Period() const
{
    return period;
}



// Get the radius of this planet, i.e. how close you must be to land.
double StellarObject::Radius() const
{
    auto it = INFO.find(sprite);
    return (it != INFO.end() ? it->second.radius : 0);
}



// If it is possible to land on this planet, this returns the Planet
// objects that gives more information about it. Otherwise, this
// function will just return nullptr.
const QString &StellarObject::GetPlanet() const
{
    return planet;
}



// Get the index of the parent object.
int StellarObject::Parent() const
{
    return parent;
}



// Get a random star, based on a probability distribution of stars.
StellarObject StellarObject::Star()
{
    int r = rand() % 100;
    auto it = INFO.lower_bound("star");
    while(it != INFO.end() && r >= it->second.info)
    {
        r -= it->second.info;
        ++it;
    }

    // This should never happen, because the weights add up to 100. But, just
    // for the sake of defensive programming:
    if(it == INFO.end())
        it = INFO.find("star/wr");

    StellarObject object;
    object.sprite = it->first;
    return object;
}



// Get a random "moon." It may also be used as a stand-alone planet.
StellarObject StellarObject::Moon()
{
    return Planet(0, MOON_RADIUS);
}



// Get a random (non-giant) planet. It may or may not be habitable.
StellarObject StellarObject::Planet()
{
    return Planet(MOON_RADIUS, GIANT_RADIUS);
}



// Get a random planet that can exist outside the habitable zone.
StellarObject StellarObject::Uninhabited()
{
    return Planet(MOON_RADIUS, GIANT_RADIUS, true);
}



// Get a random gas giant.
StellarObject StellarObject::Giant()
{
    return Planet(GIANT_RADIUS);
}



// Get a random station.
StellarObject StellarObject::Station()
{
    static int count = 0;
    if(!count)
    {
        // Skip stars and anything bigger than a radius of 50 pixels.
        for(const auto &it : INFO)
            if(it.second.info == 3 && it.first[0] == 'p')
                ++count;
    }

    StellarObject object;
    int r = rand() % count;
    for(const auto &it : INFO)
        if(it.second.info == 3 && it.first[0] == 'p')
        {
            if(!r)
            {
                object.sprite = it.first;
                break;
            }
            --r;
        }
    return object;
}



// Check if this is a star.
bool StellarObject::IsStar() const
{
    return sprite.startsWith("star");
}



bool StellarObject::IsMoon() const
{
    return (Radius() < MOON_RADIUS) && !IsStation() && !IsStar();
}



bool StellarObject::IsTerrestrial() const
{
    return !IsMoon() && !IsGiant() && !IsStation() && !IsStar();
}



bool StellarObject::IsGiant() const
{
    return (Radius() >= GIANT_RADIUS) && !IsStation() && !IsStar();
}



// Check if this is a station.
bool StellarObject::IsStation() const
{
    return sprite.startsWith("planet/station");
}



bool StellarObject::IsInhabited() const
{
    if(IsStar())
        return false;
    if(IsStation())
        return true;

    auto it = INFO.find(sprite);
    return(it != INFO.end() && it->second.info == 2);
}



void StellarObject::SetPlanet(const QString &name)
{
    planet = name;
}



StellarObject StellarObject::Planet(int minRadius, int maxRadius, bool skipHabitable)
{
    int count = 0;
    for(const auto &it : INFO)
        if(it.second.radius >= minRadius && it.second.radius < maxRadius)
            if(it.first[0] == 'p' && !(skipHabitable && it.second.info) && it.second.info != 3)
                ++count;

    StellarObject object;
    int r = rand() % count;
    for(const auto &it : INFO)
        if(it.second.radius >= minRadius && it.second.radius < maxRadius)
            if(it.first[0] == 'p' && !(skipHabitable && it.second.info) && it.second.info != 3)
            {
                if(!r)
                {
                    object.sprite = it.first;
                    break;
                }
                --r;
            }
    return object;
}
