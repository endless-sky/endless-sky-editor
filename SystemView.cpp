/* SystemView.cpp
Copyright (c) 2015 by Michael Zahniser

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#include "SystemView.h"

#include "SpriteSet.h"
#include "StellarObject.h"
#include "System.h"

#include <QPainter>
#include <QPalette>
#include <QMouseEvent>
#include <QTabWidget>

#include <algorithm>
#include <cmath>

using namespace std;



SystemView::SystemView(QTabWidget *tabs, QWidget *parent) :
    QWidget(parent), tabs(tabs)
{
    setAutoFillBackground(true);
    QPalette p = palette();
    p.setColor(backgroundRole(), QColor(0, 0, 0));
    setPalette(p);

    connect(&timer, SIGNAL(timeout()), this, SLOT(step()));
    timer.start(1. / 60.);
}



void SystemView::Select(System *system)
{
    this->system = system;
    if(system)
        system->SetDay(timeStep);
}



System *SystemView::Selected() const
{
    return system;
}



QSize SystemView::minimumSizeHint() const
{
    return QSize(400, 300);
}



QSize SystemView::sizeHint() const
{
    return QSize(1200, 900);
}



void SystemView::step()
{
    if(tabs && tabs->currentWidget() != this)
        return;

    timeStep += .01;
    if(system)
        system->SetDay(timeStep);
    update();
}



void SystemView::mousePressEvent(QMouseEvent *event)
{
    clickOff = QVector2D(event->pos()) - offset;
}



void SystemView::mouseMoveEvent(QMouseEvent *event)
{
    offset = QVector2D(event->pos()) - clickOff;
}



void SystemView::wheelEvent(QWheelEvent *event)
{
    // Figure out where in the 100% scale image the click occurred.
    QVector2D point(event->pos());
    QVector2D center(.5 * width(), .5 * height());
    // point = origin * scale + offset + center.
    QVector2D origin = (point - offset - center) / scale;

    scale = max(.0625, min(1., scale * exp(event->delta() * .001)));

    // We want: point = origin * scale + offset + center.
    offset = point - origin * scale - center;
}



void SystemView::paintEvent(QPaintEvent */*event*/)
{
    if(!system)
        return;

    QPainter painter(this);
    QBrush brush(QColor(128, 128, 128));
    painter.setBrush(brush);
    QPen pen(QColor(255, 255, 255));
    painter.setPen(pen);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    painter.translate(.5 * width(), .5 * height());
    painter.translate(offset.x(), offset.y());
    painter.scale(scale, scale);

    // Draw lines linking objects to their parents.
    for(const StellarObject &object : system->Objects())
    {
        QPointF parent;
        if(object.Parent() >= 0)
            parent = system->Objects()[object.Parent()].Position().toPointF();
        painter.drawLine(object.Position().toPointF(), parent);
    }

    for(const StellarObject &object : system->Objects())
    {
        QPixmap sprite = SpriteSet::Get(object.Sprite());
        if(sprite.width() && sprite.height())
        {
            QVector2D pos = object.Position();
            double angle = atan2(pos.x(), pos.y());
            angle *= 180. / M_PI;
            angle += 180.;

            painter.translate(pos.toPointF());
            painter.rotate(-angle);
            painter.drawPixmap(QPointF(-.5 * sprite.width(), -.5 * sprite.height()), sprite);
            painter.rotate(angle);
            painter.translate(-pos.toPointF());
        }
        else
        {
            double radius = object.Radius();
            painter.drawEllipse(object.Position().toPointF(), radius, radius);
        }
    }
}
