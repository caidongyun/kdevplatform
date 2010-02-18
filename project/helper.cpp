/* This file is part of KDevelop
    Copyright 2010 Milian Wolff <mail@milianw.de>

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

#include "helper.h"

#include <KTemporaryFile>
#include <KIO/NetAccess>
#include <kio/job.h>
#include <kio/copyjob.h>
#include <KMessageBox>
#include <KLocalizedString>

#include <QApplication>
#include <interfaces/iproject.h>
#include <vcs/interfaces/ibasicversioncontrol.h>
#include <interfaces/iplugin.h>
#include <vcs/vcsjob.h>

bool KDevelop::removeUrl(const KUrl& url, const bool isFolder)
{
    QWidget* window(QApplication::activeWindow());
    int q=KMessageBox::questionYesNo(window,
        isFolder ? i18n("Do you want to remove the directory <i>%1</i> from the filesystem too?", url.pathOrUrl())
                 : i18n("Do you want to remove the file <i>%1</i> from the filesystem too?", url.pathOrUrl()));
    if(q==KMessageBox::Yes)
    {
        if ( !KIO::NetAccess::del( url, window ) ) {
            KMessageBox::error( window,
                isFolder ? i18n( "Cannot remove folder <i>%1</i>.", url.pathOrUrl() )
                         : i18n( "Cannot remove file <i>%1</i>.", url.pathOrUrl() ) );
            return false;
        }
        return true;
    }
    return false;
}

bool KDevelop::createFile(const KUrl& file)
{
    if (KIO::NetAccess::exists( file, KIO::NetAccess::SourceSide, QApplication::activeWindow() )) {
        KMessageBox::error( QApplication::activeWindow(),
                            i18n( "The file <i>%1</i> exists already.", file.pathOrUrl() ) );
        return false;
    }

    {
        KTemporaryFile temp;
        if ( !temp.open() || temp.write("\n") == -1 ) {
            KMessageBox::error( QApplication::activeWindow(),
                                i18n( "Cannot create temporary file to create <i>%1</i>.", file.pathOrUrl() ) );
            return false;
        }
        if ( !KIO::NetAccess::upload( temp.fileName(), file, QApplication::activeWindow() ) ) {
            KMessageBox::error( QApplication::activeWindow(),
                                i18n( "Cannot create file <i>%1</i>.", file.pathOrUrl() ) );
            return false;
        }
    }
    return true;
}

bool KDevelop::createFolder(const KUrl& folder)
{
    if ( !KIO::NetAccess::mkdir( folder, QApplication::activeWindow() ) ) {
        KMessageBox::error( QApplication::activeWindow(), i18n( "Cannot create folder <i>%1</i>.", folder.pathOrUrl() ) );
        return false;
    }
    return true;
}

bool KDevelop::renameUrl(const KDevelop::IProject* project, const KUrl& oldname, const KUrl& newname)
{
    IPlugin* vcsplugin=project->versionControlPlugin();
    IBasicVersionControl* vcs=0;
    if(vcsplugin)
        vcs=vcsplugin->extension<IBasicVersionControl>();
    
    if(!vcs->isVersionControlled(oldname))
        vcs=0;
    
    bool ret=false;
    if(vcs) {
        VcsJob* job=vcs->move(oldname, newname);
        ret=job->exec();
    } else {
        KIO::CopyJob* job=KIO::move(oldname, newname);
        ret=KIO::NetAccess::synchronousRun(job, 0);
    }
    return ret;
}