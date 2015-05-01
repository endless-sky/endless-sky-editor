/* Map.cpp
Copyright (c) 2015 by Michael Zahniser

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#include "Map.h"

#include "DataFile.h"

using namespace std;



void Map::Load(const string &path)
{
    DataFile data(path);

    for(const DataNode &node : data)
    {
        if(node.Token(0) == "planet" && node.Size() >= 2)
            planets[node.Token(1)].Load(node);
        else if(node.Token(0) == "system" && node.Size() >= 2)
            systems[node.Token(1)].Load(node);
    }
}



map<string, Planet> &Map::Planets()
{
    return planets;
}



const map<string, Planet> &Map::Planets() const
{
    return planets;
}



map<string, System> &Map::Systems()
{
    return systems;
}



const map<string, System> &Map::Systems() const
{
    return systems;
}
