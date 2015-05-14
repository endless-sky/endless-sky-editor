/* LandscapeView.h
Copyright (c) 2015 by Michael Zahniser

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#ifndef LANDSCAPEVIEW_H
#define LANDSCAPEVIEW_H

#include <QWidget>

class Map;
class Planet;



class LandscapeView : public QWidget
{
    Q_OBJECT
public:
    explicit LandscapeView(const Map &mapData, QWidget *parent = 0);
    ~LandscapeView();

    void SetPlanet(Planet *planet);
    const QString &Landscape() const;

signals:

public slots:

protected:
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void paintEvent(QPaintEvent *event) override;


private:
    void SetLandscape(const QString &name);


private:
    const Map &mapData;
    Planet *planet = nullptr;
    bool showGallery = false;
    QString landscape;
};

#endif // LANDSCAPEVIEW_H
