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
#include "Map.h"
#include "pi.h"
#include "PlanetView.h"
#include "SpriteSet.h"
#include "StellarObject.h"
#include "System.h"

#include <QPainter>
#include <QPainterPath>
#include <QPalette>
#include <QMouseEvent>
#include <QTabWidget>
#include <QVector2D>

#include <algorithm>
#include <cmath>

using namespace std;



SystemView::SystemView(Map &mapData, DetailView *detailView, QTabWidget *tabs, QWidget *parent) :
    QWidget(parent), mapData(mapData), detailView(detailView), tabs(tabs)
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

    selectedObject = nullptr;
}



System *SystemView::Selected() const
{
    return system;
}



void SystemView::SetPlanetView(PlanetView *view)
{
    planetView = view;
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



void SystemView::RandomizeInhabited()
{
    if(system)
    {
        selectedObject = nullptr;
        system->Randomize(true, true);
        DidChange();
    }
}



void SystemView::Randomize()
{
    if(system)
    {
        selectedObject = nullptr;
        system->Randomize(true, false);
        DidChange();
    }
}



void SystemView::RandomizeUninhabited()
{
    if(system)
    {
        selectedObject = nullptr;
        system->Randomize(false, false);
        DidChange();
    }
}



void SystemView::ChangeAsteroids()
{
    if(system)
    {
        system->ChangeAsteroids();
        asteroids.Set(system);
        DidChange();
    }
}



void SystemView::ChangeMinables()
{
    if(system)
    {
        system->ChangeMinables();
        asteroids.Set(system);
        detailView->UpdateMinables();
        DidChange();
    }
}



void SystemView::ChangeStar()
{
    if(system)
    {
        system->ChangeStar();
        selectedObject = nullptr;
        DidChange();
    }
}



// Update the sprite for the selected planet, or create a new planet.
void SystemView::ChangePlanet()
{
    if(!system)
        return;

    if(selectedObject && selectedObject->Parent() < 0 && !selectedObject->IsStation())
    {
        system->ChangeSprite(selectedObject);
        DidChange();
    }
    else if(!selectedObject)
    {
        system->AddPlanet();
        DidChange();
    }
}



// Update the sprite for the selected moon, or create a new moon for the selected planet.
void SystemView::ChangeMoon()
{
    if(!system)
        return;

    if(selectedObject && selectedObject->Parent() >= 0 && !selectedObject->IsStation())
    {
        system->ChangeSprite(selectedObject);
        DidChange();
    }
    else if(selectedObject && selectedObject->Parent() < 0)
    {
        int index = selectedObject - &system->Objects().front();
        system->AddMoon(selectedObject);
        selectedObject = &system->Objects()[index];
        DidChange();
    }
}



// Update the sprite for the selected station, or create a new station for the selected planet.
void SystemView::ChangeStation()
{
    if(!system)
        return;

    if(selectedObject && selectedObject->IsStation())
    {
        system->ChangeSprite(selectedObject);
        DidChange();
    }
    else if(selectedObject && selectedObject->Parent() < 0)
    {
        int index = selectedObject - &system->Objects().front();
        system->AddMoon(selectedObject, true);
        selectedObject = &system->Objects()[index];
        DidChange();
    }
}



void SystemView::DeleteObject()
{
    if(selectedObject)
    {
        system->Delete(selectedObject);
        selectedObject = nullptr;
        DidChange();
    }
}



void SystemView::Pause()
{
    isPaused = !isPaused;
}



// Select a StellarObject and/or set the dragging position.
void SystemView::mousePressEvent(QMouseEvent *event)
{
    // Reset the dragging target.
    dragObject = nullptr;

    // Right- and middle-clicking deselects.
    if(event->button() != Qt::LeftButton)
    {
        selectedObject = nullptr;
        update();
        return;
    }
    else if(!system)
        return;

    clickOff = QVector2D(event->pos()) - offset;

    QVector2D pos = MapPoint(event->pos());
    // Check if the click was on a StellarObject (e.g. to start dragging it).
    for(StellarObject &object : system->Objects())
        if(!object.IsStar() && pos.distanceToPoint(object.Position()) < object.Radius())
        {
            // Correct the click position to the object's initial position.
            dragTime.start();
            selectedObject = dragObject = &object;
            clickOff = object.Position() - pos;
            planetView->SetPlanet(&object);
            update();
            return;
        }
}



// Switch the view to that of the targeted StellarObject.
void SystemView::mouseDoubleClickEvent(QMouseEvent *event)
{
    // Only double left-clicks are supported.
    if(event->button() != Qt::LeftButton || !system)
        return;

    clickOff = QVector2D(event->pos()) - offset;

    QVector2D pos = MapPoint(event->pos());
    // Check if the click was on a StellarObject (i.e. to view its details).
    for(StellarObject &object : system->Objects())
        if(!object.IsStar() && pos.distanceToPoint(object.Position()) < object.Radius())
        {
            planetView->SetPlanet(&object);
            tabs->setCurrentWidget(planetView);
            return;
        }
}



// Perform a drag action (panning the view or moving a StellarObject).
// If moving an object, update its properties appropriately.
void SystemView::mouseMoveEvent(QMouseEvent *event)
{
    if(!(event->buttons() & Qt::LeftButton) || !system)
        return;

    if(!dragObject)
        offset = QVector2D(event->pos()) - clickOff;
    else
    {
        QVector2D parentPosition;
        if(dragObject->Parent() >= 0)
            parentPosition = system->Objects()[dragObject->Parent()].Position();

        QVector2D oldPosition = dragObject->Position() - parentPosition;
        QVector2D newPosition = MapPoint(event->pos()) + clickOff - parentPosition;

        if(dragTime.elapsed() < 1000 && oldPosition.distanceToPoint(newPosition) < 5. * scale)
            return;

        double oldAngle = atan2(oldPosition.x(), oldPosition.y());
        double newAngle = atan2(newPosition.x(), newPosition.y());

        double oldRadius = oldPosition.length();
        double newRadius = newPosition.length();

        system->Move(dragObject, newRadius - oldRadius, (newAngle - oldAngle) * TO_DEG);
        system->SetDay(timeStep);
        mapData.SetChanged();
    }
    if(isPaused)
        update();
}



// Zoom in or out from the event point.
void SystemView::wheelEvent(QWheelEvent *event)
{
    // Figure out where in the 100% scale image the click occurred.
    QVector2D point(event->position());
    QVector2D center(.5 * width(), .5 * height());
    // point = origin * scale + offset + center.
    QVector2D origin = (point - offset - center) / scale;

    scale = max(.0625, min(1., scale * exp(event->angleDelta().y() * .001))); // we select only vertical (y axis) scrolling
    // additionally angleDelta returns 8ths of degree, so we should (?) convert into degrees, however that makes scrolling unbearably slow
    
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

    // Draw faint circles showing the region occupied by each planet as it orbits.
    painter.setPen(Qt::NoPen);
    QBrush occupiedBrush(QColor(20, 20, 20));
    painter.setBrush(occupiedBrush);
    double starRadius = system->StarRadius();
    painter.drawEllipse(QPointF(), starRadius, starRadius);
    for(const StellarObject &object : system->Objects())
    {
        double radius = system->OccupiedRadius(object);
        if(!radius)
            continue;

        QPainterPath outside;
        outside.addEllipse(QPointF(), object.Distance() + radius, object.Distance() + radius);
        QPainterPath inside;
        outside.addEllipse(QPointF(), object.Distance() - radius, object.Distance() - radius);
        QPainterPath ring = outside.subtracted(inside);

        painter.drawPath(ring);
    }

    // Draw circles indicating the center and edges of the habitable zone.
    if(!std::isnan(system->HabitableZone()))
    {
        painter.setBrush(Qt::NoBrush);
        double radius = system->HabitableZone();

        QPen innerPen(QColor(255., 200., 0.));
        innerPen.setWidthF(1.5);
        painter.setPen(innerPen);
        painter.drawEllipse(QPointF(), .5 * radius, .5 * radius);

        QPen centerPen(QColor(0., 255., 0.));
        centerPen.setWidthF(1.5);
        painter.setPen(centerPen);
        painter.drawEllipse(QPointF(), radius, radius);

        QPen outerPen(QColor(0., 150., 255.));
        outerPen.setWidthF(1.5);
        painter.setPen(outerPen);
        painter.drawEllipse(QPointF(), 2. * radius, 2. * radius);
    }

    // Draw lines linking objects to their parents.
    QBrush brush(QColor(128, 128, 128));
    painter.setBrush(brush);
    QPen pen(QColor(255, 255, 255));
    pen.setWidthF(1.5);
    painter.setPen(pen);
    for(const StellarObject &object : system->Objects())
    {
        QPointF parent;
        if(object.Parent() >= 0)
            parent = system->Objects()[object.Parent()].Position().toPointF();
        painter.drawLine(object.Position().toPointF(), parent);
    }

    QPen blue(QColor(0, 128, 255));
    blue.setWidthF(2.5);
    painter.setPen(blue);
    painter.setBrush(Qt::NoBrush);
    for(const StellarObject &object : system->Objects())
    {
        QPixmap sprite = SpriteSet::Get(object.Sprite());
        QVector2D pos = object.Position();
        double angle = pos.isNull() ? (-2. * PI * timeStep / object.Period()) : atan2(pos.x(), pos.y());
        angle *= TO_DEG;
        angle += 180.;

        painter.translate(pos.toPointF());
        painter.rotate(-angle);
        painter.drawPixmap(QPointF(-.5 * sprite.width(), -.5 * sprite.height()), sprite);
        painter.rotate(angle);
        painter.translate(-pos.toPointF());
        if(!object.GetPlanet().isEmpty())
        {
            double radius = object.Radius() + 5.;
            painter.drawEllipse(pos.toPointF(), radius, radius);
        }
    }
    // Get the bounding box of the paint region after scaling and offset.
    QVector2D half(.5 * width() / scale, .5 * height() / scale);
    QRectF bounds((offset / -scale - half).toPointF(), (offset / -scale + half).toPointF());
    asteroids.Draw(painter, bounds);

    if(selectedObject)
    {
        QPen pen(QColor(255, 255, 255));
        pen.setWidthF(1.5);
        painter.setPen(pen);
        painter.setBrush(Qt::NoBrush);
        double radius = selectedObject->Radius() + 10.;
        painter.drawEllipse(selectedObject->Position().toPointF(), radius, radius);
    }
}



// Figure out where in the 100% scale image the click occurred.
QVector2D SystemView::MapPoint(QPoint pos) const
{
    QVector2D point(pos);
    QVector2D center(.5 * width(), .5 * height());
    // point = origin * scale + offset + center.
    return (point - offset - center) / scale;
}



// If a method did something, this updates the map, date, and draw window.
void SystemView::DidChange()
{
    system->SetDay(timeStep);
    mapData.SetChanged();
    update();
}
