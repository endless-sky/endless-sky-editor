/* LandscapeLoader.h
Copyright (c) 2015 by Michael Zahniser

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#ifndef LANDSCAPELOADER_H
#define LANDSCAPELOADER_H

#include <QThread>

#include <QImage>
#include <QMutex>
#include <QString>

#include <list>
#include <map>
#include <vector>



class LandscapeLoader : public QThread
{
    Q_OBJECT
public:
    explicit LandscapeLoader(QObject *parent = 0);

    void Init();
    void Update();
    void Quit();

    const std::vector<QString> &Available() const;


signals:

public slots:

protected:
    virtual void run() override;

private:
    // This mutex controls access to the following:
    QMutex mutex;
    std::list<QString> toLoad;
    std::map<QString, QImage> loaded;
    int initCount = 0;

    // This object does not require mutex access:
    std::vector<QString> available;
};



#endif // LANDSCAPELOADER_H
