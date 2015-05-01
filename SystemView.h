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

#include <QWidget>

#include <QVector2D>
#include <QTimer>

class System;



class SystemView : public QWidget
{
    Q_OBJECT
public:
    explicit SystemView(QWidget *parent = 0);

    void SetSystem(System *system);

    virtual QSize minimumSizeHint() const override;
    virtual QSize sizeHint() const override;

signals:

public slots:
    void step();

protected:
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;
    virtual void wheelEvent(QWheelEvent *event) override;

    virtual void paintEvent(QPaintEvent *event) override;

private:
    System *system = nullptr;

    // Center and scale of the view:
    QVector2D offset;
    double scale = 1.;

    // Time:
    QTimer timer;
    double timeStep = 1000.;

    // Dragging:
    QVector2D clickOff;
};

#endif // SYSTEMVIEW_H
