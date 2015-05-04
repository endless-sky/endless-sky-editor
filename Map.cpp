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
#include "DataWriter.h"

#include <algorithm>

using namespace std;



void Map::Load(const QString &path)
{
    // Clear everything first.
    *this = Map();

    dataDirectory = path.left(path.lastIndexOf('/') + 1);
    DataFile data(path);
    comments = data.Comments();

    for(const DataNode &node : data)
    {
        if(node.Token(0) == "planet" && node.Size() >= 2)
            planets[node.Token(1)].Load(node);
        else if(node.Token(0) == "system" && node.Size() >= 2)
            systems[node.Token(1)].Load(node);
        else if(node.Token(0) == "galaxy")
        {
            galaxies.push_back(Galaxy());
            galaxies.back().Load(node);
        }
        else
            unparsed.push_back(node);
    }

    QString commodityPath = dataDirectory + "commodities.txt";
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



void Map::Save(const QString &path) const
{
    DataWriter file(path);
    file.WriteRaw(comments);
    file.Write();

    for(const Galaxy &it : galaxies)
    {
        it.Save(file);
        file.Write();
    }
    for(const auto &it : systems)
    {
        it.second.Save(file);
        file.Write();
    }
    for(const auto &it : planets)
    {
        it.second.Save(file);
        file.Write();
    }
    for(const auto &it : unparsed)
    {
        file.Write(it);
        file.Write();
    }
}



const QString &Map::DataDirectory() const
{
    return dataDirectory;
}



list<Galaxy> &Map::Galaxies()
{
    return galaxies;
}



const list<Galaxy> &Map::Galaxies() const
{
    return galaxies;
}



map<QString, System> &Map::Systems()
{
    return systems;
}



const map<QString, System> &Map::Systems() const
{
    return systems;
}



map<QString, Planet> &Map::Planets()
{
    return planets;
}



const map<QString, Planet> &Map::Planets() const
{
    return planets;
}



const vector<Map::Commodity> &Map::Commodities() const
{
    return commodities;
}



// Map a price to a value between 0 and 1 (lowest vs. highest).
double Map::MapPrice(QString &commodity, int price) const
{
    for(const Commodity &it : commodities)
        if(it.name == commodity)
            return max(0., min(1., static_cast<double>(price - it.low) / (it.high - it.low)));

    return .5;
}
