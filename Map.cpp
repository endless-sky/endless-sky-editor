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
#include "SpriteSet.h"

#include <algorithm>

using namespace std;



void Map::Load(const QString &path)
{
    // Clear everything first.
    *this = Map();

    dataDirectory = path.left(path.lastIndexOf('/'));
    fileName = path.right(path.lastIndexOf('/'));
    QString rootDir = dataDirectory.left(dataDirectory.lastIndexOf('/'));
    dataDirectory += "/";
    SpriteSet::SetRootPath(rootDir + "/images/");

    DataFile data(path);
    comments = data.Comments();

    for(const DataNode &node : data)
    {
        if(node.Token(0) == "planet" && node.Size() >= 2)
            planets[node.Token(1)].Load(node);
        else if(node.Token(0) == "system" && node.Size() >= 2)
            systems[node.Token(1)].Load(node);
        else if(node.Token(0) == "galaxy")
            galaxies.emplace_back(node);
        else
            unparsed.push_back(node);
    }

    QString commodityPath = dataDirectory + "commodities.txt";
    DataFile tradeData(commodityPath);

    // Load in "standard" commodities - those that supply a category, low, and high price.
    // "Special" commodities that are only used as names for mission cargo are not loaded.
    for(const DataNode &node : tradeData)
        if(node.Token(0) == "trade")
            for(const DataNode &child : node)
                if(child.Token(0) == "commodity" && child.Size() >= 4)
                    commodities.emplace_back(child.Token(1), child.Value(2), child.Value(3));

    isChanged = false;
}



void Map::Save(const QString &path)
{
    fileName = path.right(path.lastIndexOf('/'));
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
    isChanged = false;
}



const QString &Map::DataDirectory() const
{
    return dataDirectory;
}



const QString &Map::FileName() const
{
    return fileName;
}



void Map::SetChanged(bool changed)
{
    isChanged = changed;
}



bool Map::IsChanged() const
{
    return isChanged;
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
double Map::MapPrice(const QString &commodity, int price) const
{
    for(const Commodity &it : commodities)
        if(it.name == commodity)
            return max(0., min(1., static_cast<double>(price - it.low) / (it.high - it.low)));

    return .5;
}



QString Map::PriceLevel(const QString &commodity, int price) const
{
    static const QString LEVEL[] = {
                "(very low)",
                "(low)",
                "(medium)",
                "(high)",
                "(very high)"
            };

    for(const Commodity &it : commodities)
        if(it.name == commodity)
        {
            int level = max(0, min(4, ((price - it.low) * 5) / (it.high - it.low)));
            return LEVEL[level];
        }

    return "";
}



// Rename a system. This requires updating all the systems that link to it.
void Map::RenameSystem(const QString &from, const QString &to)
{
    auto it = systems.find(from);
    if(it == systems.end() || systems.find(to) != systems.end())
        return;

    System &renamed = systems[to] = it->second;
    renamed.SetName(to);
    for(const QString &link : it->second.Links())
    {
        auto oit = systems.find(link);
        if(oit != systems.end())
            oit->second.ChangeLink(from, to);
    }
    systems.erase(it);
}



// Rename a planet. The editor does not support planets sharing a name with
// a system, or renaming an object to share a planet definition (i.e. wormholes).
void Map::RenamePlanet(StellarObject *object, const QString &name)
{
    if(!object || systems.find(name) != systems.end())
        return;

    auto it = planets.find(object->GetPlanet());
    if(it != planets.end())
    {
        // Copy the existing definition to the new name.
        planets[name] = it->second;
        // Erase the previous definition.
        planets.erase(it);
    }
    planets[name].SetName(name);
    object->SetPlanet(name);
}
