/* SpriteSet.cpp
Copyright (c) 2015 by Michael Zahniser

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#include "SpriteSet.h"

#include <QFileInfo>

#include <map>

using namespace std;

namespace {
    QString root;
    map<QString, QPixmap> sprite;
}



void SpriteSet::SetRootPath(const QString &path)
{
    root = path;
    if(!root.isEmpty() && !root.endsWith('/'))
        root += '/';
}



QPixmap SpriteSet::Get(const QString &name)
{
    auto it = sprite.find(name);
    if(it != sprite.end())
        return it->second;

    QPixmap image;

    QString baseName = root + name;
    QFileInfo jpg(baseName + ".jpg");
    if(jpg.exists())
        image.load(jpg.filePath());
    else
    {
        QFileInfo png(baseName + ".png");
        if(png.exists())
            image.load(png.filePath());
    }
    sprite[name] = image;
    return image;
}
