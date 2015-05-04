/* DataWriter.h
Copyright (c) 2014 by Michael Zahniser

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#include "DataWriter.h"

#include "DataNode.h"

using namespace std;



const QString DataWriter::space = " ";



DataWriter::DataWriter(const QString &path)
    : before(&indent), file(path)
{
    if(file.open(QFile::WriteOnly | QFile::Text))
    {
        out.setDevice(&file);
        out.setCodec("UTF-8");
    }
}



void DataWriter::Write(const DataNode &node)
{
    for(int i = 0; i < node.Size(); ++i)
        WriteToken(node.Token(i));
    Write();

    if(node.begin() != node.end())
    {
        BeginChild();
        {
            for(const DataNode &child : node)
                Write(child);
        }
        EndChild();
    }
}



void DataWriter::Write()
{
    out << '\n';
    before = &indent;
}



void DataWriter::BeginChild()
{
    indent += '\t';
}



void DataWriter::EndChild()
{
    indent.chop(1);
}



void DataWriter::WriteComment(const QString &str)
{
    out << indent << "# " << str << '\n';
}



void DataWriter::WriteRaw(const QString &str)
{
    out << str;
}



void DataWriter::WriteToken(const QString &str, QChar quote)
{
    bool hasSpace = str.isEmpty() || (quote == '"');
    bool hasQuote = (quote == '`');
    for(QChar c : str)
    {
        hasSpace |= (c <= ' ');
        hasQuote |= (c == '"');
    }

    out << *before;
    if(hasSpace && hasQuote)
        out << '`' << str << '`';
    else if(hasSpace)
        out << '"' << str << '"';
    else
        out << str;
    before = &space;
}
