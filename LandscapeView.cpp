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

#include "LandscapeLoader.h"
#include "Planet.h"
#include "SpriteSet.h"

#include <QImage>
#include <QMouseEvent>
#include <QMutex>
#include <QPainter>
#include <QThread>

#include <algorithm>
#include <list>

using namespace std;

namespace {
    LandscapeLoader loader;

    static const int THUMB_SCALE = 18;
    static const int THUMB_PAD = 2;
    static const int thumbWidth = (720 / THUMB_SCALE) + 2 * THUMB_PAD;
    static const int thumbHeight = (360 / THUMB_SCALE) + 2 * THUMB_PAD;
}



LandscapeView::LandscapeView(QWidget *parent) :
    QWidget(parent)
{
    loader.Init();
}



LandscapeView::~LandscapeView()
{
    loader.Quit();
}



void LandscapeView::SetPlanet(Planet *planet)
{
    this->planet = planet;
    showGallery = planet && planet->Landscape().isEmpty();
}



void LandscapeView::mousePressEvent(QMouseEvent *event)
{
    if(!planet)
        return;

    showGallery = !showGallery;
    if(!showGallery)
    {
        int cols = width() / thumbWidth;
        int rows = height() / thumbHeight;
        int left = (width() - cols * thumbWidth) / 2;
        int top = (height() - rows * thumbHeight) / 2;
        int x = event->pos().x() - left;
        int y = event->pos().y() - top;
        if(x >= 0 && x < cols * thumbWidth && y >= 0 && y < rows * thumbHeight)
        {
            unsigned index = x / thumbWidth + cols * (y / thumbHeight);
            if(index < loader.Available().size())
                planet->SetLandscape(loader.Available()[index]);
        }
    }
    update();
}



void LandscapeView::paintEvent(QPaintEvent */*event*/)
{
    loader.Update();
    if(!planet)
        return;

    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    if(!showGallery)
    {
        if(planet->Landscape().isEmpty())
            return;

        QPixmap image = SpriteSet::Get(planet->Landscape());
        if(image.isNull())
            return;

        int x = (width() - image.width()) / 2;
        int y = (height() - image.height()) / 2;
        painter.drawPixmap(x, y, image);
        return;
    }

    int cols = width() / thumbWidth;
    int rows = height() / thumbHeight;
    int left = (width() - cols * thumbWidth) / 2;
    int top = (height() - rows * thumbHeight) / 2;

    auto it = loader.Available().begin();
    auto end = loader.Available().end();
    for(int row = 0; row < rows && it != end; ++row)
        for(int col = 0; col < cols && it != end; ++col, ++it)
        {
            QPixmap image = SpriteSet::Get(*it);
            int x = left + col * thumbWidth + THUMB_PAD;
            int y = top + row * thumbHeight + THUMB_PAD;
            painter.translate(x, y);
            painter.scale(1. / THUMB_SCALE, 1. / THUMB_SCALE);
            painter.drawPixmap(QPoint(), image);
            painter.scale(THUMB_SCALE, THUMB_SCALE);
            painter.translate(-x, -y);
        }
}
