// MediaPlayer.cpp

#include "MediaPlayer.h"
#include "Unused.h"

#include <string>
#include <QTime>
#include <fstream>
#include <iostream>

#define FOV_IMAGE_HEIGHT 600

using namespace std;

// PRIVATE SLOTS:

void MediaPlayer::HandleTimerTick()
{
    if (m_state==StoppedState || m_state==PausedState)
        return;

    // if the timer is fire due to main thread blocking, skip the frames of the delay
    if (m_startPlayMs < 0) // player starts after pause
    {
        m_startPlayMs =m_time.elapsed();
        m_startPlayFrame = m_curFrame+1;
        LoadFrame(++m_startPlayFrame);
    }
    else // it means the player was already playing
    {
        int curTime =m_time.elapsed();
        int framesElapsed = (int)(((curTime-m_startPlayMs)*m_videoParams.GetFrameRate())/1000.0 + 0.5); // round to pcloser frame
        framesElapsed = framesElapsed<1? 1: framesElapsed;
        LoadFrame(m_startPlayFrame+framesElapsed);
    }


    qint64 newTime = (qint64)(1000.0*m_curFrame/m_videoParams.GetFrameRate());

    // notify that frame has changed
    emit positionChanged(newTime);
}

// PUBLIC:

MediaPlayer::MediaPlayer(QObject *parent) : QObject(parent), m_startPlayFrame(0), m_startPlayMs(-1), m_state(StoppedState), m_curFrame(0), m_bVideoAv(false), m_pEqToFov(NULL)
{
    m_pFpsTimer = new QTimer(this); // timer hasn't started yet
    connect(m_pFpsTimer, SIGNAL(timeout()), this, SLOT(HandleTimerTick()));
    m_time.start();
}

MediaPlayer::~MediaPlayer()
{
    delete m_pFpsTimer;
}

void MediaPlayer::setPosition(qint64 position)
{
    int frameNum = (int)((double)position*m_videoParams.GetFrameRate()/1000.0);

    LoadFrame(frameNum);
}

bool MediaPlayer::setMedia(QUrl videoFile)
{
    string sVideoFile = videoFile.toString().toStdString();
    if (!VideoExtractor::IsVideoExtracted(sVideoFile.c_str()))
    {
        if (!VideoExtractor::ExtractFrames(sVideoFile))
        {
            cerr << "Could not open video file: " << sVideoFile << endl;
            return false;
        }
    }

    string paramsFile = VideoExtractor::GetParamsname();

    bool res = VideoExtractor::LoadVideoParams(paramsFile, m_videoParams);
    UNUSED(res);

    // after knowing video is extracted, load first frame
    LoadFrame(0);
    m_bVideoAv = true;

    // and start timer with correct tick interval
    int interval = (int)(1000.0/m_videoParams.GetFrameRate());
    m_pFpsTimer->start(interval);

    return true;
}

void MediaPlayer::play()
{
    m_state = PlayingState;
}

void MediaPlayer::pause()
{
    m_state = PausedState;
    m_startPlayMs = -1;
}

MediaPlayer::State MediaPlayer::state() const
{
    return m_state;
}

qint64 MediaPlayer::duration()
{
    qint64 duration = (qint64)(m_videoParams.numOfFrames*1000.0/m_videoParams.GetFrameRate());

    return duration;
}

bool MediaPlayer::isVideoAvailable() const
{
    return m_bVideoAv;
}

void MediaPlayer::Paint(QPainter *painter, QSize size)
{
    QRect sourceRect = QRect(QPoint(0,0), m_image.size());
    double videoAspectRatio = (double)m_image.width()/(double)m_image.height();
    double areaAspectRatio = (double)size.width()/(double)size.height();
    int width, height, xDisp, yDisp;
    if (videoAspectRatio > areaAspectRatio){
        width = size.width();
        xDisp = 0;
        height = width/videoAspectRatio;
        yDisp = (size.height()-height)/2;
    }
    else{
        height = size.height();
        yDisp = 0;
        width = height*videoAspectRatio;
        xDisp = (size.width()-width)/2;
    }

    QRect targetRect = QRect(xDisp, yDisp, width, height);

    painter->drawImage(targetRect, m_image, sourceRect);
}

void MediaPlayer::SetConverter(EquirectangularToFovVideo *pEqToFov)
{
    m_pEqToFov = pEqToFov;
    LoadFrame(m_curFrame);
}

// PRIVATE:

void MediaPlayer::LoadFrame(int frameNum)
{
    // check if frame number is within the video
    frameNum = frameNum<0? 0: frameNum;
    frameNum = frameNum>=m_videoParams.numOfFrames? m_videoParams.numOfFrames-1: frameNum;
    long int frameTime = (long int)(1000000.0*frameNum/m_videoParams.GetFrameRate());

    string framePath = VideoExtractor::GetFramename(frameNum);

    if (m_pEqToFov)
    {
        bool res = m_tmpImage.load(framePath.c_str());
        UNUSED(res);
        if (m_image.height() != FOV_IMAGE_HEIGHT)
        {
            int height = FOV_IMAGE_HEIGHT;
            double aspectRatio = m_pEqToFov->GetAspectRatio();
            int width = height * aspectRatio;
            m_image = QImage(width, height, m_tmpImage.format());
        }
        m_pEqToFov->Convert(&m_tmpImage, frameTime, &m_image);
    }
    else
    {
        bool res = m_image.load(framePath.c_str()); 
        UNUSED(res);
    }

    m_curFrame = frameNum;
}
