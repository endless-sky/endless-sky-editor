/* Map.h
Copyright (c) 2015 by Michael Zahniser

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#ifndef MAP_H
#define MAP_H

#include "Galaxy.h"
#include "Planet.h"
#include "System.h"

#include <list>
#include <map>
#include <string>

class DataNode;



class Map {
public:
    void Load(const QString &path);
    void Save(const QString &path) const;
    const QString &DataDirectory() const;

    std::list<Galaxy> &Galaxies();
    const std::list<Galaxy> &Galaxies() const;

    std::map<QString, System> &Systems();
    const std::map<QString, System> &Systems() const;

    std::map<QString, Planet> &Planets();
    const std::map<QString, Planet> &Planets() const;

    // Access the commodity data:
    struct Commodity { QString name; int low; int high; };
    const std::vector<Commodity> &Commodities() const;
    // Map a price to a value between 0 and 1 (lowest vs. highest).
    double MapPrice(QString &commodity, int price) const;


private:
    QString dataDirectory;

    std::list<Galaxy> galaxies;
    std::map<QString, System> systems;
    std::map<QString, Planet> planets;
    std::vector<Commodity> commodities;

    QString comments;
    std::list<DataNode> unparsed;
};

#endif // MAP_H
