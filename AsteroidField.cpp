/* AsteroidField.cpp
Copyright (c) 2015 by Michael Zahniser

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#include "AsteroidField.h"

#include "SpriteSet.h"
#include "System.h"

#include <QPainter>
#include <QString>

#include <cmath>

using namespace std;



void AsteroidField::Set(const System *system)
{
    asteroids.clear();
    if(!system)
        return;

    for(const System::Asteroid &it : system->Asteroids())
    {
        QPixmap sprite = SpriteSet::Get("asteroid/" + it.type + "/spin-00");
        for(int i = 0; i < it.count; ++i)
        {
            double angle = (rand() % 6283) * .001;
            double velocity = (rand() % 1000) * (.001 * it.energy);
            asteroids.push_back({
                QVector2D(rand() % 4096 - 2048, rand() % 4096 - 2048),
                QVector2D(velocity * sin(angle), velocity * cos(angle)),
                sprite});
        }
    }
}



void AsteroidField::Step()
{
    for(Asteroid &asteroid : asteroids)
    {
        asteroid.position += asteroid.velocity;
        if(asteroid.position.x() < -2048.)
            asteroid.position += QVector2D(4096., 0.);
        if(asteroid.position.x() >= 2048.)
            asteroid.position -= QVector2D(4096., 0.);
        if(asteroid.position.y() < -2048.)
            asteroid.position += QVector2D(0., 4096.);
        if(asteroid.position.y() >= 2048.)
            asteroid.position -= QVector2D(0., 4096.);
    }
}



void AsteroidField::Draw(QPainter &painter, const QRectF &bounds) const
{
    int firstX = round(bounds.left() / 4096.);
    int lastX = round(bounds.right() / 4096.);
    int firstY = round(bounds.top() / 4096.);
    int lastY = round(bounds.bottom() / 4096.);
    for(int y = firstY; y <= lastY; ++y)
        for(int x = firstX; x <= lastX; ++x)
        {
            QVector2D offset(x * 4096., y * 4096.);
            for(const Asteroid &asteroid : asteroids)
            {
                QPointF thisOffset = (asteroid.position + offset).toPointF();
                painter.translate(thisOffset);
                painter.scale(.5, .5);
                painter.drawPixmap(QPointF(), asteroid.sprite);
                painter.scale(2., 2.);
                painter.translate(-thisOffset);
            }
        }
}
