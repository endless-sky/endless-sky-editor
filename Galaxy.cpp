/* Galaxy.cpp
Copyright (c) 2015 by Michael Zahniser

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#include "Galaxy.h"

#include "DataNode.h"
#include "DataWriter.h"

using namespace std;



void Galaxy::Load(const DataNode &node)
{
    if(node.Size() >= 2)
        name = node.Token(1);

    for(const DataNode &child : node)
    {
        if(child.Token(0) == "pos" && child.Size() >= 3)
            position = QVector2D(child.Value(1), child.Value(2));
        else if(child.Token(0) == "sprite" && child.Size() >= 2)
            sprite = child.Token(1);
        else
            unparsed.push_back(child);
    }
}



void Galaxy::Save(DataWriter &file) const
{
    if(name.empty())
        file.Write("galaxy");
    else
        file.Write("galaxy", name);
    file.BeginChild();
    {
        file.Write("pos", position.x(), position.y());
        file.Write("sprite", sprite);
        for(const DataNode &node : unparsed)
            file.Write(node);
    }
    file.EndChild();
}



const QVector2D &Galaxy::Position() const
{
    return position;
}



const string &Galaxy::Sprite() const
{
    return sprite;
}
