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

class Planet;



class LandscapeView : public QWidget
{
    Q_OBJECT
public:
    explicit LandscapeView(QWidget *parent = 0);

    void SetPlanet(Planet *planet);

signals:

public slots:

protected:
    virtual void paintEvent(QPaintEvent *event) override;

private:
    Planet *planet = nullptr;
};

#endif // LANDSCAPEVIEW_H
