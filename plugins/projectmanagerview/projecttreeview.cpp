/* This file is part of KDevelop
    Copyright 2005 Roberto Raggi <roberto@kdevelop.org>
    Copyright 2007 Andreas Pakulat <apaku@gmx.de>
    Copyright 2009 Aleix Pol <aleixpol@kde.org>

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

#include "projecttreeview.h"


#include <QtGui/QHeaderView>
#include <QtGui/QAbstractProxyModel>
#include <QtCore/QDebug>
#include <QtGui/QMouseEvent>
#include <QApplication>

#include <kxmlguiwindow.h>
#include <kglobalsettings.h>
#include <kaction.h>
#include <kmenu.h>
#include <kdebug.h>
#include <kurl.h>
#include <klocale.h>

#include <project/projectmodel.h>
#include <language/duchain/duchainutils.h>
#include <interfaces/contextmenuextension.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iproject.h>
#include <interfaces/context.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/icore.h>
#include <interfaces/iselectioncontroller.h>
#include <interfaces/isession.h>
#include <project/interfaces/iprojectfilemanager.h>
#include <project/interfaces/ibuildsystemmanager.h>

#include "projectmanagerviewplugin.h"
#include "projectmodelsaver.h"
#include <language/duchain/duchainlock.h>
#include <language/duchain/duchain.h>
#include <language/util/navigationtooltip.h>
#include <project/projectutils.h>

using namespace KDevelop;

static const char* settingsConfigGroup = "ProjectTreeView";

ProjectTreeView::ProjectTreeView( QWidget *parent )
        : QTreeView( parent ), m_ctxProject( 0 )
{
    header()->hide();

    setEditTriggers( QAbstractItemView::EditKeyPressed );

    setContextMenuPolicy( Qt::CustomContextMenu );
    setSelectionMode( QAbstractItemView::ExtendedSelection );

    setIndentation(10);

    setDragEnabled(true);
    setDragDropMode(QAbstractItemView::InternalMove);
    setAutoScroll(true);
    setAutoExpandDelay(300);

    connect( this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(popupContextMenu(QPoint)) );
    connect( this, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(slotActivated(QModelIndex)) );

    connect( ICore::self(), SIGNAL(aboutToShutdown()),
             this, SLOT(aboutToShutdown()));
    connect( ICore::self()->projectController(), SIGNAL(projectOpened(KDevelop::IProject*)),
             this, SLOT(restoreState(KDevelop::IProject*)) );
    connect( ICore::self()->projectController(), SIGNAL(projectClosing(KDevelop::IProject*)),
             this, SLOT(saveState()) );

    restoreState();
}

QList<ProjectFileItem*> fileItemsWithin(const QList<ProjectBaseItem*> items)
{
   QList<ProjectFileItem*> fileItems;
    foreach(ProjectBaseItem* item, items)
    {
        if (ProjectFileItem *file = item->file())
            fileItems.append(file);
        else if (item->folder())
            fileItems.append(fileItemsWithin(item->children()));
    }
    return fileItems;
}

QList<ProjectBaseItem*> topLevelItemsWithin(QList<ProjectBaseItem*> items)
{
    qSort(items.begin(), items.end(), ProjectBaseItem::urlLessThan);
    KUrl lastFolder;
    for (int i = items.size() - 1; i >= 0; --i)
    {
        if (lastFolder.isParentOf(items[i]->url()))
            items.removeAt(i);
        else if (items[i]->folder())
            lastFolder = items[i]->url();
    }
    return items;
}

template<class T>
void filterDroppedItems(QList<T*> &items, ProjectBaseItem* dest)
{
    for (int i = items.size() - 1; i >= 0; --i)
    {
        //No drag and drop from and to same location
        if (items[i]->parent() == dest)
            items.removeAt(i);
        //No moving between projects (technically feasible if the projectmanager is the same though...)
        else if (items[i]->project() != dest->project())
            items.removeAt(i);
    }
}

ProjectBaseItem* ProjectTreeView::itemAtPos(QPoint pos)
{
    QAbstractProxyModel *proxy = qobject_cast<QAbstractProxyModel*>(model());
    QModelIndex index = proxy->mapToSource(indexAt(pos));
    return projectModel()->itemFromIndex(index);
}

void ProjectTreeView::dropEvent(QDropEvent* event)
{
    ProjectItemContext* selectionCtxt =
            static_cast<ProjectItemContext*>(KDevelop::ICore::self()->selectionController()->currentSelection());
    ProjectBaseItem* destItem = itemAtPos(event->pos());
    if (destItem && (dropIndicatorPosition() == AboveItem || dropIndicatorPosition() == BelowItem))
            destItem = destItem->parent();
    if (selectionCtxt && destItem)
    {
        if (ProjectFolderItem *folder = destItem->folder())
        {
            KMenu dropMenu(this);

            QString seq = QKeySequence( Qt::ShiftModifier ).toString();
            seq.chop(1); // chop superfluous '+'
            QAction* move = new QAction(i18n( "&Move Here" ) + '\t' + seq, &dropMenu);
            move->setIcon(KIcon("go-jump"));
            dropMenu.addAction(move);

            seq = QKeySequence( Qt::ControlModifier ).toString();
            seq.chop(1);
            QAction* copy = new QAction(i18n( "&Copy Here" ) + '\t' + seq, &dropMenu);
            copy->setIcon(KIcon("edit-copy"));
            dropMenu.addAction(copy);

            dropMenu.addSeparator();

            QAction* cancel = new QAction(i18n( "C&ancel" ) + '\t' + QKeySequence( Qt::Key_Escape ).toString(), &dropMenu);
            cancel->setIcon(KIcon("process-stop"));
            dropMenu.addAction(cancel);

            QAction *executedAction = 0;

            Qt::KeyboardModifiers modifiers = QApplication::keyboardModifiers();
            if (modifiers == Qt::ControlModifier) {
                executedAction = copy;
            } else if (modifiers == Qt::ShiftModifier) {
                executedAction = move;
            } else {
                executedAction = dropMenu.exec(this->mapToGlobal(event->pos()));
            }

            QList<ProjectBaseItem*> usefulItems = topLevelItemsWithin(selectionCtxt->items());
            filterDroppedItems(usefulItems, destItem);
            QList<KUrl> urls;
            foreach (ProjectBaseItem* i, usefulItems) {
                urls << i->url();
            }
            bool success = false;
            if (executedAction == copy) {
                success =~ destItem->project()->projectFileManager()->copyFilesAndFolders(urls, folder);
            } else if (executedAction == move) {
                success =~ destItem->project()->projectFileManager()->moveFilesAndFolders(usefulItems, folder);
            }

            if (success) {
                QAbstractProxyModel *proxy = qobject_cast<QAbstractProxyModel*>(model());

                //expand target folder
                expand( proxy->mapFromSource(projectModel()->indexFromItem(folder)));

                //and select new items
                QItemSelection selection;
                foreach (const KUrl &url, urls) {
                    KUrl targetUrl = folder->url();
                    targetUrl.addPath(url.fileName());
                    foreach (ProjectBaseItem *item, folder->children()) {
                        KUrl itemUrl = item->url();
                        itemUrl.adjustPath(KUrl::RemoveTrailingSlash); //required to correctly compare urls
                        if (itemUrl == targetUrl) {
                            QModelIndex indx = proxy->mapFromSource( projectModel()->indexFromItem( item ) );
                            selection.append(QItemSelectionRange(indx, indx));
                            setCurrentIndex(indx);
                        }
                    }
                }
                selectionModel()->select(selection, QItemSelectionModel::ClearAndSelect);
            }
        }
        else if (destItem->target() && destItem->project()->buildSystemManager())
        {
            KMenu dropMenu(this);

            QString seq = QKeySequence( Qt::ControlModifier ).toString();
            seq.chop(1);
            QAction* addToTarget = new QAction(i18n( "&Add to Target" ) + '\t' + seq, &dropMenu);
            addToTarget->setIcon(KIcon("edit-link"));
            dropMenu.addAction(addToTarget);

            dropMenu.addSeparator();

            QAction* cancel = new QAction(i18n( "C&ancel" ) + '\t' + QKeySequence( Qt::Key_Escape ).toString(), &dropMenu);
            cancel->setIcon(KIcon("process-stop"));
            dropMenu.addAction(cancel);

            QAction *executedAction = 0;

            Qt::KeyboardModifiers modifiers = QApplication::keyboardModifiers();
            if (modifiers == Qt::ControlModifier) {
                executedAction = addToTarget;
            } else {
                executedAction = dropMenu.exec(this->mapToGlobal(event->pos()));
            }
            if (executedAction == addToTarget) {
                QList<ProjectFileItem*> usefulItems = fileItemsWithin(selectionCtxt->items());
                filterDroppedItems(usefulItems, destItem);
                destItem->project()->buildSystemManager()->addFilesToTarget(usefulItems, destItem->target());
            }
        }
    }
    event->accept();
}

ProjectTreeView::~ProjectTreeView()
{
}

ProjectFolderItem *ProjectTreeView::currentFolderItem() const
{
    Q_ASSERT( projectModel() != 0 );

    QAbstractProxyModel *proxy = qobject_cast<QAbstractProxyModel*>(model());
    QItemSelectionModel *selection = selectionModel();
    QModelIndex current = proxy->mapToSource(selection->currentIndex());

    while ( current.isValid() )
    {
        if ( ProjectFolderItem *folderItem = dynamic_cast<ProjectFolderItem*>( projectModel()->itemFromIndex( current ) ) )
            return folderItem;

        current = proxy->mapFromSource(projectModel()->parent( current ));
    }

    return 0;
}

ProjectFileItem *ProjectTreeView::currentFileItem() const

{
    Q_ASSERT( projectModel() != 0 );

    QAbstractProxyModel *proxy = qobject_cast<QAbstractProxyModel*>(model());
    QItemSelectionModel *selection = selectionModel();
    QModelIndex current = proxy->mapToSource(selection->currentIndex());

    while ( current.isValid() )
    {
        if ( ProjectFileItem *fileItem = dynamic_cast<ProjectFileItem*>( projectModel()->itemFromIndex( current ) ) )
            return fileItem;

        current = proxy->mapFromSource(projectModel()->parent( current ));
    }

    return 0;
}

ProjectTargetItem *ProjectTreeView::currentTargetItem() const

{
    Q_ASSERT( projectModel() != 0 );

    QItemSelectionModel *selection = selectionModel();
    QModelIndex current = selection->currentIndex();

    while ( current.isValid() )
    {
        if ( ProjectTargetItem *targetItem = dynamic_cast<ProjectTargetItem*>( projectModel()->itemFromIndex( current ) ) )
            return targetItem;

        current = projectModel()->parent( current );
    }

    return 0;
}

KDevelop::ProjectModel *ProjectTreeView::projectModel() const
{
    KDevelop::ProjectModel *ret;
    QAbstractProxyModel *proxy = qobject_cast<QAbstractProxyModel*>(model());
    ret=qobject_cast<KDevelop::ProjectModel*>( proxy->sourceModel() );
    Q_ASSERT(ret);
    return ret;
}

void ProjectTreeView::slotActivated( const QModelIndex &index )
{
    QAbstractProxyModel *proxy = qobject_cast<QAbstractProxyModel*>(model());
    KDevelop::ProjectBaseItem *item = projectModel()->itemFromIndex( proxy->mapToSource(index) );
    if ( item && item->file() )
    {
        emit activateUrl( item->file()->url() );
    }
}
//TODO test whether this could be replaced by projectbuildsetwidget.cpp::showContextMenu_appendActions
void popupContextMenu_appendActions(QMenu& menu, const QList<QAction*>& actions)
{
    menu.addActions(actions);
    menu.addSeparator();
}

void ProjectTreeView::popupContextMenu( const QPoint &pos )
{
    QList<KDevelop::ProjectBaseItem*> itemlist;

    if ( indexAt(pos).isValid() )
    {
        QAbstractProxyModel *proxy = qobject_cast<QAbstractProxyModel*>(model());
        QModelIndexList indexes = selectionModel()->selectedRows();

        foreach( const QModelIndex& index, indexes )
        {
            if ( KDevelop::ProjectBaseItem *item = projectModel()->itemFromIndex( proxy->mapToSource(index) ) )
                itemlist << item;
        }
    }

    if( !itemlist.isEmpty() )
    {
        m_ctxProject = itemlist.at(0)->project();
    } else
    {
        m_ctxProject = 0;
    }

    KMenu menu( this );

    KDevelop::ProjectItemContext context(itemlist);
    QList<ContextMenuExtension> extensions = ICore::self()->pluginController()->queryPluginsForContextMenuExtensions( &context );

    QList<QAction*> buildActions;
    QList<QAction*> vcsActions;
    QList<QAction*> extActions;
    QList<QAction*> projectActions;
    QList<QAction*> fileActions;
    QList<QAction*> runActions;
    foreach( const ContextMenuExtension& ext, extensions )
    {
        buildActions += ext.actions(ContextMenuExtension::BuildGroup);
        fileActions += ext.actions(ContextMenuExtension::FileGroup);
        projectActions += ext.actions(ContextMenuExtension::ProjectGroup);
        vcsActions += ext.actions(ContextMenuExtension::VcsGroup);
        extActions += ext.actions(ContextMenuExtension::ExtensionGroup);
        runActions += ext.actions(ContextMenuExtension::RunGroup);
    }

    popupContextMenu_appendActions(menu, buildActions);
    popupContextMenu_appendActions(menu, runActions );
    popupContextMenu_appendActions(menu, fileActions);
    popupContextMenu_appendActions(menu, vcsActions);
    popupContextMenu_appendActions(menu, extActions);

    if ( !itemlist.isEmpty() && itemlist.size() == 1 && itemlist[0]->folder() && !itemlist[0]->folder()->parent() )
    {
        KAction* projectConfig = new KAction(i18n("Open Configuration..."), this);
        projectConfig->setIcon(KIcon("configure"));
        connect( projectConfig, SIGNAL(triggered()), this, SLOT(openProjectConfig()) );
        projectActions << projectConfig;
    }
    popupContextMenu_appendActions(menu, projectActions);
    
    if(!itemlist.isEmpty())
        KDevelop::populateParentItemsMenu(itemlist.front(), &menu);

    if ( !menu.isEmpty() ) {
        menu.exec( mapToGlobal( pos ) );
    }
}

void ProjectTreeView::openProjectConfig()
{
    if( m_ctxProject )
    {
        IProjectController* ip = ICore::self()->projectController();
        ip->configureProject( m_ctxProject );
    }
}

void ProjectTreeView::saveState()
{
    KConfigGroup configGroup( ICore::self()->activeSession()->config(), settingsConfigGroup );

    ProjectModelSaver saver;
    saver.setView( this );
    saver.saveState( configGroup );
}

void ProjectTreeView::restoreState(IProject* project)
{
    KConfigGroup configGroup( ICore::self()->activeSession()->config(), settingsConfigGroup );

    // Saver will delete itself when it is complete.
    ProjectModelSaver *saver = new ProjectModelSaver;
    saver->setProject( project );
    saver->setView( this );
    saver->restoreState( configGroup );
}

void ProjectTreeView::aboutToShutdown()
{
    // save all projects, not just the last one that is closed
    disconnect( ICore::self()->projectController(), SIGNAL(projectClosing(KDevelop::IProject*)),
                this, SLOT(saveState()) );
    saveState();
}

bool ProjectTreeView::event(QEvent* event)
{
    if(event->type()==QEvent::ToolTip)
    {
        QPoint p=mapFromGlobal(QCursor::pos());
        QModelIndex idxView = indexAt(p);
        
        QAbstractProxyModel *proxy = qobject_cast<QAbstractProxyModel*>(model());
        QModelIndex idx = proxy->mapToSource(idxView);
        
        ProjectBaseItem* it=projectModel()->itemFromIndex(idx);
        if((m_idx!=idx || !m_tooltip) && it && it->file())
        {
            m_idx=idx;
            ProjectFileItem* file=it->file();
            KDevelop::DUChainReadLocker lock(KDevelop::DUChain::lock());
            TopDUContext* top= DUChainUtils::standardContextForUrl(file->url());
            
            if(m_tooltip)
                m_tooltip->close();
            
            if(top)
            {
                QWidget* navigationWidget = top->createNavigationWidget();
                if( navigationWidget )
                {
                    m_tooltip = new KDevelop::NavigationToolTip(this, mapToGlobal(p) + QPoint(40, 0), navigationWidget);
                    m_tooltip->resize( navigationWidget->sizeHint() + QSize(10, 10) );
                    kDebug() << "tooltip size" << m_tooltip->size();
                    ActiveToolTip::showToolTip(m_tooltip);
                    return true;
                }
            }
        }
    }
    
    return QAbstractItemView::event(event);
}


void ProjectTreeView::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Return && currentIndex().isValid() && state()!=QAbstractItemView::EditingState)
    {
        event->accept();
        slotActivated(currentIndex());
    }
    else
        QTreeView::keyPressEvent(event);
}


#include "projecttreeview.moc"

