/* This file is part of KDevelop
 *
 * Copyright 1999-2001 John Birch <jbb@kdevelop.org>
 * Copyright 2001 by Bernd Gehrmann <bernd@kdevelop.org>
 * Copyright 2006 Vladimir Prus <ghost@cs.msu.su>
 * Copyright 2007 Hamish Rodda <rodda@kde.org>
 * Copyright 2009 Niko Sams <niko.sams@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "debugcontroller.h"

#include <QtCore/QMetaEnum>

#include <KLocale>
#include <KDebug>
#include <KActionCollection>
#include <KAction>
#include <KParts/Part>
#include <KParts/PartManager>
#include <KTextEditor/Document>
#include <KTextEditor/MarkInterface>

#include "../interfaces/idocument.h"
#include "../interfaces/icore.h"
#include "../interfaces/idocumentcontroller.h"
#include "../interfaces/iuicontroller.h"
#include "../interfaces/contextmenuextension.h"
#include "../interfaces/context.h"
#include "../language/interfaces/editorcontext.h"
#include "../sublime/view.h"
#include "debugger/framestackwidget.h"
#include "core.h"


namespace KDevelop {

template<class T>
class DebuggerToolFactory : public KDevelop::IToolViewFactory
{
public:
  DebuggerToolFactory(DebugController* controller, const QString &id, Qt::DockWidgetArea defaultArea)
  : m_controller(controller), m_id(id), m_defaultArea(defaultArea) {}

  virtual QWidget* create(QWidget *parent = 0)
  {
    return new T(m_controller, parent);
  }

  virtual QString id() const
  {
    return m_id;
  }

  virtual Qt::DockWidgetArea defaultPosition()
  {
    return m_defaultArea;
  }

  virtual void viewCreated(Sublime::View* view)
  {
      if (view->widget()->metaObject()->indexOfSignal(SIGNAL(requestRaise())) != -1)
          QObject::connect(view->widget(), SIGNAL(requestRaise()), view, SLOT(requestRaise()));
  }

  /* At present, some debugger widgets (e.g. breakpoint) contain actions so that shortcuts
     work, but they don't need any toolbar.  So, suppress toolbar action.  */
  virtual QList<QAction*> toolBarActions( QWidget* viewWidget ) const
  {
      Q_UNUSED(viewWidget);
      return QList<QAction*>();
  }

private:
  DebugController* m_controller;
  QString m_id;
  Qt::DockWidgetArea m_defaultArea;
};

DebugController::DebugController(QObject *parent)
    : IDebugController(parent), KXMLGUIClient()
{
    setComponentData(KComponentData("kdevdebugger"));
    setXMLFile("kdevdebuggershellui.rc");

    if((Core::self()->setupFlags() & Core::NoUi)) return;
    setupActions();

    ICore::self()->uiController()->addToolView(
        i18n("Frame Stack"),
        new DebuggerToolFactory<FramestackWidget>(
            this, "org.kdevelop.debugger.StackView",
            Qt::BottomDockWidgetArea));

    foreach(KParts::Part* p, KDevelop::ICore::self()->partController()->parts())
        partAdded(p);
    connect(KDevelop::ICore::self()->partController(),
            SIGNAL(partAdded(KParts::Part*)),
            this,
            SLOT(partAdded(KParts::Part*)));

}

void DebugController::initialize()
{
    stateChanged("stopped");
}


void DebugController::partAdded(KParts::Part* part)
{
    if (KTextEditor::Document* doc = dynamic_cast<KTextEditor::Document*>(part)) {
        KTextEditor::MarkInterface *iface = dynamic_cast<KTextEditor::MarkInterface*>(doc);
        if( !iface )
            return;
        
        iface->setMarkDescription((KTextEditor::MarkInterface::MarkTypes)BreakpointMark, i18n("Breakpoint"));
        iface->setMarkPixmap((KTextEditor::MarkInterface::MarkTypes)BreakpointMark, *inactiveBreakpointPixmap());
        iface->setMarkPixmap((KTextEditor::MarkInterface::MarkTypes)ActiveBreakpointMark, *activeBreakpointPixmap());
        iface->setMarkPixmap((KTextEditor::MarkInterface::MarkTypes)ReachedBreakpointMark, *reachedBreakpointPixmap());
        iface->setMarkPixmap((KTextEditor::MarkInterface::MarkTypes)DisabledBreakpointMark, *disabledBreakpointPixmap());
        iface->setMarkPixmap((KTextEditor::MarkInterface::MarkTypes)ExecutionPointMark, *executionPointPixmap());
        iface->setEditableMarks( BookmarkMark | BreakpointMark );
    }
}

IDebugSession* DebugController::currentSession()
{
    return m_currentSession;
}

void DebugController::setupActions()
{
    KActionCollection* ac = actionCollection();

    KAction* action = m_startDebugger = new KAction(KIcon("dbgrun"), i18n("&Start (not yet working)"), this);
    action->setShortcut(Qt::Key_F9);
    action->setToolTip( i18n("Start in debugger") );
    action->setWhatsThis( i18n("<b>Start in debugger</b><p>"
                               "Starts the debugger with the project's main "
                               "executable. You may set some breakpoints "
                               "before this, or you can interrupt the program "
                               "while it is running, in order to get information "
                               "about variables, frame stack, and so on.</p>") );
    ac->addAction("debug_run", action);
    connect(action, SIGNAL(triggered(bool)), this, SLOT(startDebugger()));

    m_restartDebugger = action = new KAction(KIcon("dbgrestart"), i18n("&Restart"), this);
    action->setToolTip( i18n("Restart program") );
    action->setWhatsThis( i18n("<b>Restarts application</b><p>"
                               "Restarts applications from the beginning.</p>"
                              ) );
    action->setEnabled(false);
    connect(action, SIGNAL(triggered(bool)), this, SLOT(restartDebugger()));
    ac->addAction("debug_restart", action);


    m_stopDebugger = action = new KAction(KIcon("process-stop"), i18n("Sto&p"), this);
    action->setToolTip( i18n("Stop debugger") );
    action->setWhatsThis(i18n("<b>Stop debugger</b><p>Kills the executable and exits the debugger.</p>"));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(stopDebugger()));
    ac->addAction("debug_stop", action);

    m_interruptDebugger = action = new KAction(KIcon("media-playback-pause"), i18n("Interrupt"), this);
    action->setToolTip( i18n("Interrupt application") );
    action->setWhatsThis(i18n("<b>Interrupt application</b><p>Interrupts the debugged process or current debugger command.</p>"));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(interruptDebugger()));
    ac->addAction("debug_pause", action);

    m_runToCursor = action = new KAction(KIcon("dbgrunto"), i18n("Run to &Cursor"), this);
    action->setToolTip( i18n("Run to cursor") );
    action->setWhatsThis(i18n("<b>Run to cursor</b><p>Continues execution until the cursor position is reached.</p>"));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(runToCursor()));
    ac->addAction("debug_runtocursor", action);


    m_jumpToCursor = action = new KAction(KIcon("dbgjumpto"), i18n("Set E&xecution Position to Cursor"), this);
    action->setToolTip( i18n("Jump to cursor") );
    action->setWhatsThis(i18n("<b>Set Execution Position </b><p>Set the execution pointer to the current cursor position.</p>"));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(jumpToCursor()));
    ac->addAction("debug_jumptocursor", action);

    m_stepOver = action = new KAction(KIcon("dbgnext"), i18n("Step &Over"), this);
    action->setShortcut(Qt::Key_F10);
    action->setToolTip( i18n("Step over the next line") );
    action->setWhatsThis( i18n("<b>Step over</b><p>"
                               "Executes one line of source in the current source file. "
                               "If the source line is a call to a function the whole "
                               "function is executed and the app will stop at the line "
                               "following the function call.</p>") );
    connect(action, SIGNAL(triggered(bool)), this, SLOT(stepOver()));
    ac->addAction("debug_stepover", action);


    m_stepOverInstruction = action = new KAction(KIcon("dbgnextinst"), i18n("Step over Ins&truction"), this);
    action->setToolTip( i18n("Step over instruction") );
    action->setWhatsThis(i18n("<b>Step over instruction</b><p>Steps over the next assembly instruction.</p>"));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(stepIntoInstruction()));
    ac->addAction("debug_stepoverinst", action);


    m_stepInto = action = new KAction(KIcon("dbgstep"), i18n("Step &Into"), this);
    action->setShortcut(Qt::Key_F11);
    action->setToolTip( i18n("Step into the next statement") );
    action->setWhatsThis( i18n("<b>Step into</b><p>"
                               "Executes exactly one line of source. If the source line "
                               "is a call to a function then execution will stop after "
                               "the function has been entered.</p>") );
    connect(action, SIGNAL(triggered(bool)), this, SLOT(stepInto()));
    ac->addAction("debug_stepinto", action);


    m_stepIntoInstruction = action = new KAction(KIcon("dbgstepinst"), i18n("Step into I&nstruction"), this);
    action->setToolTip( i18n("Step into instruction") );
    action->setWhatsThis(i18n("<b>Step into instruction</b><p>Steps into the next assembly instruction.</p>"));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(stepOverInstruction()));
    ac->addAction("debug_stepintoinst", action);

    m_stepOut = action = new KAction(KIcon("dbgstepout"), i18n("Step O&ut"), this);
    action->setShortcut(Qt::Key_F12);
    action->setToolTip( i18n("Steps out of the current function") );
    action->setWhatsThis( i18n("<b>Step out</b><p>"
                               "Executes the application until the currently executing "
                               "function is completed. The debugger will then display "
                               "the line after the original call to that function. If "
                               "program execution is in the outermost frame (i.e. in "
                               "main()) then this operation has no effect.</p>") );
    connect(action, SIGNAL(triggered(bool)), this, SLOT(stepOut()));
    ac->addAction("debug_stepout", action);

    m_toggleBreakpoint = action = new KAction(i18n("Toggle Breakpoint"), this);
    action->setToolTip(i18n("Toggle breakpoint"));
    action->setWhatsThis(i18n("<b>Toggle breakpoint</b><p>Toggles the breakpoint at the current line in editor.</p>"));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(toggleBreakpoint()));
    ac->addAction("debug_toggle_breakpoint", action);
}

void DebugController::addSession(IDebugSession* session)
{
    kDebug() << session;

    //TODO support multiple sessions
    if (m_currentSession) {
        m_currentSession->stopDebugger();
    }
    m_currentSession = session;
    
    connect(session, SIGNAL(stateChanged(KDevelop::IDebugSession::DebuggerState)), SLOT(debuggerStateChanged(KDevelop::IDebugSession::DebuggerState)));
    connect(session, SIGNAL(showStepInSource(KUrl,int)), SLOT(showStepInSource(KUrl,int)));
    connect(session, SIGNAL(clearExecutionPoint()), SLOT(clearExecutionPoint()));
    
    updateDebuggerState(session->state(), session);
    
    emit sessionAdded(session);
}

void DebugController::clearExecutionPoint()
{
    kDebug();
    foreach (KDevelop::IDocument* document, KDevelop::ICore::self()->documentController()->openDocuments()) {
        KTextEditor::MarkInterface *iface = dynamic_cast<KTextEditor::MarkInterface*>(document->textDocument());
        if (!iface)
            continue;

        QHashIterator<int, KTextEditor::Mark*> it = iface->marks();
        while (it.hasNext())
        {
            KTextEditor::Mark* mark = it.next().value();
            if( mark->type & ExecutionPointMark )
                iface->removeMark( mark->line, ExecutionPointMark );
        }
    }
}

void DebugController::showStepInSource(const KUrl &url, int lineNum)
{
    clearExecutionPoint();
    kDebug() << url << lineNum;
    KDevelop::IDocument* document = KDevelop::ICore::self()
        ->documentController()
        ->openDocument(url, KTextEditor::Cursor(lineNum, 0));

    if( !document )
        return;

    KTextEditor::MarkInterface *iface = dynamic_cast<KTextEditor::MarkInterface*>(document->textDocument());
    if( !iface )
        return;

    document->textDocument()->blockSignals(true);
    iface->addMark( lineNum, ExecutionPointMark );
    document->textDocument()->blockSignals(false);
}


void DebugController::debuggerStateChanged(KDevelop::IDebugSession::DebuggerState state)
{
    Q_ASSERT(dynamic_cast<IDebugSession*>(sender()));
    IDebugSession* session = static_cast<IDebugSession*>(sender());
    kDebug() << session << state;
    if (session == m_currentSession) {
        updateDebuggerState(state, session);
    }
}

void DebugController::updateDebuggerState(IDebugSession::DebuggerState state, IDebugSession *session)
{
    kDebug() << state;
    switch (state) {
        case IDebugSession::StoppedState:
        case IDebugSession::NotStartedState:
        case IDebugSession::StoppingState:
            kDebug() << "new state: stopped";
            stateChanged("stopped");
            clearExecutionPoint();
            m_restartDebugger->setEnabled(false);
            break;
        case IDebugSession::StartingState:
        case IDebugSession::PausedState:
            kDebug() << "new state: paused";
            stateChanged("paused");
            m_restartDebugger->setEnabled(session->restartAvaliable());
            break;
        case IDebugSession::ActiveState:
            kDebug() << "new state: active";
            stateChanged("active");
            m_restartDebugger->setEnabled(false);
            break;
    }

    if (state == IDebugSession::StoppedState || state == IDebugSession::NotStartedState
       || state == IDebugSession::StoppingState || state == IDebugSession::ActiveState) {
        clearExecutionPoint();
    }

    if (state == IDebugSession::StoppedState || state == IDebugSession::NotStartedState || state == IDebugSession::StoppingState) {
        m_startDebugger->setText( i18n("&Start") );
        m_startDebugger->setToolTip( i18n("Runs the program in the debugger") );
        m_startDebugger->setWhatsThis( i18n("<b>Start in debugger</b><p>"
                                                "Starts the debugger with the project's main "
                                                "executable. You may set some breakpoints "
                                                "before this, or you can interrupt the program "
                                                "while it is running, in order to get information "
                                                "about variables, frame stack, and so on.</p>") );
        m_startDebugger->disconnect(this);
        connect(m_startDebugger, SIGNAL(triggered(bool)), this, SLOT(startDebugger()));
    } else {
        m_startDebugger->setText( i18n("&Continue") );
        m_startDebugger->setToolTip( i18n("Continues the application execution") );
        m_startDebugger->setWhatsThis( i18n("<b>Continue application execution</b><p>"
            "Continues the execution of your application in the "
            "debugger. This only takes effect when the application "
            "has been halted by the debugger (i.e. a breakpoint has "
            "been activated or the interrupt was pressed).</p>") );
        m_startDebugger->disconnect(this);
        connect(m_startDebugger, SIGNAL(triggered(bool)), this, SLOT(run()));
    }
}

ContextMenuExtension DebugController::contextMenuExtension( Context* context )
{
    ContextMenuExtension menuExt;

    if( context->type() != Context::EditorContext )
        return menuExt;

    KDevelop::EditorContext *econtext = dynamic_cast<KDevelop::EditorContext*>(context);
    if (!econtext)
        return menuExt;

    if (m_currentSession && m_currentSession->isRunning()) {
        menuExt.addAction( KDevelop::ContextMenuExtension::DebugGroup, m_runToCursor);
    }

    if (econtext->url().isLocalFile()) {
        menuExt.addAction( KDevelop::ContextMenuExtension::DebugGroup, m_toggleBreakpoint);
    }
    return menuExt;
}


void DebugController::startDebugger() {
    if (m_currentSession) {
        m_currentSession->startDebugger();
    }
}
void DebugController::restartDebugger() {
    if (m_currentSession) {
        m_currentSession->restartDebugger();
    }
}
void DebugController::stopDebugger() {
    if (m_currentSession) {
        m_currentSession->stopDebugger();
    }
}
void DebugController::interruptDebugger() {
    if (m_currentSession) {
        m_currentSession->interruptDebugger();
    }
}

void DebugController::run() {
    if (m_currentSession) {
        m_currentSession->run();
    }
}

void DebugController::runToCursor() {
    if (m_currentSession) {
        m_currentSession->runToCursor();
    }
}
void DebugController::jumpToCursor() {
    if (m_currentSession) {
        m_currentSession->jumpToCursor();
    }
}
void DebugController::stepOver() {
    if (m_currentSession) {
        m_currentSession->stepOver();
    }
}
void DebugController::stepIntoInstruction() {
    if (m_currentSession) {
        m_currentSession->stepIntoInstruction();
    }
}
void DebugController::stepInto() {
    if (m_currentSession) {
        m_currentSession->stepInto();
    }
}
void DebugController::stepOverInstruction() {
    if (m_currentSession) {
        m_currentSession->stepOverInstruction();
    }
}
void DebugController::stepOut() {
    if (m_currentSession) {
        m_currentSession->stepOut();
    }
}
void DebugController::toggleBreakpoint() {
    if (m_currentSession) {
        m_currentSession->toggleBreakpoint();
    }
}

const QPixmap* DebugController::inactiveBreakpointPixmap()
{
  static QPixmap pixmap=KIcon("script-error").pixmap(QSize(22,22), QIcon::Normal, QIcon::Off);
  return &pixmap;
}

const QPixmap* DebugController::activeBreakpointPixmap()
{
  static QPixmap pixmap=KIcon("script-error").pixmap(QSize(22,22), QIcon::Active, QIcon::Off);
  return &pixmap;
}

const QPixmap* DebugController::reachedBreakpointPixmap()
{
  static QPixmap pixmap=KIcon("script-error").pixmap(QSize(22,22), QIcon::Selected, QIcon::Off);
  return &pixmap;
}

const QPixmap* DebugController::disabledBreakpointPixmap()
{
  static QPixmap pixmap=KIcon("script-error").pixmap(QSize(22,22), QIcon::Disabled, QIcon::Off);
  return &pixmap;
}

const QPixmap* DebugController::executionPointPixmap()
{
  static QPixmap pixmap=KIcon("go-next").pixmap(QSize(22,22), QIcon::Normal, QIcon::Off);
  return &pixmap;
}

}