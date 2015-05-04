/* DataWriter.h
Copyright (c) 2014 by Michael Zahniser

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#ifndef DATA_WRITER_H_
#define DATA_WRITER_H_

#include <QFile>
#include <QString>
#include <QTextStream>

class DataNode;



// This class writes data in a hierarchical format, where an indented line is
// considered the "child" of the first line above it that is less indented. By
// using this class, you can have a function add data to the file without having
// to tell that function what indentation level it is at. This class also
// automatically adds quotation marks around strings if they contain whitespace.
class DataWriter {
public:
    DataWriter(const QString &path);

  template <class ...B>
    void Write(const char *a, B... others);
  template <class ...B>
    void Write(const QString &a, B... others);
  template <class A, class ...B>
    void Write(const A &a, B... others);
    void Write(const DataNode &node);
    void Write();

    void BeginChild();
    void EndChild();

    // Write a raw string. It's your responsibility to make sure this string
    // does not mess up the file formatting, since no checks are done on it.
    void WriteRaw(const QString &str);
    void WriteComment(const QString &str);
    void WriteToken(const QString &str, QChar quote = '\0');


private:
    QString indent;
    static const QString space;
    const QString *before;

    QFile file;
    QTextStream out;
};



template <class ...B>
void DataWriter::Write(const char *a, B... others)
{
    WriteToken(QString::fromUtf8(a));
    Write(others...);
}



template <class ...B>
void DataWriter::Write(const QString &a, B... others)
{
    WriteToken(a);
    Write(others...);
}



template <class A, class ...B>
void DataWriter::Write(const A &a, B... others)
{
    static_assert(std::is_arithmetic<A>::value,
        "DataWriter cannot output anything but strings and arithmetic types.");

    out << *before << a;
    before = &space;

    Write(others...);
}



#endif
