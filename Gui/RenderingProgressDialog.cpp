//  Natron
//
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
/*
 * Created by Alexandre GAUTHIER-FOICHAT on 6/1/2012.
 * contact: immarespond at gmail dot com
 *
 */

// from <https://docs.python.org/3/c-api/intro.html#include-files>:
// "Since Python may define some pre-processor definitions which affect the standard headers on some systems, you must include Python.h before any standard headers are included."
#include <Python.h>

#include "RenderingProgressDialog.h"

#include <cmath>

CLANG_DIAG_OFF(deprecated)
CLANG_DIAG_OFF(uninitialized)
#include <QVBoxLayout>
#include <QProgressBar>
#include <QFrame>
#include <QTextBrowser>
#include <QApplication>
#include <QThread>
#include <QKeyEvent>
#include <QString>
#include <QTextStream>
CLANG_DIAG_ON(deprecated)
CLANG_DIAG_ON(uninitialized)

#include "Engine/ProcessHandler.h"
#include "Engine/Timer.h"

#include "Gui/Button.h"
#include "Gui/GuiApplicationManager.h"
#include "Gui/Gui.h"
#include "Gui/Label.h"

struct RenderingProgressDialogPrivate
{
    Gui* _gui;
    QVBoxLayout* _mainLayout;
    Natron::Label* _totalProgressLabel;
    Natron::Label* _totalProgressInfo;
    QProgressBar* _totalProgressBar;
    Natron::Label* _estimatedWaitTimeLabel;
    Natron::Label* _estimatedWaitTimeInfo;
    Button* _cancelButton;
    QString _sequenceName;
    int _firstFrame;
    int _lastFrame;
    boost::shared_ptr<ProcessHandler> _process;

    int _nFramesRendered;

    RenderingProgressDialogPrivate(Gui* gui,
                                   const QString & sequenceName,
                                   int firstFrame,
                                   int lastFrame,
                                   const boost::shared_ptr<ProcessHandler> & proc)
    : _gui(gui)
    , _mainLayout(0)
    , _totalProgressLabel(0)
    , _totalProgressInfo(0)
    , _totalProgressBar(0)
    , _estimatedWaitTimeLabel(0)
    , _estimatedWaitTimeInfo(0)
    , _cancelButton(0)
    , _sequenceName(sequenceName)
    , _firstFrame(firstFrame)
    , _lastFrame(lastFrame)
    , _process(proc)
    , _nFramesRendered(0)
    {
    }
};




void
RenderingProgressDialog::onFrameRenderedWithTimer(int frame, double /*timeElapsedForFrame*/, double remainingTime)
{
    assert(QThread::currentThread() == qApp->thread());
    
    ++_imp->_nFramesRendered;
    
    double percent = _imp->_nFramesRendered / (double)(_imp->_lastFrame - _imp->_firstFrame + 1);
    double progress = percent * 100;
    
    _imp->_totalProgressBar->setValue(progress);
    
    QString infoStr;
    QTextStream ts(&infoStr);
    ts << "Frame " << frame << " (" << QString::number(progress,'f',1) << "%)";
    _imp->_totalProgressInfo->setText(infoStr);
    
    QString timeStr = Timer::printAsTime(remainingTime, true);
    _imp->_estimatedWaitTimeInfo->setText(timeStr);
}

void
RenderingProgressDialog::onFrameRendered(int frame)
{


    assert(QThread::currentThread() == qApp->thread());

    ++_imp->_nFramesRendered;

    double percent = _imp->_nFramesRendered / (double)(_imp->_lastFrame - _imp->_firstFrame + 1);
    double progress = percent * 100;

    _imp->_totalProgressBar->setValue(progress);
    
    QString infoStr;
    QTextStream ts(&infoStr);
    ts << "Frame " << frame << " (" << QString::number(progress,'f',1) << "%)";
    _imp->_totalProgressInfo->setText(infoStr);
    _imp->_estimatedWaitTimeInfo->setText("...");
}


void
RenderingProgressDialog::onProcessCanceled()
{
	close();
}

void
RenderingProgressDialog::onProcessFinished(int retCode)
{
    if ( isVisible() ) {
        hide();

        bool showLog = false;
        if (retCode == 0) {
            if (_imp->_process) {
                Natron::StandardButtonEnum reply = Natron::questionDialog( tr("Render").toStdString(),tr("The render ended successfully.\n"
                                                                                                     "Would you like to see the log ?").toStdString(), false );
                if (reply == Natron::eStandardButtonYes) {
                    showLog = true;
                }
            } else {
                Natron::informationDialog( tr("Render").toStdString(), tr("The render ended successfully.").toStdString() );
            }
        } else if (retCode == 1) {
            if (_imp->_process) {
                Natron::StandardButtonEnum reply = Natron::questionDialog( tr("Render").toStdString(),
                                                                       tr("The render ended with a return code of 1, a problem occured.\n"
                                                                          "Would you like to see the log ?").toStdString(), false );
                if (reply == Natron::eStandardButtonYes) {
                    showLog = true;
                }
            } else {
                Natron::errorDialog( tr("Render").toStdString(),tr("The render ended with a return code of 1, a problem occured.").toStdString() );
            }
        } else {
            if (_imp->_process) {
                Natron::StandardButtonEnum reply = Natron::questionDialog( tr("Render").toStdString(),tr("The render crashed.\n"
                                                                                                         "Would you like to see the log ?").toStdString() , false);
                if (reply == Natron::eStandardButtonYes) {
                    showLog = true;
                }
            } else {
                Natron::errorDialog( tr("Render").toStdString(),tr("The render crashed.").toStdString() );
            }
        }
        if (showLog) {
            assert(_imp->_process);
            LogWindow log(_imp->_process->getProcessLog(),this);
            ignore_result(log.exec());
        }
    }
    accept();
}

void
RenderingProgressDialog::onVideoEngineStopped(int retCode)
{
    if (retCode == 1) {
        onProcessCanceled();
    } else {
        onProcessFinished(0);
    }
}

void
RenderingProgressDialog::keyPressEvent(QKeyEvent* e)
{
    if (e->key() == Qt::Key_Escape) {
        onCancelButtonClicked();
    } else {
        QDialog::keyPressEvent(e);
    }
}

void
RenderingProgressDialog::closeEvent(QCloseEvent* /*e*/)
{
    QDialog::DialogCode ret = (QDialog::DialogCode)result();
    if (ret != QDialog::Accepted) {
        Q_EMIT canceled();
        reject();
        Natron::informationDialog( tr("Render").toStdString(), tr("Render aborted.").toStdString() );
        
    }
    
}

void
RenderingProgressDialog::onCancelButtonClicked()
{
    Q_EMIT canceled();
    close();
}

RenderingProgressDialog::RenderingProgressDialog(Gui* gui,
                                                 const QString & sequenceName,
                                                 int firstFrame,
                                                 int lastFrame,
                                                 const boost::shared_ptr<ProcessHandler> & process,
                                                 QWidget* parent)
    : QDialog(parent)
      , _imp( new RenderingProgressDialogPrivate(gui,sequenceName,firstFrame,lastFrame,process) )

{
    setMinimumWidth(fontMetrics().width(_imp->_sequenceName) + 100);
    setWindowTitle(_imp->_sequenceName);
    //setWindowFlags(Qt::WindowStaysOnTopHint);
    _imp->_mainLayout = new QVBoxLayout(this);
    setLayout(_imp->_mainLayout);
    _imp->_mainLayout->setContentsMargins(5, 5, 0, 0);
    _imp->_mainLayout->setSpacing(5);

    QWidget* totalProgressContainer = new QWidget(this);
    QHBoxLayout* totalProgressLayout = new QHBoxLayout(totalProgressContainer);
    _imp->_mainLayout->addWidget(totalProgressContainer);

    
    _imp->_totalProgressLabel = new Natron::Label(tr("Total progress:"),totalProgressContainer);
    totalProgressLayout->addWidget(_imp->_totalProgressLabel);
    
    _imp->_totalProgressInfo = new Natron::Label("0%",totalProgressContainer);
    totalProgressLayout->addWidget(_imp->_totalProgressInfo);
    
    QWidget* waitTimeContainer = new QWidget(this);
    QHBoxLayout* waitTimeLayout = new QHBoxLayout(waitTimeContainer);
    _imp->_mainLayout->addWidget(waitTimeContainer);

    _imp->_estimatedWaitTimeLabel = new Natron::Label(tr("Time remaining:"),waitTimeContainer);
    waitTimeLayout->addWidget(_imp->_estimatedWaitTimeLabel);
    
    _imp->_estimatedWaitTimeInfo = new Natron::Label("...",waitTimeContainer);
    waitTimeLayout->addWidget(_imp->_estimatedWaitTimeInfo);

    _imp->_totalProgressBar = new QProgressBar(this);
    _imp->_totalProgressBar->setRange(0, 100);
    _imp->_totalProgressBar->setMinimumWidth(150);
    
    _imp->_mainLayout->addWidget(_imp->_totalProgressBar);
    
    
    _imp->_cancelButton = new Button(tr("Cancel"),this);
    _imp->_cancelButton->setMaximumWidth(50);
    _imp->_mainLayout->addWidget(_imp->_cancelButton);

    QObject::connect( _imp->_cancelButton, SIGNAL( clicked() ), this, SLOT( onCancelButtonClicked() ) );


    if (process) {
        QObject::connect( this,SIGNAL( canceled() ),process.get(),SLOT( onProcessCanceled() ) );
        QObject::connect( process.get(),SIGNAL( processCanceled() ),this,SLOT( onProcessCanceled() ) );
        QObject::connect( process.get(),SIGNAL( frameRendered(int) ),this,SLOT( onFrameRendered(int) ) );
        QObject::connect( process.get(),SIGNAL( frameRenderedWithTimer(int,double,double)),this,SLOT(onFrameRenderedWithTimer(int,double,double)));
        QObject::connect( process.get(),SIGNAL( processFinished(int) ),this,SLOT( onProcessFinished(int) ) );
        QObject::connect( process.get(),SIGNAL( deleted() ),this,SLOT( onProcessDeleted() ) );
    }
}

RenderingProgressDialog::~RenderingProgressDialog()
{
}

void
RenderingProgressDialog::onProcessDeleted()
{
    assert(_imp->_process);
    QObject::disconnect( this,SIGNAL( canceled() ),_imp->_process.get(),SLOT( onProcessCanceled() ) );
    QObject::disconnect( _imp->_process.get(),SIGNAL( processCanceled() ),this,SLOT( onProcessCanceled() ) );
    QObject::disconnect( _imp->_process.get(),SIGNAL( frameRendered(int) ),this,SLOT( onFrameRendered(int) ) );
    QObject::disconnect( _imp->_process.get(),SIGNAL( frameRenderedWithTimer(int,double,double) ),this,
                        SLOT( onFrameRenderedWithTimer(int,double,double) ) );
    QObject::disconnect( _imp->_process.get(),SIGNAL( processFinished(int) ),this,SLOT( onProcessFinished(int) ) );
    QObject::disconnect( _imp->_process.get(),SIGNAL( deleted() ),this,SLOT( onProcessDeleted() ) );
}

LogWindow::LogWindow(const QString & log,
                     QWidget* parent)
    : QDialog(parent)
{
    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    textBrowser = new QTextBrowser(this);
    textBrowser->setOpenExternalLinks(true);
    textBrowser->setText(log);

    mainLayout->addWidget(textBrowser);

    QWidget* buttonsContainer = new QWidget(this);
    QHBoxLayout* buttonsLayout = new QHBoxLayout(buttonsContainer);
    
    clearButton = new Button(tr("Clear"),buttonsContainer);
    buttonsLayout->addWidget(clearButton);
    QObject::connect(clearButton, SIGNAL(clicked()), this, SLOT(onClearButtonClicked()));
    buttonsLayout->addStretch();
    okButton = new Button(tr("Ok"),buttonsContainer);
    buttonsLayout->addWidget(okButton);
    QObject::connect( okButton, SIGNAL( clicked() ), this, SLOT( accept() ) );
    mainLayout->addWidget(buttonsContainer);
}

void
LogWindow::onClearButtonClicked()
{
    appPTR->clearOfxLog_mt_safe();
    textBrowser->clear();
}
