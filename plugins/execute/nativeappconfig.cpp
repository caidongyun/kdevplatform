/*  This file is part of KDevelop
    Copyright 2009 Andreas Pakulat <apaku@gmx.de>

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
#include "nativeappconfig.h"

#include <klocale.h>
#include <kdebug.h>
#include <kicon.h>

#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/ilaunchconfiguration.h>

#include <project/projectmodel.h>

#include "nativeappjob.h"
#include "executepluginconstants.h"

KIcon NativeAppConfigPage::icon() const
{
    return KIcon("system-run");
}

//TODO: Make sure to auto-add the executable target to the dependencies when its used.

void NativeAppConfigPage::loadFromConfiguration(const KConfigGroup& cfg) 
{
    bool b = blockSignals( true );
    if( cfg.readEntry( ExecutePlugin::isExecutableEntry, false ) ) 
    {
        executableRadio->setChecked( true );
        executablePath->setUrl( cfg.readEntry( ExecutePlugin::executableEntry, KUrl() ) );
    } else 
    {
        projectTargetRadio->setChecked( true );
        projectTarget->setText( cfg.readEntry( ExecutePlugin::projectTargetEntry, "" ) );
    }
    arguments->setText( cfg.readEntry( ExecutePlugin::argumentsEntry, "" ) );
    workingDirectory->setUrl( cfg.readEntry( ExecutePlugin::workingDirEntry, KUrl() ) );
    environment->setCurrentProfile( cfg.readEntry( ExecutePlugin::environmentGroupEntry, "default" ) );
    runInTerminal->setChecked( cfg.readEntry( ExecutePlugin::useTerminalEntry, false ) );
    dependencies->addItems( cfg.readEntry( ExecutePlugin::dependencyEntry, QStringList() ) );
    dependencyAction->setCurrentIndex( dependencyAction->findData( cfg.readEntry( ExecutePlugin::dependencyActionEntry, "Nothing" ) ) );
    blockSignals( b );
}

NativeAppConfigPage::NativeAppConfigPage( QWidget* parent ) 
    : LaunchConfigurationPage( parent )
{
    setupUi(this);
    //Setup completions for the target lineedits
    projectTarget->setCompleter( new ProjectItemCompleter( KDevelop::ICore::self()->projectController()->projectModel(), this ) );
    targetDependency->setCompleter( new ProjectItemCompleter( KDevelop::ICore::self()->projectController()->projectModel(), this ) );
    
    //Setup data info for combobox
    dependencyAction->setItemData(0, "Nothing" );
    dependencyAction->setItemData(1, "Build" );
    dependencyAction->setItemData(2, "Install" );
    dependencyAction->setItemData(3, "SudoInstall" );
    
    //connect signals to changed signal
    connect( projectTarget, SIGNAL(textEdited(const QString&)), SIGNAL(changed()) );
    connect( projectTargetRadio, SIGNAL(toggled(bool)), SIGNAL(changed()) );
    connect( executableRadio, SIGNAL(toggled(bool)), SIGNAL(changed()) );
    connect( executablePath->lineEdit(), SIGNAL(textEdited(const QString&)), SIGNAL(changed()) );
    connect( executablePath, SIGNAL(urlSelected(const KUrl&)), SIGNAL(changed()) );
    connect( arguments, SIGNAL(textEdited(const QString&)), SIGNAL(changed()) );
    connect( workingDirectory, SIGNAL(urlSelected(const KUrl&)), SIGNAL(changed()) );
    connect( workingDirectory->lineEdit(), SIGNAL(textEdited(const QString&)), SIGNAL(changed()) );
    connect( environment, SIGNAL(currentIndexChanged(int)), SIGNAL(changed()) );
    connect( addDependency, SIGNAL(clicked(bool)), SIGNAL(changed()) );
    connect( removeDependency, SIGNAL(clicked(bool)), SIGNAL(changed()) );
    connect( moveDepDown, SIGNAL(clicked(bool)), SIGNAL(changed()) );
    connect( moveDepUp, SIGNAL(clicked(bool)), SIGNAL(changed()) );
    connect( dependencyAction, SIGNAL(currentIndexChanged(int)), SIGNAL(changed()) );
    connect( runInTerminal, SIGNAL(toggled(bool)), SIGNAL(changed()) );
}

void NativeAppConfigPage::saveToConfiguration(KConfigGroup cfg) const
{
    cfg.writeEntry( ExecutePlugin::isExecutableEntry, executableRadio->isChecked() );
    if( executableRadio-> isChecked() )
    {
        cfg.writeEntry( ExecutePlugin::executableEntry, executablePath->url() );
        cfg.deleteEntry( ExecutePlugin::projectTargetEntry );
    } else
    {
        cfg.writeEntry( ExecutePlugin::projectTargetEntry, projectTarget->text() );
        cfg.deleteEntry( ExecutePlugin::executableEntry );
    }
    cfg.writeEntry( ExecutePlugin::argumentsEntry, arguments->text() );
    cfg.writeEntry( ExecutePlugin::workingDirEntry, workingDirectory->url() );
    cfg.writeEntry( ExecutePlugin::environmentGroupEntry, environment->currentProfile() );
    cfg.writeEntry( ExecutePlugin::useTerminalEntry, runInTerminal->isChecked() );
    cfg.writeEntry( ExecutePlugin::dependencyActionEntry, dependencyAction->itemData( dependencyAction->currentIndex() ).toString() );
    QStringList deps;
    for( int i = 0; i < dependencies->count(); i++ )
    {
        deps << dependencies->item( i )->text();
    }
    cfg.writeEntry( ExecutePlugin::dependencyEntry, deps );
}

QString NativeAppConfigPage::title() const 
{
    return i18n("Configure Native Application");
}

QList< KDevelop::LaunchConfigurationPageFactory* > NativeAppLauncher::configPages() const 
{
    return QList<KDevelop::LaunchConfigurationPageFactory*>();
}

QString NativeAppLauncher::description() const
{
    return "Executes Native Applications";
}

QString NativeAppLauncher::id() 
{
    return "nativeAppLauncher";
}

QString NativeAppLauncher::name() const 
{
    return i18n("Native Application");
}

NativeAppLauncher::NativeAppLauncher()
{
}

KJob* NativeAppLauncher::start(const QString& launchMode, KDevelop::ILaunchConfiguration* cfg)
{
    Q_ASSERT(cfg);
    if( !cfg )
    {
        return 0;
    }
    if( launchMode == "execute" )
    {
        return new NativeAppJob( KDevelop::ICore::self()->runController(), cfg );
    }
    kWarning() << "Unknown launch mode for config:" << cfg->name();
    return 0;
}

QStringList NativeAppLauncher::supportedModes() const
{
    return QStringList() << "execute";
}

KDevelop::LaunchConfigurationPage* NativeAppPageFactory::createWidget(QWidget* parent)
{
    return new NativeAppConfigPage( parent );
}

NativeAppPageFactory::NativeAppPageFactory()
{
}

NativeAppConfigType::NativeAppConfigType() 
{
    factoryList.append( new NativeAppPageFactory() );
}

QString NativeAppConfigType::name() const
{
    return i18n("Native Application");
}


QList<KDevelop::LaunchConfigurationPageFactory*> NativeAppConfigType::configPages() const 
{
    return factoryList;
}

QString NativeAppConfigType::id() const 
{
    return ExecutePlugin::nativeAppConfigTypeId;;
}

KIcon NativeAppConfigType::icon() const
{
    return KIcon("system-run");
}



#include "nativeappconfig.moc"