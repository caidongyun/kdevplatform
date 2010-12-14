/**************************************************************************
*   Copyright 2010 Silvère Lestang <silvere.lestang@gmail.com>            *
*   Copyright 2010 Julien Desgats <julien.desgats@gmail.com>              *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef GREPOUTPUTVIEW_H
#define GREPOUTPUTVIEW_H

#include <interfaces/iuicontroller.h>

#include "ui_grepoutputview.h"

namespace KDevelop
{
    class IStatus;
}

class QModelIndex;

class GrepViewPlugin;
class GrepOutputModel;
class GrepOutputDelegate;

class GrepOutputViewFactory: public KDevelop::IToolViewFactory
{
public:
    GrepOutputViewFactory();
    virtual QWidget* create(QWidget *parent = 0);
    virtual Qt::DockWidgetArea defaultPosition();
    virtual QString id() const;
};

class GrepOutputView : public QWidget, Ui::GrepOutputView
{
  Q_OBJECT

public:
    GrepOutputView(QWidget* parent);
    ~GrepOutputView();
    GrepOutputModel* model();
    
    /**
     * This causes the creation of a new model, the old one is deleted.
     * This is necessary because the previous job, if any, is not killed instantly and
     * sometimes continues to feed the model.
     * @return pointer to the new model
     */
    GrepOutputModel* renewModel();
    
    void setMessage(const QString& msg);
    void setPlugin(GrepViewPlugin *plugin);

public Q_SLOTS:
    void showErrorMessage( const QString& errorMessage );
    void showMessage( KDevelop::IStatus*, const QString& message );
    void updateApplyState(const QModelIndex &topLeft, const QModelIndex &bottomRight);

Q_SIGNALS:
    void outputViewIsClosed();
    
private:
    QAction* m_next;
    QAction* m_prev;
    QAction* m_collapseAll;
    QAction* m_expandAll;
    GrepViewPlugin *m_plugin;
    
private slots:
    void selectPreviousItem();
    void selectNextItem();
    void collapseAllItems();
    void expandAllItems();
    void onApply();
    void showDialog();
    void expandRootElement( const QModelIndex & parent );
    void rowsRemoved();
};

#endif // GREPOUTPUTVIEW_H