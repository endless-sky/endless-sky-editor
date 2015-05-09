/* SystemView.h
Copyright (c) 2015 by Michael Zahniser

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#ifndef SYSTEMVIEW_H
#define SYSTEMVIEW_H

#include "AsteroidField.h"

#include <QWidget>

#include <QElapsedTimer>
#include <QVector2D>
#include <QTimer>

class DetailView;
class PlanetView;
class StellarObject;
class System;

class QTabWidget;



class SystemView : public QWidget
{
    Q_OBJECT
public:
    explicit SystemView(DetailView *detailView, QTabWidget *tabs, QWidget *parent = 0);

    void Select(System *system);
    System *Selected() const;

    void SetPlanetView(PlanetView *view);

    virtual QSize minimumSizeHint() const override;
    virtual QSize sizeHint() const override;

signals:

public slots:
    void step();
    void RandomizeInhabited();
    void Randomize();
    void RandomizeUninhabited();
    void ChangeAsteroids();
    void ChangeStar();
    void ChangePlanet();
    void ChangeMoon();
    void ChangeStation();
    void DeleteObject();
    void Pause();

protected:
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseDoubleClickEvent(QMouseEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;
    virtual void wheelEvent(QWheelEvent *event) override;

    virtual void paintEvent(QPaintEvent *event) override;

private:
    QVector2D MapPoint(QPoint pos) const;


private:
    DetailView *detailView;
    QTabWidget *tabs;
    PlanetView *planetView;
    System *system = nullptr;
    StellarObject *selectedObject = nullptr;

    // Center and scale of the view:
    QVector2D offset;
    double scale = 1.;

    // Time:
    QTimer timer;
    double timeStep = 1000.;
    bool isPaused = false;

    // Dragging:
    QVector2D clickOff;
    StellarObject *dragObject = nullptr;
    QElapsedTimer dragTime;

    AsteroidField asteroids;
};

#endif // SYSTEMVIEW_H
