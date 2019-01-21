// MainWindow.h

#ifndef __MAINWINDOW_H__
#define __MAINWINDOW_H__

#include "VideoWidget.h"
#include "ArffWidgetCoord.h"
#include "ArffWidgetSpeed.h"
#include "PaintGaze.h"
#include "../arffHelper/Arff.h"

#include <QMainWindow>
#include <memory>

enum class GazeType
{
    EYE_PLUS_HEAD,
    FOV,
    HEAD
};

struct SetupValues
{
    QString arffFile;
    QString saveFile;
    QString videoFile;
    QString primaryLabel;
    QString primaryLabelValues;
    QString secondaryLabel;
    QString secondaryLabelValues;
    GazeType gazeType;
};

class MainWindow : public QMainWindow
{

    Q_OBJECT

public:
    MainWindow();

    MainWindow(SetupValues setup);
    // Constructor for command line use

    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;

private slots:
    void SaveArff();
    ///< Saves Arff to m_saveFilename. Gets value if it is empty.

    void SaveAsArff();
    ///< proimpts user to provide a name

    void OpenFiles();
    ///< opens video and arff file

    void Undo();
    ///< undoes last change

    void Redo();
    ///< redoes change

    void HandleTime(int curTime_us);

    void HandleWindowDur(int dur_us);

    void HandleUpdate();

signals:
    void SendTime(int curTime_us, QObject *pSender);

    void SendWindowDur(int dur_us, QObject *pSender);

    void SendUpdate();

    void SendSelectedEyeMovement(int eyeMovement);

    void SendToggleView();

private:
    void InitializeMainWidget();
    ///< initializes object and connects signals

    void InitializeVariables();
    ///< initializes all needed variables

    void InitializeMenu();
    ///< initializes the menu and te actions

    int InitializeAtt(QString name, QString values);

    void ConnectTimeSignals(const QObject *pObject);

    void ConnectWinDurSignals(const QObject *pObject);

    void ConnectUpdateSignals(const QObject *pObject);

    void ConnectEyeMovementSignals(const QObject *pObject);

    void ConnectToggleViewSignals(const QObject *pObject);

    void SetData(int attIndex);

    void keyPressEvent(QKeyEvent *event);
    ///< overwrites key press events when it has focus

    bool eventFilter(QObject *watched, QEvent *event);
    ///< overwrites the event filter in order to provide keyboard short cuts

    bool ProcessKeyPress(QKeyEvent *event);
    ///< processes key events. Returns true if the event was handled. False otherwise

    bool UseAttributeDialog(QString attName);
    ///< shows a dialog if attribute already exists and returns true if the user selected 
    ///< to change the existing one or the attribute wans't present




    SetupValues     m_setup;
    QWidget         *m_pMainWidget;
    QDir            path; // path to last used directory

    VideoWidget     *m_pVideoWidget;
    ArffWidgetCoordX     *m_pArffWidgetCoordX;
    ArffWidgetCoordY     *m_pArffWidgetCoordY;
    ArffWidgetSpeed      *m_pArffWidgetSpeed;
    ArffWidgetBase       *m_pArffSecondWidgetSpeed;
    ArffWidgetBase       *m_pArffSecondWidgetY;
    PaintGaze       *m_pPaintGaze;

    shared_ptr<Arff>         m_pArff;
    shared_ptr<Arff>         m_pFovArff; // arff for FOV conversion from equirectangular

    // menu windows and actions
    QMenu           *m_fileMenu;
    QMenu           *m_editMenu;

    QAction         *m_openAction;
    QAction         *m_saveAction;
    QAction         *m_saveAsAction;
    QAction         *m_undoAction;
    QAction         *m_redoAction;
};

#endif /*__MAINWINDOW_H__*/
