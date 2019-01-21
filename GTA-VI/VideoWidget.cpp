// VideoWidget.cpp

#include "VideoWidget.h"

#include <iostream>
#include <QThread>
using namespace std;

// PUBLIC SLOTS:

void VideoWidget::HandleTime(int time, QObject *pSender)
{
    if (pSender == this)
        return;
    else
        Pause();

    // block signals
    blockSignals(true);

    // Pause on user input
    //Pause();
    SetCurrentTime(time);

    // unblock singals
    blockSignals(false);
}

void VideoWidget::HandleToggleView()
{
    if (!m_pEqToFov)
        return;

    if (m_toggled)
    {
        m_mediaPlayer.SetConverter(nullptr);
        m_toggled = false;
    }
    else
    {
        m_mediaPlayer.SetConverter(m_pEqToFov);
        m_toggled = true;
    }

    update();
}

// PRIVATE SLOTS:

void VideoWidget::HandleTimeChanged(qint64 time)
{
    const QSignalBlocker blocker(m_mediaPlayer);
    // convert time from ms to us
    if (!m_isPaused)
    {
        m_previousTime = time*1000;
        // emit signal
        emit SendTime(m_previousTime);
        update();
    }
}

// PUBLIC:

VideoWidget::VideoWidget(QWidget *parent) : QWidget(parent), m_pPaintGaze(0), m_pEqToFov(0), m_isPaused(true), m_toggled(false)
{
    SetCurrentTime(0);

    // set the following attributes to get some speed up
    setAttribute(Qt::WA_NoSystemBackground, true);

    // connect objects signal with private slot
    QObject::connect(&m_mediaPlayer, SIGNAL(positionChanged(qint64)), this, SLOT(HandleTimeChanged(qint64)));
}

VideoWidget::~VideoWidget()
{
    if (!m_pEqToFov)
        delete m_pEqToFov;
}

QSize VideoWidget::minimumSizeHint() const
{
    return QSize(160, 90);
}

void VideoWidget::SetCurrentTime(int currentTime)
{
    if(!m_mutex.tryLock())
        return;
    //const QSignalBlocker blocker(m_mediaPlayer);
    m_previousTime = currentTime;
    int msTime = currentTime/1000; // convert to ms
    m_mediaPlayer.setPosition(msTime);

    update();
    m_mutex.unlock();
}

bool VideoWidget::SetMedia(QString pathToFile)
{
    return m_mediaPlayer.setMedia(QUrl::fromLocalFile(pathToFile));
}

void VideoWidget::SetGazePaint(PaintGaze *pPaintGaze)
{
    m_pPaintGaze = pPaintGaze;
}

void VideoWidget::Play()
{
    if (m_mediaPlayer.isVideoAvailable())
    {
        m_isPaused = false;
        //m_mediaPlayer.blockSignals(false);
        m_mediaPlayer.play();
        repaint();
    }
}

void VideoWidget::Pause()
{
    if (m_mediaPlayer.isVideoAvailable())
    {
        m_isPaused = true;
        //m_mediaPlayer.blockSignals(true);
        m_mediaPlayer.pause();
    }
}

void VideoWidget::TogglePlayer()
{
    if (m_mediaPlayer.state()==MediaPlayer::PausedState || m_mediaPlayer.state()==MediaPlayer::StoppedState)
        Play();
    else
        Pause();
}

qint64 VideoWidget::Duration()
{
    qint64 duration = m_mediaPlayer.duration();

    if (duration < 0)
        return -1;
    else
        return 1000*duration;
}

void VideoWidget::ConvertToFov(Arff *pArff)
{
    m_pEqToFov = new EquirectangularToFovVideo(pArff);
}


// PROTECTED:

void VideoWidget::paintEvent(QPaintEvent *) /*override*/
{
    QPainter painter(this);
    // paint video
    QRect geom = geometry();
    m_mediaPlayer.Paint(&painter, geom.size());
    // paint gaze
    if (m_pPaintGaze != NULL)
    {
        m_pPaintGaze->Paint(&painter, geom.size());
    }
}

void VideoWidget::resizeEvent(QResizeEvent *event) /*override*/
{
    QWidget::resizeEvent(event);
}
