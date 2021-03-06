/* This file is part of KDevelop
    Copyright 2010 Aleix Pol Gonzalez <aleixpol@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "projectchangesmodel.h"
#include <KIcon>
#include <KPluginInfo>
#include <KLocalizedString>
#include <vcs/interfaces/ibasicversioncontrol.h>
#include <interfaces/iplugin.h>
#include <interfaces/iproject.h>
#include <interfaces/icore.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/iprojectcontroller.h>
#include <vcs/vcsstatusinfo.h>
#include <vcs/vcsjob.h>
#include <interfaces/iruncontroller.h>
#include <interfaces/idocumentcontroller.h>
#include <project/projectmodel.h>

using namespace KDevelop;

ProjectChangesModel::ProjectChangesModel(QObject* parent)
    : VcsFileChangesModel(parent)
{
    foreach(IProject* p, ICore::self()->projectController()->projects())
        addProject(p);
    
    connect(ICore::self()->projectController(), SIGNAL(projectOpened(KDevelop::IProject*)),
                                              SLOT(addProject(KDevelop::IProject*)));
    connect(ICore::self()->projectController(), SIGNAL(projectClosing(KDevelop::IProject*)),
                                              SLOT(removeProject(KDevelop::IProject*)));
    
    connect(ICore::self()->documentController(), SIGNAL(documentSaved(KDevelop::IDocument*)),
                                                SLOT(documentSaved(KDevelop::IDocument*)));
    connect(ICore::self()->projectController()->projectModel(), SIGNAL(rowsInserted(QModelIndex,int,int)),
                                                SLOT(itemsAdded(QModelIndex,int,int)));
    
    connect(ICore::self()->runController(), SIGNAL(jobUnregistered(KJob*)), SLOT(jobUnregistered(KJob*)));
}

ProjectChangesModel::~ProjectChangesModel()
{}

void ProjectChangesModel::addProject(IProject* p)
{
    QStandardItem* it = new QStandardItem(p->name());
    QStandardItem* itStatus = new QStandardItem;
    if(p->versionControlPlugin()) {
        IPlugin* plugin = p->versionControlPlugin();
        
        IBasicVersionControl* vcs = plugin->extension<IBasicVersionControl>();

        KPluginInfo info = ICore::self()->pluginController()->pluginInfo(plugin);
        
        itStatus->setIcon(KIcon(info.icon()));
        itStatus->setText(vcs->name());
        reload(QList<IProject*>() << p);
    } else {
        it->setEnabled(false);
        itStatus->setEnabled(false);
        itStatus->setText(i18n("No Version Control support for this project."));
    }
    
    appendRow(QList<QStandardItem*>() << it << itStatus);
}

void ProjectChangesModel::removeProject(IProject* p)
{
    QStandardItem* it=projectItem(p);
    
    removeRow(it->row());
}

QStandardItem* findItemChild(QStandardItem* parent, const QVariant& value, int role = Qt::DisplayRole)
{
    for(int i=0; i<parent->rowCount(); i++) {
        QStandardItem* curr=parent->child(i);
        
        if(curr->data(role) == value)
            return curr;
    }
    return 0;
}

QStandardItem* ProjectChangesModel::projectItem(IProject* p) const
{
    return findItemChild(invisibleRootItem(), p->name());
}

void ProjectChangesModel::addStates(const QVariantList& states)
{
    IProjectController* projectController = ICore::self()->projectController();
        
    foreach(const QVariant& state, states) {
        VcsStatusInfo st = state.value<VcsStatusInfo>();
        
        IProject* project = projectController->findProjectForUrl(st.url());
        
        if(project)
            updateState(project, st);
    }
}

void ProjectChangesModel::updateState(IProject* p, const KDevelop::VcsStatusInfo& status)
{
    QStandardItem* pItem = projectItem(p);
    Q_ASSERT(pItem);
    
    VcsFileChangesModel::updateState(pItem, status);
}

void ProjectChangesModel::changes(IProject* project, const KUrl::List& urls, IBasicVersionControl::RecursionMode mode)
{
    IPlugin* vcsplugin=project->versionControlPlugin();
    IBasicVersionControl* vcs = vcsplugin ? vcsplugin->extension<IBasicVersionControl>() : 0;
    
    if(vcs && vcs->isVersionControlled(urls.first())) { //TODO: filter?
        VcsJob* job=vcs->status(urls, mode);
        connect(job, SIGNAL(finished(KJob*)), SLOT(statusReady(KJob*)));
        
        ICore::self()->runController()->registerJob(job);
    }
}

void ProjectChangesModel::statusReady(KJob* job)
{
    VcsJob* status=static_cast<VcsJob*>(job);
    
    addStates(status->fetchResults().toList());
}

void ProjectChangesModel::documentSaved(KDevelop::IDocument* document)
{
    reload(KUrl::List() << document->url());
}

void ProjectChangesModel::itemsAdded(const QModelIndex& parent, int start, int end)
{
    ProjectModel* model=ICore::self()->projectController()->projectModel();
    ProjectBaseItem* item=model->itemFromIndex(parent);

    if(!item)
        return;

    IProject* project=item->project();
    
    if(!project)
        return;
    
    KUrl::List urls;
    
    for(int i=start; i<end; i++) {
        QModelIndex idx=parent.child(i, 0);
        item=model->itemFromIndex(idx);
        
        if(item->type()==ProjectBaseItem::File || item->type()==ProjectBaseItem::Folder || item->type()==ProjectBaseItem::BuildFolder)
            urls += item->url();
    }
        
    if(!urls.isEmpty())
        changes(project, urls, KDevelop::IBasicVersionControl::NonRecursive);
}

void ProjectChangesModel::reload(const QList<IProject*>& projects)
{
    foreach(IProject* project, projects)
        changes(project, project->folder(), KDevelop::IBasicVersionControl::Recursive);
}

void ProjectChangesModel::reload(const QList<KUrl>& urls)
{
    foreach(const KUrl& url, urls) {
        IProject* project=ICore::self()->projectController()->findProjectForUrl(url);
        
        if(project)
            changes(project, url, KDevelop::IBasicVersionControl::NonRecursive);
    }
}

void ProjectChangesModel::reloadAll()
{
    QList< IProject* > projects = ICore::self()->projectController()->projects();
    reload(projects);
}

void ProjectChangesModel::jobUnregistered(KJob* job)
{
    static QList<VcsJob::JobType> readOnly = QList<VcsJob::JobType>()
		<< KDevelop::VcsJob::Add
		<< KDevelop::VcsJob::Remove
		<< KDevelop::VcsJob::Pull
		<< KDevelop::VcsJob::Commit
		<< KDevelop::VcsJob::Move
        << KDevelop::VcsJob::Copy
        << KDevelop::VcsJob::Revert
		;
    
    VcsJob* vcsjob=dynamic_cast<VcsJob*>(job);
    if(vcsjob && readOnly.contains(vcsjob->type())) {
        reloadAll();
    }
}
