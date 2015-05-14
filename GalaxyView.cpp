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

#include <QInputDialog>
#include <QMessageBox>
#include <QPainter>
#include <QPalette>
#include <QMouseEvent>
#include <QTabWidget>

#include <algorithm>

using namespace std;

namespace {
    // Map a value between -1 and 1 to a color.
    QColor MapColor(double value)
    {
        value = min(1., max(-1., value));
        if(value < 0.)
            return QColor(
                (.12 + .12 * value) * 255.9,
                (.48 + .36 * value) * 255.9,
                (.48 - .12 * value) * 255.9);
        else
            return QColor(
                (.12 + .48 * value) * 255.9,
                (.48 + .00 * value) * 255.9,
                (.48 - .48 * value) * 255.9);
    }
    QColor MapGrey(double value)
    {
        value = max(0., min(1., (value + 1.) / 2.));
        return QColor(
            200. * value + 55.9,
            200. * value + 55.9,
            200. * value + 55.9);
    }
}



GalaxyView::GalaxyView(Map &mapData, QTabWidget *tabs, QWidget *parent) :
    QWidget(parent), mapData(mapData), tabs(tabs)
{
    setAutoFillBackground(true);
    QPalette p = palette();
    p.setColor(backgroundRole(), QColor(0, 0, 0));
    setPalette(p);

    for(const auto &it : mapData.Systems())
        offset -= it.second.Position();
    offset /= mapData.Systems().size();
}



void GalaxyView::SetSystemView(SystemView *view)
{
    systemView = view;
}



void GalaxyView::SetCommodity(const QString &name)
{
    if(commodity != name)
    {
        commodity = name;
        government.clear();
        update();
    }
}



void GalaxyView::SetGovernment(const QString &name)
{
    if(government != name)
    {
        government = name;
        commodity.clear();
        update();
    }
}



void GalaxyView::KeyPress(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace)
        DeleteSystem();
}



void GalaxyView::DeleteSystem()
{
    if(!systemView || !systemView->Selected())
        return;

    System *system = systemView->Selected();
    QMessageBox::StandardButton button = QMessageBox::question(this, "Delete system",
        "Are you sure you want to delete \"" + system->Name() + "\"?");
    if(button == QMessageBox::Yes)
    {
        systemView->Select(nullptr);
        while(!system->Links().empty())
            system->ToggleLink(&mapData.Systems().find(system->Links().front())->second);

        auto it = mapData.Systems().find(system->Name());
        mapData.Systems().erase(it);
        mapData.SetChanged();
    }
}



void GalaxyView::mousePressEvent(QMouseEvent *event)
{
    clickOff = QVector2D(event->pos()) - offset;

    dragSystem = nullptr;
    QVector2D origin = MapPoint(event->pos());
    for(auto &it : mapData.Systems())
        if(origin.distanceToPoint(it.second.Position()) < 10.)
        {
            dragSystem = &it.second;
            break;
        }

    if(!dragSystem)
    {
        if(event->button() == Qt::RightButton)
        {
            QString text = QInputDialog::getText(this, "New system", "Name:");
            if(!text.isEmpty())
            {
                auto it = mapData.Systems().find(text);
                if(it != mapData.Systems().end())
                    QMessageBox::warning(this, "Duplicate name",
                        "A system named \"" + text + "\" already exists.");
                else
                {
                    System &system = mapData.Systems()[text];
                    system.Init(text, origin);
                    if(systemView && systemView->Selected())
                    {
                        System &previous = *systemView->Selected();
                        for(const Map::Commodity &commodity : mapData.Commodities())
                            system.SetTrade(commodity.name, previous.Trade(commodity.name));
                        system.SetGovernment(previous.Government());
                    }
                    else
                        for(const Map::Commodity &commodity : mapData.Commodities())
                            system.SetTrade(commodity.name, (commodity.low + commodity.high) / 2);
                    if(systemView)
                        systemView->Select(&system);
                    mapData.SetChanged();
                    update();
                }
            }
        }
        return;
    }
    if(event->button() == Qt::LeftButton)
    {
        dragTime.start();
        clickOff = QVector2D(event->pos());
        if(systemView)
        {
            systemView->Select(dragSystem);
            update();
        }
    }
    else if(event->button() == Qt::RightButton)
    {
        if(systemView && systemView->Selected())
        {
            systemView->Selected()->ToggleLink(dragSystem);
            mapData.SetChanged();
            update();
        }
        dragSystem = nullptr;
    }
}



void GalaxyView::mouseDoubleClickEvent(QMouseEvent *event)
{
    if(event->button() != Qt::LeftButton)
        return;

    clickOff = QVector2D(event->pos()) - offset;
    if(!tabs || !systemView)
        return;

    QVector2D origin = MapPoint(event->pos());
    for(auto &it : mapData.Systems())
        if(origin.distanceToPoint(it.second.Position()) < 5.)
        {
            systemView->Select(&it.second);
            tabs->setCurrentWidget(systemView);
            return;
        }
}



void GalaxyView::mouseMoveEvent(QMouseEvent *event)
{
    if(!(event->buttons() & Qt::LeftButton))
        return;

    QVector2D distance = QVector2D(event->pos()) - clickOff;
    if(!dragSystem)
        offset = distance;
    else
    {
        if(dragTime.elapsed() < 1000 && distance.length() < 5.)
            return;

        dragSystem->SetPosition(dragSystem->Position() + distance / scale);
        mapData.SetChanged();
        clickOff = QVector2D(event->pos());
    }
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
    QPen blackPen;
    QPen mediumPen(QColor(120, 120, 120));
    QPen brightPen(QColor(180, 180, 180));

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    painter.translate(.5 * width(), .5 * height());
    painter.translate(offset.x(), offset.y());
    painter.scale(scale, scale);

    // Draw the "galaxy" images.
    for(const Galaxy &it : mapData.Galaxies())
    {
        QPixmap sprite = SpriteSet::Get(it.Sprite());
        QPointF pos = (it.Position() - QVector2D(.5 * sprite.width(), .5 * sprite.height())).toPointF();
        painter.drawPixmap(pos, sprite);
    }

    painter.setBrush(Qt::NoBrush);
    for(const auto &it : mapData.Systems())
    {
        QPointF pos = it.second.Position().toPointF();
        for(const QString &link : it.second.Links())
        {
            auto lit = mapData.Systems().find(link);
            if(lit == mapData.Systems().end())
                continue;

            double value = 0.;
            if(!commodity.isEmpty())
            {
                int difference = abs(it.second.Trade(commodity) - lit->second.Trade(commodity));
                value = (difference - 60) / 60.;
            }
            else if(!government.isEmpty())
                value = (it.second.Government() != lit->second.Government());
            // Set the link color based on the "value".
            QPen pen(value < 1. ? MapGrey(value) : QColor(255, 0, 0));
            painter.setPen(pen);
            painter.drawLine(pos, lit->second.Position().toPointF());
        }
    }

    for(const auto &it : mapData.Systems())
    {
        QPointF pos = it.second.Position().toPointF();
        bool isSelected = (systemView && &it.second == systemView->Selected());
        double value = 0.;
        if(!commodity.isEmpty())
            value = mapData.MapPrice(commodity, it.second.Trade(commodity)) * 2. - 1.;
        else if(!government.isEmpty())
            value = (it.second.Government() == government);
        // Set the link color based on the "value".
        QColor color = MapColor(value);
        if(isSelected)
            color.setRgbF(color.redF() * 1.5, color.greenF() * 1.5, color.blueF() * 1.5);
        QBrush brush(color);
        painter.setBrush(brush);
        painter.setPen(blackPen);
        painter.drawEllipse(pos, 5, 5);

        painter.drawText(pos + QPointF(6, 6), it.first);
        painter.setPen(brightPen);
        painter.drawText(pos + QPointF(5, 5), it.first);
    }

    painter.setPen(mediumPen);
    painter.setBrush(Qt::NoBrush);
    if(systemView && systemView->Selected())
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
