/* LandscapeView.cpp
Copyright (c) 2015 by Michael Zahniser

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#include "LandscapeView.h"

#include "Planet.h"
#include "SpriteSet.h"

#include <QPainter>



LandscapeView::LandscapeView(QWidget *parent) :
    QWidget(parent)
{
}



void LandscapeView::SetPlanet(Planet *planet)
{
    this->planet = planet;
}



void LandscapeView::paintEvent(QPaintEvent */*event*/)
{
    if(!planet || planet->Landscape().isEmpty())
        return;

    QPixmap image = SpriteSet::Get(planet->Landscape());
    if(image.isNull())
        return;

    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    int x = (width() - image.width()) / 2;
    int y = (height() - image.height()) / 2;
    painter.drawPixmap(x, y, image);
}
