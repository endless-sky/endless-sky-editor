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

#include <limits>
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
    void LoadTribute(const DataNode &node);
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
    // Stipend for conquering the planet
    double Tribute() const;
    // Combat Rating level before you are an actual threat. Balance this with tribute and fleet.
    double TributeThreshold() const;
    // Number of Defending Ships
    double TributeFleetQuantity() const;
    // Get the fleet assigned to protect planet
    const QString &TributeFleetName() const;

    // Editing a planet:
    void SetName(const QString &name);
    void SetLandscape(const QString &sprite);
    void SetDescription(const QString &text);
    void SetSpaceportDescription(const QString &text);
    std::vector<QString> &Attributes();
    std::vector<QString> &Shipyard();
    std::vector<QString> &Outfitter();
    void SetRequiredReputation(double value);
    void SetBribe(double value);
    void SetSecurity(double value);
    void SetTribute(double value);
    void SetTributeThreshold(double value);
    void SetTributeFleetName(QString &value);
    void SetTributeFleetQuantity(double value);

private:
    QString name;
    QString landscape;
    QString description;
    QString spaceport;
    QString government;
    QString music;
    QString tributeFleetName;

    // Use a vector so the printing order is preserved.
    std::vector<QString> attributes;
    std::vector<QString> shipyard;
    std::vector<QString> outfitter;

    double requiredReputation = std::numeric_limits<double>::quiet_NaN();
    double bribe = std::numeric_limits<double>::quiet_NaN();
    double security = std::numeric_limits<double>::quiet_NaN();

    double tribute = std::numeric_limits<double>::quiet_NaN();
    double tributeThreshold = std::numeric_limits<double>::quiet_NaN();
    double tributeFleetQuantity = std::numeric_limits<double>::quiet_NaN();
    std::list<DataNode> unparsed;
    std::list<DataNode> tributeUnparsed;
};



#endif
