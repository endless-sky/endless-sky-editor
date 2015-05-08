/* LandscapeLoader.cpp
Copyright (c) 2015 by Michael Zahniser

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#include "LandscapeLoader.h"

#include "SpriteSet.h"

#include <QDir>
#include <QDirIterator>

using namespace std;



LandscapeLoader::LandscapeLoader(QObject *parent) :
    QThread(parent)
{
}



void LandscapeLoader::Init()
{
    // If this is not the first object to initialize this thread, there is
    // no need to do any work. Note that new objects will always be created
    // in the GUI thread, so no synchronization is needed here.
    if(initCount++)
        return;

    // The thread is not started yet, so no mutex is needed.
    QDir dir(SpriteSet::RootPath() + "land/");
    QDirIterator it(dir);
    while(it.hasNext())
    {
        it.next();
        if(!it.fileName().endsWith(".jpg"))
            continue;

        QString name = "land/" + it.fileName();
        name.chop(4);
        toLoad.push_back(name);
    }
    toLoad.sort();

    start();
}



void LandscapeLoader::Update()
{
    QMutexLocker lock(&mutex);
    for(const auto &it : loaded)
    {
        available.push_back(it.first);
        SpriteSet::Set(it.first, it.second);
    }
    loaded.clear();
}



void LandscapeLoader::Quit()
{
    // If other instances are still active, do nothing.
    if(--initCount)
        return;

    // Signal the worker that it should quit, even if it had work left to do.
    {
        QMutexLocker lock(&mutex);
        toLoad.clear();
    }
    wait();
}



const vector<QString> &LandscapeLoader::Available() const
{
    return available;
}



void LandscapeLoader::run()
{
    while(true)
    {
        QString nextPath;
        {
            QMutexLocker lock(&mutex);
            if(toLoad.empty())
                return;
            nextPath = toLoad.front();
            toLoad.pop_front();
        }

        QImage image(SpriteSet::RootPath() + nextPath + ".jpg");

        {
            QMutexLocker lock(&mutex);
            loaded[nextPath] = image;
        }
    }
}
