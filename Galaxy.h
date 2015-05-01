/* Galaxy.h
Copyright (c) 2015 by Michael Zahniser

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#ifndef GALAXY_H
#define GALAXY_H

#include "DataNode.h"

#include <QVector2D>

#include <list>
#include <string>



class Galaxy {
public:
    void Load(const DataNode &node);

    const QVector2D &Position() const;
    const std::string &Sprite() const;


private:
    QVector2D position;
    std::string sprite;

    std::list<DataNode> unparsed;
};



#endif // GALAXY_H
