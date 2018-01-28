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

#include "DetailView.h"
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
#include <cmath>
#include <map>
#include <set>
#include <stack>

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
    setToolTip("Left click to select a system. Drag to move a system or pan the view.\n"
        "Right click to create a new system or to toggle links between systems.\n"
        "Use the scroll wheel to zoom in and out.");

    Center();
}



void GalaxyView::Center()
{
    if(mapData.Systems().empty())
        return;

    offset = QVector2D();
    for(const auto &it : mapData.Systems())
        offset -= it.second.Position();

    offset /= mapData.Systems().size();
}



void GalaxyView::SetSystemView(SystemView *view)
{
    systemView = view;
}



void GalaxyView::SetDetailView(DetailView *view)
{
    detailView = view;
}



// Color the map by this commodity.
void GalaxyView::SetCommodity(const QString &name)
{
    if(commodity != name)
    {
        commodity = name;
        government.clear();
        update();
    }
}



// Color the map by this government.
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



// Create a system at (0, 0).
void GalaxyView::CreateSystem()
{
    // TODO: Create a custom event filter to receive the next mouse click event
    // TODO: Draw crosshairs and / or a simulated system and neighbor ring when
    // this custom filter is active, then pass the coordinates here (rather than
    // always using 0, 0).
    CreateSystem({0, 0});
}



// Change the name of a system, which involves creating a new pointer
// and updating the system and detail views with it.
bool GalaxyView::RenameSystem(const QString &from, const QString &to)
{
    if(!mapData.Systems().count(from))
    {
        QMessageBox::warning(this, "Missing name",
            "A system named \"" + from + "\" didn't exist.");
        mapData.Systems()[from];
        mapData.SetChanged();
        update();
        return false;
    }
    // If the desired name is empty, prompt to delete the system.
    else if(to.isEmpty())
    {
        if(systemView && systemView->Selected())
            DeleteSystem();
        else
            return false;
    }
    else if(mapData.Systems().count(to))
    {
        QMessageBox::warning(this, "Duplicate name",
            "A system named \"" + to + "\" already exists.");
        return false;
    }
    else
    {
        mapData.RenameSystem(from, to);
        mapData.SetChanged();

        // Update the system pointed to by the two views, as the old pointer is invalid.
        System *newSystem = &mapData.Systems()[to];
        if(systemView)
            systemView->Select(newSystem);
        if(detailView)
            detailView->SetSystem(newSystem);

        // Redraw the Galaxy map using the new system's name.
        update();
    }

    return true;
}



// Delete a system, and remove any string references to it (i.e. links).
void GalaxyView::DeleteSystem()
{
    if(!systemView || !systemView->Selected())
        return;

    System *system = systemView->Selected();
    QMessageBox::StandardButton button = QMessageBox::question(this, "Delete system",
        "Are you sure you want to delete \"" + system->Name() + "\"?");
    if(button == QMessageBox::Yes)
    {
        // Deselect this system.
        systemView->Select(nullptr);
        // Remove all links from this system (and the corresponding return links).
        while(!system->Links().empty())
            system->ToggleLink(&mapData.Systems().find(*system->Links().begin())->second);

        // Remove this system from known systems.
        mapData.Systems().erase(system->Name());
        mapData.SetChanged();
    }
    update();
}



void GalaxyView::Recenter()
{
    Center();
    update();
}



void GalaxyView::RandomizeCommodity()
{
    // Randomize the values of the currently selected commodity.
    
    // First, make sure a system and a commodity are selected.
    if(commodity.isEmpty() || !systemView || !systemView->Selected())
        return;
    
    // Next, find all the systems connected via hyperlinks to the current system.
    set<System *> connected;
    stack<System *> edge;
    edge.push(systemView->Selected());
    while(!edge.empty())
    {
        System *system = edge.top();
        edge.pop();
        
        if(connected.count(system))
            continue;
        connected.insert(system);
        
        for(const QString &name : system->Links())
            if(mapData.Systems().count(name))
                edge.push(&mapData.Systems()[name]);
    }
    
    // Commodity parameters.
    static const map<QString, int> BASE = {
        {"Clothing", 140},
        {"Electronics", 590},
        {"Equipment", 330},
        {"Food", 100},
        {"Heavy Metals", 610},
        {"Industrial", 520},
        {"Luxury Goods", 920},
        {"Medical", 430},
        {"Metal", 190},
        {"Plastic", 240}
    };
    static const map<QString, vector<int>> BINS = {
        {"Clothing", {20, 60, 20}},
        {"Electronics", {30, 40, 30}},
        {"Equipment", {30, 20, 20, 30}},
        {"Food", {24, 18, 16, 18, 24}},
        {"Heavy Metals", {8, 12, 20, 20, 20, 12, 8}},
        {"Industrial", {20, 30, 30, 20}},
        {"Luxury Goods", {25, 20, 15, 10, 10, 20}},
        {"Medical", {20, 20, 20, 20, 20}},
        {"Metal", {30, 25, 20, 25}},
        {"Plastic", {40, 20, 40}}
    };
    auto baseIt = BASE.find(commodity);
    auto binIt = BINS.find(commodity);
    if(baseIt == BASE.end() || binIt == BINS.end())
        return;
    
    // Generate the quotas for each bin level.
    const int base = baseIt->second;
    
    // Try to find a set of bins to assign the systems to such that neighboring
    // systems only differ by one bin, and the desired distribution is achieved.
    map<const System *, int> bin;
    for(int tries = 0; true; ++tries)
    {
        // Each time we try 4 times to match the quota and are unable to,
        // loosen the quota a little bit.
        vector<int> quota;
        for(int weight : binIt->second)
            quota.emplace_back((connected.size() * weight) / 100 + tries / 4 + 1);
        
        vector<const System *> unassigned;
        map<const System *, int> low;
        map<const System *, int> high;
        for(const System *system : connected)
        {
            unassigned.push_back(system);
            low[system] = 0;
            high[system] = quota.size();
        }
        
        while(!unassigned.empty())
        {
            int i = rand() % unassigned.size();
            const System *system = unassigned[i];
            unassigned[i] = unassigned.back();
            unassigned.pop_back();
            
            // Pick a bin, based on what is available.
            int possibilities = 0;
            for(int i = low[system]; i < high[system]; ++i)
                possibilities += quota[i];
            if(!possibilities)
                break;
            
            // Pick a random one of those items to assign to it.
            int index = rand() % possibilities;
            int choice = low[system];
            while(true)
            {
                index -= quota[choice];
                if(index < 0)
                    break;
                ++choice;
            }
            --quota[choice];
            
            // Record our choice.
            bin[system] = choice;
            int newLow = low[system] = choice;
            int newHigh = high[system] = choice + 1;
            
            // Starting from this star, trace outwards system by system. Each
            // neighboring system must be within 1 of this star's level; each
            // system neighboring those, within 2, and so on.
            vector<const System *> sources = {system};
            set<const System *> done = {system};
            while(!sources.empty())
            {
                // For each step outward, expand the allowable range.
                --newLow;
                ++newHigh;
                
                vector<const System *> next;
                
                // Check if any systems adjacent to any of the sources must be
                // updated.
                for(const System *source : sources)
                    for(const QString &name : source->Links())
                    {
                        const auto &it = mapData.Systems().find(name);
                        if(it == mapData.Systems().end() || done.count(&it->second))
                            continue;
                        const System *link = &it->second;
                        done.insert(link);
                        
                        // No need to go further if this system is already at
                        // least as constrained as the new constraints.
                        if(low[link] >= newLow && high[link] <= newHigh)
                            continue;
                        
                        low[link] = max(low[link], newLow);
                        high[link] = min(high[link], newHigh);
                        next.push_back(link);
                    }
                
                // Now, visit neighbors of those neighbors.
                next.swap(sources);
            }
        }
        if(unassigned.empty())
            break;
    }
    
    // Assign each star system a value based on its bin.
    map<const System *, int> rough;
    for(const auto &it : bin)
        rough[it.first] = base + (rand() % 100) + 100 * it.second;
    
    // Smooth out the values by averaging each system with the average of all
    // its neighbors.
    for(System *system : connected)
    {
        int count = 0;
        int sum = 0;
        for(const QString &link : system->Links())
            if(mapData.Systems().count(link))
            {
                sum += rough[&mapData.Systems()[link]];
                ++count;
            }

        if(!count)
            sum = rough[system];
        else
        {
            sum += count * rough[system];
            sum = (sum + count) / (2 * count);
        }
        system->SetTrade(commodity, sum);
    }
    mapData.SetChanged();
    if(detailView)
        detailView->UpdateCommodities();
    update();
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
            CreateSystem(origin);
        return;
    }
    // An existing system was clicked. Select it, or toggle a link with the
    // already-selected system.
    if(event->button() == Qt::LeftButton)
    {
        dragTime.start();
        clickOff = QVector2D(event->pos());
        if(systemView)
        {
            systemView->Select(dragSystem);
            // Update the coloring scheme if coloring by government.
            if(!government.isEmpty() && !dragSystem->Government().isEmpty())
                government = dragSystem->Government();
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



// Drag either the selected system, or the background.
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



// Zoom in or out.
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

    // Draw the links between systems.
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

    // Draw the systems, colored by commodity or if the government is the selected government.
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

    // Draw the selection circle and neighbor radius ring.
    painter.setPen(mediumPen);
    painter.setBrush(Qt::NoBrush);
    if(systemView && systemView->Selected())
    {
        QPointF pos = systemView->Selected()->Position().toPointF();
        painter.drawEllipse(pos, 10, 10);
        painter.drawEllipse(pos, 100, 100);
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



// Create a system at the given position.
void GalaxyView::CreateSystem(const QVector2D &origin)
{
    QString text = QInputDialog::getText(this, "New system", "Name:");
    if(!text.isEmpty())
    {
        if(mapData.Systems().count(text))
            QMessageBox::warning(this, "Duplicate name",
                "A system named \"" + text + "\" already exists.");
        else
        {
            System &system = mapData.Systems()[text];
            system.Init(text, origin);
            // If a previous system was selected, the new system extends from it.
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
