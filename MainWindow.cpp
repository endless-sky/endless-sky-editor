/* MainWindow.cpp
Copyright (c) 2015 by Michael Zahniser

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#include "MainWindow.h"

#include "DetailView.h"
#include "GalaxyView.h"
#include "Map.h"
#include "PlanetView.h"
#include "SystemView.h"

#include <QAction>
#include <QDragEnterEvent>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QMimeData>
#include <QSizePolicy>
#include <QTabWidget>
#include <QUrl>

using namespace std;



MainWindow::MainWindow(Map &map, QWidget *parent)
    : QMainWindow(parent), map(map)
{
    CreateWidgets();
    CreateMenus();
    setAcceptDrops(true);

    resize(1200, 900);
    show();
}



MainWindow::~MainWindow()
{
}



void MainWindow::DoOpen(const QString &path)
{
    if(path.isEmpty())
        return;

    map.Load(path);
    galaxyView->Center();
    systemView->Select(nullptr);
    planetView->Reinitialize();
    tabs->setCurrentWidget(galaxyView);
    update();
    galaxyView->update();
}



void MainWindow::Open()
{
    // TODO: ask about saving changes.
    QString dir = map.DataDirectory();
    QString path = QFileDialog::getOpenFileName(this, "Open map file", dir);
    if(!path.isEmpty())
        DoOpen(path);
}



void MainWindow::Save()
{
    QString dir = map.DataDirectory();
    QString path = QFileDialog::getSaveFileName(this, "Save map file", dir);
    if(!path.isEmpty())
        map.Save(path);
}




void MainWindow::Quit()
{
    close();
}



void MainWindow::TabChanged(int)
{
    if(tabs)
    {
        galaxyMenu->setEnabled(tabs->currentWidget() == galaxyView);
        systemMenu->setEnabled(tabs->currentWidget() == systemView);
    }
}



void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if(tabs)
    {
        if(tabs->currentWidget() == galaxyView)
            galaxyView->KeyPress(event);
    }
}



void MainWindow::closeEvent(QCloseEvent */*event*/)
{
    if(map.IsChanged())
    {
        QMessageBox::StandardButton button = QMessageBox::question(this, "Save changes?",
            "Save changes to the map file before quitting?");
        if(button == QMessageBox::Yes)
            Save();
    }
}



void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    const QMimeData *mime = event->mimeData();
    if(mime->hasUrls() && mime->urls().size() == 1 && mime->text().startsWith("file://"))
        event->acceptProposedAction();
}



void MainWindow::dropEvent(QDropEvent *event)
{
    QString path = event->mimeData()->urls().at(0).path();
#ifdef _WIN32
    // Clean up Windows paths.
    if(path.startsWith("/"))
        path = path.mid(1);
#endif
    DoOpen(path);
}



void MainWindow::CreateWidgets()
{
    QWidget *box = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(box);
    setCentralWidget(box);

    tabs = new QTabWidget(box);
    QSizePolicy policy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    tabs->setSizePolicy(policy);

    galaxyView = new GalaxyView(map, tabs, tabs);

    detailView = new DetailView(map, galaxyView, box);
    detailView->setMinimumWidth(300);
    detailView->setMaximumWidth(300);
    galaxyView->SetDetailView(detailView);

    systemView = new SystemView(map, detailView, tabs, tabs);
    auto it = map.Systems().find("Sol");
    if(it != map.Systems().end())
        systemView->Select(&it->second);
    galaxyView->SetSystemView(systemView);

    planetView = new PlanetView(map, tabs);
    systemView->SetPlanetView(planetView);

    layout->addWidget(detailView);

    tabs->addTab(galaxyView, "Galaxy");
    tabs->addTab(systemView, "System");
    tabs->addTab(planetView, "Planet");
    layout->addWidget(tabs);

    connect(tabs, SIGNAL(currentChanged(int)), this, SLOT(TabChanged(int)));
}



void MainWindow::CreateMenus()
{
    QMenu *fileMenu = menuBar()->addMenu("File");

    QAction *openAction = fileMenu->addAction("Open...", this, SLOT(Open()));
    openAction->setShortcut(QKeySequence::Open);

    QAction *saveAction = fileMenu->addAction("Save...", this, SLOT(Save()));
    saveAction->setShortcut(QKeySequence::Save);
    fileMenu->addSeparator();

    QAction *quitAction = fileMenu->addAction("Quit", this, SLOT(Quit()));
    quitAction->setShortcut(QKeySequence::Quit);


    galaxyMenu = menuBar()->addMenu("Galaxy");

    QAction *deleteSystemAction = galaxyMenu->addAction("Delete System");
    connect(deleteSystemAction, SIGNAL(triggered()), galaxyView, SLOT(DeleteSystem()));
    deleteSystemAction->setShortcut(QKeySequence(Qt::Key_Backspace));

    QAction *randomizeCommodityAction = galaxyMenu->addAction("Randomize Commodity");
    connect(randomizeCommodityAction, SIGNAL(triggered()), galaxyView, SLOT(RandomizeCommodity()));
    randomizeCommodityAction->setShortcut(QKeySequence("C"));


    systemMenu = menuBar()->addMenu("System");

    QAction *randomInhabited = systemMenu->addAction("Randomize (Inhabited)");
    connect(randomInhabited, SIGNAL(triggered()), systemView, SLOT(RandomizeInhabited()));
    randomInhabited->setShortcut(QKeySequence("I"));

    QAction *randomSystem = systemMenu->addAction("Randomize");
    connect(randomSystem, SIGNAL(triggered()), systemView, SLOT(Randomize()));
    randomSystem->setShortcut(QKeySequence("R"));

    QAction *randomUninhabited = systemMenu->addAction("Randomize (Uninhabited)");
    connect(randomUninhabited, SIGNAL(triggered()), systemView, SLOT(RandomizeUninhabited()));
    randomUninhabited->setShortcut(QKeySequence("U"));

    systemMenu->addSeparator();

    QAction *changeAsteroids = systemMenu->addAction("Change Asteroids");
    connect(changeAsteroids, SIGNAL(triggered()), systemView, SLOT(ChangeAsteroids()));
    changeAsteroids->setShortcut(QKeySequence("A"));

    QAction *changeMinables = systemMenu->addAction("Change Minables");
    connect(changeMinables, SIGNAL(triggered()), systemView, SLOT(ChangeMinables()));
    changeMinables->setShortcut(QKeySequence("H"));

    systemMenu->addSeparator();

    QAction *changeStar = systemMenu->addAction("Change Star");
    connect(changeStar, SIGNAL(triggered()), systemView, SLOT(ChangeStar()));
    changeStar->setShortcut(QKeySequence("T"));

    QAction *addPlanet = systemMenu->addAction("Add/Change Planet");
    connect(addPlanet, SIGNAL(triggered()), systemView, SLOT(ChangePlanet()));
    addPlanet->setShortcut(QKeySequence("P"));

    QAction *addMoon = systemMenu->addAction("Add/Change Moon");
    connect(addMoon, SIGNAL(triggered()), systemView, SLOT(ChangeMoon()));
    addMoon->setShortcut(QKeySequence("M"));

    QAction *addStation = systemMenu->addAction("Add/Change Station");
    connect(addStation, SIGNAL(triggered()), systemView, SLOT(ChangeStation()));
    addStation->setShortcut(QKeySequence("S"));

    QAction *deleteObject = systemMenu->addAction("Delete Object");
    connect(deleteObject, SIGNAL(triggered()), systemView, SLOT(DeleteObject()));
    deleteObject->setShortcut(QKeySequence("X"));

    systemMenu->addSeparator();

    QAction *pause = systemMenu->addAction("Pause/Unpause");
    connect(pause, SIGNAL(triggered()), systemView, SLOT(Pause()));
    pause->setShortcut(QKeySequence(Qt::Key_Space));

    // Activate only the menu for the current tab.
    TabChanged(0);
}
