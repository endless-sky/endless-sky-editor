/* main.cpp
Copyright (c) 2015 by Michael Zahniser

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#include "MainWindow.h"
#include "Map.h"
#include "SpriteSet.h"

#include <QApplication>
#include <QString>

using namespace std;



int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Map mapData;

    QString path = argc > 1 ? argv[1] : "/usr/share/games/endless-sky/data/map.txt";
    QString dataDir = path.left(path.lastIndexOf('/'));
    QString rootDir = dataDir.left(dataDir.lastIndexOf('/'));
    SpriteSet::SetRootPath(rootDir + "/images/");

    mapData.Load(path);

    MainWindow w(mapData);
    w.resize(1200, 900);
    w.show();

    return a.exec();
}
