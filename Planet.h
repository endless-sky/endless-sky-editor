/* Planet.h
Copyright (c) 2014 by Michael Zahniser

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#ifndef PLANET_H_
#define PLANET_H_

#include <QString>

#include <list>
#include <vector>

class DataNode;
class DataWriter;
class System;



// Class representing a stellar object you can land on. (This includes planets,
// moons, and space stations.) Each planet has a certain set of services that
// are available, as well as attributes that determine what sort of missions
// might choose it as a source or destination.
class Planet {
public:
    // Load a planet's description from a file.
    void Load(const DataNode &node);
    void Save(DataWriter &file) const;

    // Get the name of the planet.
    const QString &Name() const;
    // Get the planet's descriptive text.
    const QString &Description() const;
    // Get the landscape sprite.
    const QString &Landscape() const;

    // Get the list of "attributes" of the planet.
    const std::vector<QString> &Attributes() const;

    // Check whether there is a spaceport (which implies there is also trading,
    // jobs, banking, and hiring).
    bool HasSpaceport() const;
    // Get the spaceport's descriptive text.
    const QString &SpaceportDescription() const;

    // Check if this planet has a shipyard.
    bool HasShipyard() const;
    // Get the list of ships in the shipyard.
    const std::vector<QString> &Shipyard() const;
    // Check if this planet has an outfitter.
    bool HasOutfitter() const;
    // Get the list of outfits available from the outfitter.
    const std::vector<QString> &Outfitter() const;

    // You need this good a reputation with this system's government to land here.
    double RequiredReputation() const;
    // This is what fraction of your fleet's value you must pay as a bribe in
    // order to land on this planet. (If zero, you cannot bribe it.)
    double Bribe() const;
    // This is how likely the planet's authorities are to notice if you are
    // doing something illegal.
    double Security() const;

    // Editing a planet:
    void SetLandscape(const QString &sprite);


private:
    QString name;
    QString description;
    QString spaceport;
    QString landscape;

    std::vector<QString> attributes;

    std::vector<QString> shipyard;
    std::vector<QString> outfitter;

    double requiredReputation;
    double bribe;
    double security;

    std::list<DataNode> unparsed;
};



#endif
