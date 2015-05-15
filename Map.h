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
class StellarObject;



class Map {
public:
    void Load(const QString &path);
    void Save(const QString &path) const;
    const QString &DataDirectory() const;

    // Mark this file as changed.
    void SetChanged(bool changed = true);
    bool IsChanged() const;

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
    double MapPrice(const QString &commodity, int price) const;
    QString PriceLevel(const QString &commodity, int price) const;

    // Rename a system. This involves changing all the systems that link to it
    // and moving it to a new place in the map.
    void RenameSystem(const QString &from, const QString &to);
    void RenamePlanet(StellarObject *object, const QString &name);


private:
    QString dataDirectory;

    std::list<Galaxy> galaxies;
    std::map<QString, System> systems;
    std::map<QString, Planet> planets;
    std::vector<Commodity> commodities;

    QString comments;
    std::list<DataNode> unparsed;

    mutable bool isChanged = false;
};

#endif // MAP_H
