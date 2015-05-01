/* SpriteSet.h
Copyright (c) 2015 by Michael Zahniser

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#ifndef SPRITESET_H
#define SPRITESET_H

#include <QPixmap>

#include <string>



class SpriteSet {
public:
    static void SetRootPath(const std::string &path);
    static QPixmap Get(const std::string &name);
};



#endif // SPRITESET_H
