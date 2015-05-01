/* DetailView.h
Copyright (c) 2015 by Michael Zahniser

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#ifndef DETAILVIEW_H
#define DETAILVIEW_H

#include <QWidget>

#include <QList>

class Map;
class System;

class QLineEdit;
class QTreeWidgetItem;



class DetailView : public QWidget
{
    Q_OBJECT
public:
    explicit DetailView(Map &mapData, QWidget *parent = 0);

    void SetSystem(System *system);

signals:

public slots:

private:
    Map &mapData;
    System *system = nullptr;

    QLineEdit *name;
    QLineEdit *government;

    QList<QTreeWidgetItem *> trade;
};



#endif // DETAILVIEW_H
