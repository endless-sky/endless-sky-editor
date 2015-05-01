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
        else if(node.Token(0) == "galaxy" && node.Size() >= 2)
            galaxies[node.Token(1)].Load(node);
        else
            unparsed.push_back(node);
    }

    string commodityPath = path.substr(0, path.rfind('/') + 1) + "commodities.txt";
    DataFile tradeData(commodityPath);

    for(const DataNode &node : tradeData)
        if(node.Token(0) == "trade")
            for(const DataNode &child : node)
                if(child.Token(0) == "commodity" && child.Size() >= 4)
                {
                    int low = static_cast<int>(child.Value(2));
                    int high = static_cast<int>(child.Value(3));
                    commodities.push_back({child.Token(1), low, high});
                }
}



map<string, Galaxy> &Map::Galaxies()
{
    return galaxies;
}



const map<string, Galaxy> &Map::Galaxies() const
{
    return galaxies;
}



map<string, System> &Map::Systems()
{
    return systems;
}



const map<string, System> &Map::Systems() const
{
    return systems;
}



map<string, Planet> &Map::Planets()
{
    return planets;
}



const map<string, Planet> &Map::Planets() const
{
    return planets;
}



const std::vector<Map::Commodity> &Map::Commodities() const
{
    return commodities;
}
