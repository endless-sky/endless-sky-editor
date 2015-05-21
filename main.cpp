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
#include <QFileInfo>
#include <QFileOpenEvent>
#include <QString>

#include <iostream>

using namespace std;

class EventFilter: public QObject {
public:
    EventFilter(MainWindow &window) : QObject(), window(window) {};
    ~EventFilter() {};

    bool eventFilter(QObject *obj, QEvent *event)
    {
        if(event->type() == QEvent::FileOpen)
        {
            window.DoOpen(dynamic_cast<QFileOpenEvent *>(event)->file());
            return false;
        }
        return obj->event(event);
    }

    MainWindow &window;
};



void PrintHelp();
void PrintVersion();



int main(int argc, char *argv[])
{
    QString path;
    for(int i = 1; i < argc; ++i)
    {
        QString arg = argv[i];
        if(arg == "-v" || arg == "--version")
        {
            PrintVersion();
            return 0;
        }
        else if(arg[0] != '-')
            path = arg;
        else
        {
            PrintHelp();
            return 0;
        }
    }
    if(path.isEmpty())
    {
#if defined __APPLE__
        path = "/Applications/Endless Sky.app/Contents/Resources/data/map.txt";
#elif defined _WIN32
        path = "C:/Program Files/Endless Sky/data/map.txt";
#else
        path = "/usr/share/games/endless-sky/data/map.txt";
#endif
        if(!path.isEmpty() && !QFileInfo(path).exists())
            path.clear();
    }

    QApplication app(argc, argv);
    Map mapData;
    if(!path.isEmpty())
        mapData.Load(path);

    MainWindow window(mapData);
    app.installEventFilter(new EventFilter(window));

    return app.exec();
}



void PrintHelp()
{
    cerr << endl;
    cerr << "Command line options:" << endl;
    cerr << "    -h, --help: print this help message." << endl;
    cerr << "    -v, --version: print version information." << endl;
    cerr << "    <path to map.txt>: load the given map file." << endl;
    cerr << "        Sprites are then loaded from ../images/ relative to the map file." << endl;
    cerr << endl;
    cerr << "Report bugs to: mzahniser@gmail.com" << endl;
    cerr << "Home page: <https://endless-sky.github.io>" << endl;
    cerr << endl;
}



void PrintVersion()
{
    cerr << endl;
    cerr << "Endless Sky Editor v. 0.7.10" << endl;
    cerr << "License GPLv3+: GNU GPL version 3 or later: <https://gnu.org/licenses/gpl.html>" << endl;
    cerr << "This is free software: you are free to change and redistribute it." << endl;
    cerr << "There is NO WARRANTY, to the extent permitted by law." << endl;
    cerr << endl;
}
