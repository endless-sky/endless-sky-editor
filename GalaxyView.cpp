/* GalaxyView.cpp
Copyright (c) 2015 by Michael Zahniser

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#include "GalaxyView.h"

#include "Map.h"
#include "SpriteSet.h"
#include "SystemView.h"

#include <QPainter>
#include <QPalette>
#include <QMouseEvent>

using namespace std;



GalaxyView::GalaxyView(Map &mapData, SystemView *systemView, QWidget *parent) :
    QWidget(parent), mapData(mapData), systemView(systemView)
{
    setAutoFillBackground(true);
    QPalette p = palette();
    p.setColor(backgroundRole(), QColor(0, 0, 0));
    setPalette(p);
}



void GalaxyView::mousePressEvent(QMouseEvent *event)
{
    clickOff = QVector2D(event->pos()) - offset;

    QVector2D origin = MapPoint(event->pos());
    for(auto &it : mapData.Systems())
        if(origin.distanceToPoint(it.second.Position()) < 5.)
        {
            systemView->Select(&it.second);
            update();
            return;
        }
}



void GalaxyView::mouseMoveEvent(QMouseEvent *event)
{
    offset = QVector2D(event->pos()) - clickOff;
    update();
}



void GalaxyView::wheelEvent(QWheelEvent *event)
{
    QVector2D point(event->pos());
    QVector2D center(.5 * width(), .5 * height());
    // point = origin * scale + offset + center.
    QVector2D origin = (point - offset - center) / scale;

    scale = max(.0625, min(1., scale * exp(event->delta() * .001)));

    // We want: point = origin * scale + offset + center.
    offset = point - origin * scale - center;
    update();
}



void GalaxyView::paintEvent(QPaintEvent */*event*/)
{
    QPen mediumPen(QColor(80, 80, 80));
    QPen brightPen(QColor(160, 160, 160));

    QBrush mediumBrush(QColor(160, 160, 160));
    QBrush brightBrush(QColor(240, 240, 240));

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    painter.translate(.5 * width(), .5 * height());
    painter.translate(offset.x(), offset.y());
    painter.scale(scale, scale);

    // Draw the "galaxy" images.
    for(const auto &it : mapData.Galaxies())
    {
        QPixmap sprite = SpriteSet::Get(it.second.Sprite());
        QPointF pos = (it.second.Position() - QVector2D(.5 * sprite.width(), .5 * sprite.height())).toPointF();
        painter.drawPixmap(pos, sprite);
    }

    painter.setPen(mediumPen);
    painter.setBrush(Qt::NoBrush);
    for(const auto &it : mapData.Systems())
    {
        QPointF pos = it.second.Position().toPointF();
        for(const string &link : it.second.Links())
        {
            auto lit = mapData.Systems().find(link);
            if(lit == mapData.Systems().end())
                continue;

            painter.drawLine(pos, lit->second.Position().toPointF());
        }
    }

    for(const auto &it : mapData.Systems())
    {
        QPointF pos = it.second.Position().toPointF();
        painter.setPen(Qt::NoPen);
        painter.setBrush(&it.second == systemView->Selected() ? brightBrush : mediumBrush);
        painter.drawEllipse(pos, 5, 5);

        painter.setPen(brightPen);
        painter.drawText(pos + QPointF(5, 5), QString::fromStdString(it.first));
    }

    painter.setPen(brightPen);
    painter.setBrush(Qt::NoBrush);
    if(systemView->Selected())
    {
        QPointF pos = systemView->Selected()->Position().toPointF();
        painter.drawEllipse(pos, 10, 10);
    }
}



// Figure out where in the 100% scale image the click occurred.
QVector2D GalaxyView::MapPoint(QPoint pos) const
{
    QVector2D point(pos);
    QVector2D center(.5 * width(), .5 * height());
    // point = origin * scale + offset + center.
    return (point - offset - center) / scale;
}
