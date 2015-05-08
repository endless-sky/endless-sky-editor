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

#include "DetailView.h"
#include "SpriteSet.h"
#include "StellarObject.h"
#include "System.h"

#include <QPainter>
#include <QPainterPath>
#include <QPalette>
#include <QMouseEvent>
#include <QTabWidget>

#include <algorithm>
#include <cmath>

using namespace std;



SystemView::SystemView(DetailView *detailView, QTabWidget *tabs, QWidget *parent) :
    QWidget(parent), detailView(detailView), tabs(tabs)
{
    setAutoFillBackground(true);
    QPalette p = palette();
    p.setColor(backgroundRole(), QColor(0, 0, 0));
    setPalette(p);

    connect(&timer, SIGNAL(timeout()), this, SLOT(step()));
    timer.start(1000. / 60.);
}



void SystemView::Select(System *system)
{
    if(detailView)
        detailView->SetSystem(system);

    this->system = system;
    asteroids.Set(system);
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
    if(isPaused || (tabs && tabs->currentWidget() != this))
        return;

    timeStep += .1;
    asteroids.Step();
    if(system)
        system->SetDay(timeStep);
    update();
}



void SystemView::Pause()
{
    isPaused = !isPaused;
}



void SystemView::mousePressEvent(QMouseEvent *event)
{
    clickOff = QVector2D(event->pos()) - offset;
}



void SystemView::mouseMoveEvent(QMouseEvent *event)
{
    offset = QVector2D(event->pos()) - clickOff;
    if(isPaused)
        update();
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

    if(isPaused)
        update();
}



void SystemView::paintEvent(QPaintEvent */*event*/)
{
    if(!system)
        return;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    painter.translate(.5 * width(), .5 * height());
    painter.translate(offset.x(), offset.y());
    painter.scale(scale, scale);

    // Draw faint circles showing the region occupied by each planet.
    painter.setPen(Qt::NoPen);
    for(const StellarObject &object : system->Objects())
    {
        double radius = system->OccupiedRadius(object);
        if(!radius)
            continue;

        QBrush brush(QColor(20, 20, 20));
        painter.setBrush(brush);

        QPainterPath outside;
        outside.addEllipse(QPointF(), object.Distance() + radius, object.Distance() + radius);
        QPainterPath inside;
        outside.addEllipse(QPointF(), object.Distance() - radius, object.Distance() - radius);
        QPainterPath ring = outside.subtracted(inside);

        painter.drawPath(ring);
    }

    // Draw circles indicating the center and edges of the habitable zone.
    {
        painter.setBrush(Qt::NoBrush);
        double radius = system->HabitableZone();

        QPen innerPen(QColor(255., 200., 0.));
        painter.setPen(innerPen);
        painter.drawEllipse(QPointF(), .5 * radius, .5 * radius);

        QPen centerPen(QColor(0., 255., 0.));
        painter.setPen(centerPen);
        painter.drawEllipse(QPointF(), radius, radius);

        QPen outerPen(QColor(0., 150., 255.));
        painter.setPen(outerPen);
        painter.drawEllipse(QPointF(), 2. * radius, 2. * radius);
    }

    // Draw lines linking objects to their parents.
    QBrush brush(QColor(128, 128, 128));
    painter.setBrush(brush);
    QPen pen(QColor(255, 255, 255));
    painter.setPen(pen);
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
            double angle = pos.isNull() ? 0. : atan2(pos.x(), pos.y());
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
    asteroids.Draw(painter);
}
