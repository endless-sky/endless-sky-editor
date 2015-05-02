/* GalaxyView.h
Copyright (c) 2015 by Michael Zahniser

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#ifndef GALAXYVIEW_H
#define GALAXYVIEW_H

#include <QWidget>

#include <QVector2D>

class Map;
class SystemView;

class QTabWidget;



class GalaxyView : public QWidget
{
    Q_OBJECT
public:
    explicit GalaxyView(Map &mapData, QTabWidget *tabs, QWidget *parent = 0);

    void SetSystemView(SystemView *view);
    void SetCommodity(const std::string &name);
    void SetGovernment(const std::string &name);

signals:

public slots:

protected:
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseDoubleClickEvent(QMouseEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;
    virtual void wheelEvent(QWheelEvent *event) override;

    virtual void paintEvent(QPaintEvent *event) override;

private:
    QVector2D MapPoint(QPoint pos) const;


private:
    Map &mapData;
    SystemView *systemView = nullptr;
    QTabWidget *tabs;

    // Center and scale of the view:
    QVector2D offset;
    double scale = 1.;

    // Dragging:
    QVector2D clickOff;

    // Color systems by:
    std::string commodity;
    std::string government;
};



#endif // GALAXYVIEW_H
