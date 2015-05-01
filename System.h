/* System.h
Copyright (c) 2014 by Michael Zahniser

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#ifndef SYSTEM_H_
#define SYSTEM_H_

#include "StellarObject.h"

#include <QVector2D>

#include <string>
#include <vector>

class DataNode;
class Planet;



// Class representing a star system. This includes characteristics like what
// ships enter that system, what asteroids are present, who owns the system, and
// what prices the trade goods have in that system. It also includes the stellar
// objects in each system, and the hyperspace links between systems.
class System {
public:
    struct Asteroid { std::string type; int count; double energy; };
    struct Fleet { std::string name; int period; };


public:
    // Load a system's description.
    void Load(const DataNode &node);

    // Get this system's name and position (in the star map).
    const std::string &Name() const;
    const QVector2D &Position() const;
    // Get this system's government.
    const std::string &Government() const;

    // Get a list of systems you can travel to through hyperspace from here.
    const std::vector<std::string> &Links() const;

    // Get the stellar object locations on the most recently set date.
    const std::vector<StellarObject> &Objects() const;
    // Get the habitable zone's center.
    double HabitableZone() const;

    // Get the specification of how many asteroids of each type there are.
    const std::vector<Asteroid> &Asteroids() const;

    // Get the price of the given commodity in this system.
    int Trade(const std::string &commodity) const;

    // Get the probabilities of various fleets entering this system.
    const std::vector<Fleet> &Fleets() const;

    // Position the planets, etc.
    void SetDay(double day);


private:
    void LoadObject(const DataNode &node, int parent = -1);


private:
    // Name and position (within the star map) of this system.
    std::string name;
    QVector2D position;
    std::string government;

    // Hyperspace links to other systems.
    std::vector<std::string> links;

    // Stellar objects, listed in such an order that an object's parents are
    // guaranteed to appear before it (so that if we traverse the vector in
    // order, updating positions, an object's parents will already be at the
    // proper position before that object is updated).
    std::vector<StellarObject> objects;
    double habitable;

    std::vector<Asteroid> asteroids;
    std::map<std::string, int> trade;
    std::vector<Fleet> fleets;

    std::list<DataNode> unparsed;
};



#endif
