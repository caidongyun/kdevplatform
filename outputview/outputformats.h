/***************************************************************************
 *   Copyright 1999-2001 Bernd Gehrmann <bernd@kdevelop.org>               *
 *   Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>                         *
 *   Copyright (C) 2012  Morten Danielsen Volden mvolden2@gmail.com        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef OUTPUTFORMATS_H
#define OUTPUTFORMATS_H

#include <QString>
#include <QRegExp>

namespace KDevelop
{

class ActionFormat
{
    public:
        ActionFormat( const QString& _action, const QString& _tool, const QString& regExp, int file );
        ActionFormat( const QString& _action, int tool, int file, const QString& regExp );
        QString action;
        QRegExp expression;
        QString tool;
        int toolGroup;
        int fileGroup;
};

class ErrorFormat
{
    public:
        ErrorFormat( const QString& regExp, int file, int line, int text, int column=-1 );
        ErrorFormat( const QString& regExp, int file, int line, int text, const QString& comp, int column=-1 );
        QRegExp expression;
        int fileGroup;
        int lineGroup, columnGroup;
        int textGroup;
        QString compiler;
};

}
#endif



