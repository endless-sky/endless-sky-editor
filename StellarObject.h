/* StellarObject.h
Copyright (c) 2014 by Michael Zahniser

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#ifndef STELLAR_OBJECT_H_
#define STELLAR_OBJECT_H_

#include "DataNode.h"

#include <QVector2D>
#include <QString>



// Class representing a planet, star, moon, or other large object in space. This
// does not store the details of what is on that object, if anything; that is
// handled by the Planet class. Each object's position depends on what it is
// orbiting around and how far away it is from that object. Each day, all the
// objects in each system move slightly in their orbits.
class StellarObject {
public:
    // Some objects do not have sprites, because they are just an orbital
    // center for two or more other objects.
    const QString &Sprite() const;
    // Get this object's position on the date most recently passed to this
    // system's SetDate() function.
    const QVector2D &Position() const;
    // Get this object's distance from its parent.
    double Distance() const;
    // Get the number of days it takes for this object to complete one orbit.
    double Period() const;
    // Get the radius of this planet, i.e. how close you must be to land.
    double Radius() const;
    // If it is possible to land on this planet, this returns the Planet
    // objects that gives more information about it. Otherwise, this
    // function will just return nullptr.
    const QString &GetPlanet() const;

    // Get the index of the parent object.
    int Parent() const;

    // Get a random star, based on a probability distribution of stars.
    static StellarObject Star();
    // Get a random "moon." It may also be used as a stand-alone planet.
    static StellarObject Moon();
    // Get a random (non-giant) planet. It may or may not be habitable.
    static StellarObject Planet();
    // Get a random planet that can exist outside the habitable zone.
    static StellarObject Uninhabited();
    // Get a random gas giant.
    static StellarObject Giant();
    // Get a random station.
    static StellarObject Station();

    // Check if this is a star.
    bool IsStar() const;
    // Check this object's size.
    bool IsMoon() const;
    bool IsGiant() const;
    // Check if this is a station.
    bool IsStation() const;

    // Editing a stellar object:
    void SetPlanet(const QString &name);


private:
    static StellarObject Planet(int minRadius, int maxRadius = 1000, bool skipHabitable = false);


private:
    QString sprite;

    QVector2D position;
    QString planet;

    double distance = 0.;
    double period = 0.;
    double offset = 0.;
    int parent = -1;

    std::list<DataNode> unparsed;

    // Let System handle setting all the values of an Object.
    friend class System;
};



#endif
