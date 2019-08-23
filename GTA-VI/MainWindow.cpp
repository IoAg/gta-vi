// MainWindow.cpp

#include "MainWindow.h"
#include "EquirectangularToFovGaze.h"
#include "EquirectangularToHead.h"
#include "../arffHelper/ArffOps.h"

#include <iostream>
#include <cassert>

using namespace std;

// PUBLIC:

MainWindow::MainWindow() : m_pMainWidget(0), m_pVideoWidget(0), m_pArffWidgetCoordX(0), m_pArffWidgetCoordY(0), m_pArffWidgetSpeed(0), m_pPaintGaze(0), m_pArff(0), m_pFovArff(0), m_openAction(0), m_saveAction(0), m_undoAction(0), m_redoAction(0)
{
    InitializeVariables();
    InitializeMainWidget();
    InitializeMenu();
    SetData(0);
}

MainWindow::MainWindow(SetupValues setup) : m_setup(setup), m_pMainWidget(0), m_pVideoWidget(0), m_pArffWidgetCoordX(0), m_pArffWidgetCoordY(0), m_pArffWidgetSpeed(0), m_pPaintGaze(0), m_pArff(0), m_pFovArff(0), m_openAction(0), m_saveAction(0), m_undoAction(0), m_redoAction(0)
{
    InitializeVariables();
    InitializeMainWidget();
    InitializeMenu();

    // assign values
    if (!m_setup.arffFile.isEmpty())
        m_setup.arffFile = QFileInfo(m_setup.arffFile).absoluteFilePath();
    else
    {
       cerr << "ERROR: your should provide an ARFF file with the --af option" << endl;
       exit(-1);
    }
    if (!m_setup.saveFile.isEmpty())
        m_setup.saveFile = QFileInfo(m_setup.saveFile).absoluteFilePath();
    if (!m_setup.videoFile.isEmpty())
        m_setup.videoFile = QFileInfo(m_setup.videoFile).absoluteFilePath();

    // Load Files
    if (!m_setup.videoFile.isEmpty() &&!m_pVideoWidget->SetMedia(m_setup.videoFile))
        exit(-1);

    if (!m_pArff->Load(m_setup.arffFile.toStdString().c_str()))
       exit(-1); 

    int primAttPosition = InitializeAtt(m_setup.primaryLabel, m_setup.primaryLabelValues);
    SetData(primAttPosition);
    if (!m_setup.secondaryLabel.isEmpty())
    {
        int secAttPosition = InitializeAtt(m_setup.secondaryLabel, m_setup.secondaryLabelValues);
        m_pArffSecondWidgetSpeed->SetData(*m_pArff, secAttPosition);
        m_pArffSecondWidgetSpeed->SetIntervalAtt(primAttPosition);
        m_pArffSecondWidgetY->SetData(*m_pArff, secAttPosition);
        m_pArffSecondWidgetY->SetIntervalAtt(primAttPosition);
    }

    if (m_setup.gazeType == GazeType::FOV)
    {
        EquirectangularToFovGaze eqToFov(m_pArff.get());
        m_pFovArff = eqToFov.Convert();
        m_pPaintGaze->SetFovData(*m_pFovArff);
        m_pVideoWidget->ConvertToFov(m_pArff.get());
        m_pArffWidgetCoordX->SetFovData(*m_pFovArff, m_pFovArff->WidthPx());
        m_pArffWidgetCoordY->SetFovData(*m_pFovArff, m_pFovArff->HeightPx());
        m_pArffWidgetSpeed->DisplayFov();
        emit SendToggleView();
    }
    else if (m_setup.gazeType == GazeType::HEAD)
    {
        EquirectangularToHead converter(m_pArff.get());
        m_pFovArff = converter.Convert();
        m_pPaintGaze->SetFovData(*m_pFovArff);
        m_pArffWidgetCoordX->SetFovData(*m_pFovArff, m_pFovArff->WidthPx());
        m_pArffWidgetCoordY->SetFovData(*m_pFovArff, m_pFovArff->HeightPx());
        m_pArffWidgetSpeed->DisplayHead();
        emit SendToggleView();
    }
}

MainWindow::~MainWindow()
{
    delete m_pMainWidget;
    delete m_pVideoWidget;
    delete m_pArffWidgetCoordX;
    delete m_pArffWidgetCoordY;
    delete m_pArffWidgetSpeed;
    delete m_pArffSecondWidgetSpeed;
    delete m_pArffSecondWidgetY;
    delete m_pPaintGaze;
    delete m_openAction;
    delete m_saveAction;
    delete m_undoAction;
    delete m_redoAction;
}

// PROTECTED:
void MainWindow::closeEvent(QCloseEvent *event)
{
    if (m_pArff->IsDataChanged())
    {
        // display message box and ask for action
        QMessageBox msgBox;
        msgBox.setText("Gaze samples have been modified.");
        msgBox.setInformativeText("Do you want to save your changes?");
        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Cancel);

        int ret = msgBox.exec();
        switch (ret) {
            case QMessageBox::Save:
                SaveArff();
                event->accept();
                break;
            case QMessageBox::Discard:
                event->accept();
                break;
            case QMessageBox::Cancel:
                event->ignore();
                break;
        }
    }
    else
    {
        event->accept();
    }
}

// PRIVATE:

void MainWindow::InitializeMainWidget()
{
    // load static file for beginning
    m_pArff = make_shared<Arff>();

    m_pVideoWidget = new VideoWidget;
    m_pArffWidgetCoordX = new ArffWidgetCoordX;
    m_pArffWidgetCoordY = new ArffWidgetCoordY;
    m_pArffWidgetSpeed = new ArffWidgetSpeed;

    m_pPaintGaze = new PaintGaze;

    // connect time signals-slots
    ConnectTimeSignals(m_pArffWidgetCoordX);
    ConnectTimeSignals(m_pArffWidgetCoordY);
    ConnectTimeSignals(m_pArffWidgetSpeed);
    ConnectTimeSignals(m_pVideoWidget);
    QObject::connect(this, SIGNAL(SendTime(int, QObject*)), m_pPaintGaze, SLOT(HandleTime(int, QObject*)));

    ConnectWinDurSignals(m_pArffWidgetCoordX);
    ConnectWinDurSignals(m_pArffWidgetCoordY);
    ConnectWinDurSignals(m_pArffWidgetSpeed);
    
    ConnectUpdateSignals(m_pArffWidgetCoordX);
    ConnectUpdateSignals(m_pArffWidgetCoordY);
    ConnectUpdateSignals(m_pArffWidgetSpeed);
    
    ConnectEyeMovementSignals(m_pArffWidgetCoordX);
    ConnectEyeMovementSignals(m_pArffWidgetCoordY);
    ConnectEyeMovementSignals(m_pArffWidgetSpeed);

    ConnectToggleViewSignals(m_pArffWidgetCoordX);
    ConnectToggleViewSignals(m_pArffWidgetCoordY);
    ConnectToggleViewSignals(m_pArffWidgetSpeed);
    ConnectToggleViewSignals(m_pVideoWidget);
    ConnectToggleViewSignals(m_pPaintGaze);
    
    QGridLayout *layout = new QGridLayout;
    // add widgets to layout
    layout->addWidget(m_pVideoWidget,0,0);
    layout->addWidget(m_pArffWidgetCoordX,0,1);
    layout->addWidget(m_pArffWidgetSpeed,1,0);
    layout->addWidget(m_pArffWidgetCoordY,1,1);
    layout->setColumnStretch(0,1);
    layout->setColumnStretch(1,1);
    layout->setRowStretch(0,4);
    layout->setRowStretch(1,4);

    // install event filter to all widgets
    m_pVideoWidget->installEventFilter(this);
    m_pArffWidgetCoordX->installEventFilter(this);
    m_pArffWidgetCoordY->installEventFilter(this);
    m_pArffWidgetSpeed->installEventFilter(this);

    // add secondary label widgets
    if (!m_setup.secondaryLabel.isEmpty())
    {
        m_pArffSecondWidgetSpeed = new ArffWidgetBase;
        m_pArffSecondWidgetY = new ArffWidgetBase;

        ConnectTimeSignals(m_pArffSecondWidgetSpeed);
        ConnectTimeSignals(m_pArffSecondWidgetY);
        ConnectWinDurSignals(m_pArffSecondWidgetSpeed);
        ConnectWinDurSignals(m_pArffSecondWidgetY);
        ConnectUpdateSignals(m_pArffSecondWidgetSpeed);
        ConnectUpdateSignals(m_pArffSecondWidgetY);
        ConnectEyeMovementSignals(m_pArffSecondWidgetSpeed);
        ConnectEyeMovementSignals(m_pArffSecondWidgetY);

        layout->addWidget(m_pArffSecondWidgetSpeed,2,0);
        layout->addWidget(m_pArffSecondWidgetY,2,1);
        layout->setRowStretch(2, 1);

        m_pArffSecondWidgetSpeed->installEventFilter(this);
        m_pArffSecondWidgetY->installEventFilter(this);
    }

    // create main widget
    m_pMainWidget = new QWidget;
    m_pMainWidget->setLayout(layout);
    setCentralWidget(m_pMainWidget);
}

void MainWindow::ConnectTimeSignals(const QObject *pObject)
{
    QObject::connect(pObject, SIGNAL(SendTime(int)), this, SLOT(HandleTime(int)));
    QObject::connect(this, SIGNAL(SendTime(int, QObject*)), pObject, SLOT(HandleTime(int, QObject*)));
}

void MainWindow::ConnectWinDurSignals(const QObject *pObject)
{
    QObject::connect(pObject, SIGNAL(SendWindowDur(int)), this, SLOT(HandleWindowDur(int)));
    QObject::connect(this, SIGNAL(SendWindowDur(int, QObject*)), pObject, SLOT(HandleWindowDur(int, QObject*)));
}

void MainWindow::ConnectUpdateSignals(const QObject *pObject)
{
    QObject::connect(pObject, SIGNAL(SendUpdate()), this, SLOT(HandleUpdate()));
    QObject::connect(this, SIGNAL(SendUpdate()), pObject, SLOT(HandleUpdate()));
}

void MainWindow::ConnectEyeMovementSignals(const QObject *pObject)
{
    QObject::connect(this, SIGNAL(SendSelectedEyeMovement(int)), pObject, SLOT(HandleSelectedEyeMovement(int)));
}

void MainWindow::ConnectToggleViewSignals(const QObject *pObject)
{
    QObject::connect(this, SIGNAL(SendToggleView()), pObject, SLOT(HandleToggleView()));
}

void MainWindow::InitializeVariables()
{
    path = QDir::current();
}

void MainWindow::InitializeMenu()
{
    // create new statusbar. Otherwise it is not visible
    QStatusBar* status = new QStatusBar(this);
    setStatusBar(status);

    // making menu non native makes it visible in ubuntu
    menuBar()->setNativeMenuBar(false);

    // create actions
    m_openAction = new QAction(tr("&Open"), this);
    m_openAction->setShortcuts(QKeySequence::Open);
    m_openAction->setStatusTip(tr("Open video and Arff files"));
    connect(m_openAction, SIGNAL(triggered()), this, SLOT(OpenFiles()));

    m_saveAction = new QAction(tr("&Save"), this);
    m_saveAction->setShortcuts(QKeySequence::Save);
    m_saveAction->setStatusTip(tr("Save Arff to file"));
    connect(m_saveAction, &QAction::triggered, this, &MainWindow::SaveArff);

    m_saveAsAction = new QAction(tr("Save&As"), this);
    m_saveAsAction->setShortcuts(QKeySequence::SaveAs);
    m_saveAsAction->setStatusTip(tr("Save Arff to new file"));
    connect(m_saveAsAction, &QAction::triggered, this, &MainWindow::SaveAsArff);

    m_undoAction = new QAction(tr("&Undo"), this);
    m_undoAction->setShortcuts(QKeySequence::Undo);
    m_undoAction->setStatusTip(tr("Undo last change"));
    connect(m_undoAction, &QAction::triggered, this, &MainWindow::Undo);

    m_redoAction = new QAction(tr("&Redo"), this);
    m_redoAction->setShortcuts(QKeySequence::Redo);
    m_redoAction->setStatusTip(tr("Redo last change"));
    connect(m_redoAction, &QAction::triggered, this, &MainWindow::Redo);

    m_fileMenu = menuBar()->addMenu(tr("&File"));
    m_fileMenu->addAction(m_openAction);
    m_fileMenu->addAction(m_saveAction);
    m_fileMenu->addAction(m_saveAsAction);

    m_editMenu = menuBar()->addMenu(tr("&Edit"));
    m_editMenu->addAction(m_undoAction);
    m_editMenu->addAction(m_redoAction);
}

int MainWindow::InitializeAtt(QString name, QString values)
{
    // check for hand labelling attribute use and prompt user for action
    if (!UseAttributeDialog(name))
        exit(-1);

    int attPosition;
    bool res = m_pArff->GetAttIndex(name.toStdString().c_str(), attPosition);
    if (!res)
    {
        if (values.isEmpty())
            // Get majority vote of attributes
            ArffOps::MajorityVote(m_pArff.get(), name.toStdString().c_str()); // add mode att the last column
        else
            m_pArff->AddColumn(name.toStdString(), values.toStdString());

        int rows, columns;
        m_pArff->Size(rows, columns);
        attPosition = columns-1;
    }
    else
    {
        cout << "WARNING: attribute '" << name.toStdString() << "' already in use." << endl;
    }

    return attPosition;
}

void MainWindow::SetData(int attIndex)
{
    // Set time to the beginning of both video and gaze
    // Set max limits on each after reloading
    m_pArffWidgetCoordX->SetData(*m_pArff, attIndex, m_pArff->WidthPx());
    m_pArffWidgetCoordY->SetData(*m_pArff, attIndex, m_pArff->HeightPx());
    m_pArffWidgetSpeed->SetData(*m_pArff, attIndex);
    m_pVideoWidget->SetGazePaint(m_pPaintGaze);
    m_pVideoWidget->SetCurrentTime(0);
    m_pPaintGaze->SetData(*m_pArff);
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (!ProcessKeyPress(event))
        QMainWindow::keyPressEvent(event);

}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (ProcessKeyPress(keyEvent))
            return true;
    }

    return QObject::eventFilter(watched, event);
}


bool MainWindow::ProcessKeyPress(QKeyEvent *event)
{
    statusBar()->showMessage(tr(""));

	int key = event->key();
    int key_0 = 0x30; // Qt::Key_0
    int key_9 = 0x39; // Qt::Key_9
    int mask = 0x0F; 
    if (key >= key_0 && key <= key_9)
    {
        int eyeMovement = key & mask;
        emit SendSelectedEyeMovement(eyeMovement);
        return true;
    }
    else if (key == Qt::Key_Space)
    {
        m_pVideoWidget->TogglePlayer();
        return true;
    }
    else if (key == Qt::Key_T)
    {
        emit SendToggleView();
        return true;
    }
    else
        return false;
}

bool MainWindow::UseAttributeDialog(QString attName)
{
    int tmpPos;
    if (m_pArff->GetAttIndex(attName.toStdString().c_str(), tmpPos))
    {
        QMessageBox::StandardButton reply;
        string message = "Do you still want to use attribute '" + attName.toStdString() + "'?";
        reply = QMessageBox::question(this, "Attribute already exists", message.c_str(), QMessageBox::Yes|QMessageBox::No);

        if (reply == QMessageBox::Yes){
            return true;
        }
        else{
            return false;
        }
    }
    return true;
}

// PRIVATE SLOTS:

void MainWindow::SaveArff()
{
    if (m_setup.saveFile.isEmpty())
    {
        m_setup.saveFile =  QFileDialog::getSaveFileName(this, tr("Save File"), 
                path.path(), tr("Arff files (*.arff *.txt)"));
        // update path
        path = QFileInfo(m_setup.saveFile).dir();
    }

    // handle canceled window case
    if (!m_setup.saveFile.isEmpty())
    {
        m_pArff->Save(m_setup.saveFile.toStdString().c_str());
        statusBar()->showMessage(tr("Saved"));
    }
    else
    {
        statusBar()->showMessage(tr("Could not save Arff file"));
    }

}

void MainWindow::SaveAsArff()
{
    m_setup.saveFile.clear();

    SaveArff();
}

void MainWindow::OpenFiles()
{
    // first check for unsaved changes
    if (m_pArff->IsDataChanged())
    {
        // display message box and ask for action
        QMessageBox msgBox;
        msgBox.setText("Gaze samples have been modified.");
        msgBox.setInformativeText("Do you want to save your changes?");
        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Cancel);

        int ret = msgBox.exec();
        switch (ret) 
        {
            case QMessageBox::Save:
                SaveArff();
                break;
            case QMessageBox::Discard:
                // do nothing
                break;
            case QMessageBox::Cancel:
                return;
                break;
        }
    }


    m_setup.videoFile = QFileDialog::getOpenFileName(this, tr("Open Video File"),
            path.path(), tr("Video files (*.m2t *.avi *.mp4 *.wmv)"));
    // return if file is empty
    if (m_setup.videoFile.isEmpty())
        return;
    // update path
    path = QFileInfo(m_setup.videoFile).dir();
    
    m_setup.arffFile = QFileDialog::getOpenFileName(this, tr("Open Arff File"),
            path.path(), tr("Arff files (*.arff *.txt)"));
    if (m_setup.arffFile.isEmpty())
        return;
    // update path
    path = QFileInfo(m_setup.arffFile).dir();

    m_pVideoWidget->SetMedia(m_setup.videoFile);
    m_pArff->Load(m_setup.arffFile.toStdString().c_str());

    bool accepted=false;
    do
    {
        // check if name for hand labelling attribute exists
        if (m_setup.primaryLabel.isEmpty())
        {
            m_setup.primaryLabel = QInputDialog::getText(this, tr("Hande labeller name"), tr("Attribute name:"), QLineEdit::Normal, QDir::home().dirName(), &accepted);

            // remove whitespace from input value
            m_setup.primaryLabel = m_setup.primaryLabel.simplified();
            m_setup.primaryLabel.replace(" ", "");
            
            // if canceled or empty string provided return
            if (!accepted || m_setup.primaryLabel.isEmpty())
                return;
        }

        // check for hand labelling attribute use
        accepted = UseAttributeDialog(m_setup.primaryLabel);
        if (!accepted)
            m_setup.primaryLabel.clear();
    }
    while(!accepted);


    int attPosition;
    bool res = m_pArff->GetAttIndex(m_setup.primaryLabel.toStdString().c_str(), attPosition);
    if (!res){
        // Get majority vote for of attributes
        ArffOps::MajorityVote(m_pArff.get(), m_setup.primaryLabel.toStdString().c_str()); // add mode att the last column
        int rows, columns;
        m_pArff->Size(rows, columns);
        attPosition = columns-1;
    }

    SetData(attPosition);
}

void MainWindow::Undo()
{
    m_pArff->UndoLastChange();
    emit SendUpdate();

    statusBar()->showMessage(tr("Last change reverted"));
}

void MainWindow::Redo()
{
    m_pArff->RedoLastChange();
    emit SendUpdate();

    statusBar()->showMessage(tr("Last change remade"));
}

void MainWindow::HandleTime(int curTime_us)
{
    QObject *pSender = sender();
    
    emit SendTime(curTime_us, pSender);
}

void MainWindow::HandleWindowDur(int dur_us)
{
    QObject *pSender = sender();

    emit SendWindowDur(dur_us, pSender);
}

void MainWindow::HandleUpdate()
{
    emit SendUpdate();
}
