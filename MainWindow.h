/* MainWindow.h
Copyright (c) 2015 by Michael Zahniser

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class Map;
class DetailView;
class GalaxyView;
class SystemView;
class PlanetView;

class QMenu;
class QTabWidget;



class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(Map &map, QWidget *parent = 0);
    ~MainWindow();

public slots:
    void Open();
    void Save();
    void Quit();

    void TabChanged(int);

protected:
    virtual void keyPressEvent(QKeyEvent *event) override;
    virtual void closeEvent(QCloseEvent *event) override;

private:
    void CreateWidgets();
    void CreateMenus();


private:
    Map &map;

    QTabWidget *tabs = nullptr;
    DetailView *detailView = nullptr;
    GalaxyView *galaxyView = nullptr;
    SystemView *systemView = nullptr;
    PlanetView *planetView = nullptr;

    QMenu *galaxyMenu = nullptr;
    QMenu *systemMenu = nullptr;
};

#endif // MAINWINDOW_H
