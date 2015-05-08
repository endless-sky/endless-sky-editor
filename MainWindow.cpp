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
#include "SystemView.h"

#include <QAction>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QMenu>
#include <QMenuBar>
#include <QSizePolicy>
#include <QTabWidget>

#include <string>

using namespace std;



MainWindow::MainWindow(Map &map, QWidget *parent)
    : QMainWindow(parent), map(map)
{
    CreateMenus();
    CreateWidgets();
}

MainWindow::~MainWindow()
{
}



void MainWindow::Open()
{
    // TODO: ask about saving changes.
    QString dir = map.DataDirectory();
    QString path = QFileDialog::getOpenFileName(this, "Open map file", dir);
    if(!path.isEmpty())
    {
        map.Load(path);
        systemView->Select(nullptr);
        update();
    }
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
    // TODO: ask about saving changes.
    close();
}



void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if(tabs)
    {
        if(tabs->currentWidget() == galaxyView)
            galaxyView->KeyPress(event);
    }
}



void MainWindow::CreateMenus()
{
    QMenu *fileMenu = menuBar()->addMenu("File");
    QAction *openAction = fileMenu->addAction("Open...", this, SLOT(Open()));
    openAction->setShortcut(QKeySequence::Open);
    QAction *saveAction = fileMenu->addAction("Save...", this, SLOT(Save()));
    saveAction->setShortcut(QKeySequence::Save);
    QAction *quitAction = fileMenu->addAction("Quit", this, SLOT(Quit()));
    quitAction->setShortcut(QKeySequence::Quit);
}



void MainWindow::CreateWidgets()
{
    QHBoxLayout *layout = new QHBoxLayout;
    QWidget *box = new QWidget(this);
    box->setLayout(layout);
    setCentralWidget(box);

    tabs = new QTabWidget(box);
    QSizePolicy policy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    tabs->setSizePolicy(policy);

    galaxyView = new GalaxyView(map, tabs, tabs);

    detailView = new DetailView(map, galaxyView, box);
    detailView->setMinimumWidth(300);

    systemView = new SystemView(detailView, tabs, tabs);
    auto it = map.Systems().find("Sol");
    if(it != map.Systems().end())
        systemView->Select(&it->second);
    galaxyView->SetSystemView(systemView);

    layout->addWidget(detailView);

    tabs->addTab(galaxyView, "Galaxy");
    tabs->addTab(systemView, "System");
    layout->addWidget(tabs);
}
