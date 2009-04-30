/* This file is part of the KDE project
   Copyright (C) 2002 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
   Copyright (C) 2002 John Firebaugh <jfirebaugh@kde.org>
   Copyright (C) 2007 Hamish Rodda <rodda@kde.org>
   Copyright (C) 2009 Niko Sams <niko.sams@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KDEVELOP_IBREAKPOINTCONTROLLER_H
#define KDEVELOP_IBREAKPOINTCONTROLLER_H

#include <QtCore/QObject>
#include <QtCore/QMap>
#include <QtCore/QSet>
#include "../debuggerexport.h"
#include "idebugsession.h"

class QModelIndex;
namespace KDevelop {
class BreakpointModel;
class Breakpoint;
class IDebugSession;

class KDEVPLATFORMDEBUGGER_EXPORT IBreakpointController : public QObject
{
    Q_OBJECT
public:
    enum BreakpointState {
        DirtyState,
        PendingState,
        CleanState
    };

    IBreakpointController(IDebugSession* parent);
    
    BreakpointState breakpointState(const Breakpoint *breakpoint) const;

protected:
    IDebugSession *debugSession() const;
    BreakpointModel *breakpointModel() const;

    void breakpointStateChanged(Breakpoint* breakpoint);

    void sendMaybeAll();
    virtual void sendMaybe(Breakpoint *breakpoint) = 0;

    QMap<const Breakpoint*, QSet<int> > m_dirty;
    QSet<const Breakpoint*> m_pending;
    bool m_dontSendChanges;

private Q_SLOTS:
    void dataChanged(const QModelIndex &topRight, const QModelIndex &bottomLeft);
    void breakpointDeleted(KDevelop::Breakpoint *breakpoint);
};

}

#endif // KDEVELOP_IBREAKPOINTCONTROLLER_H