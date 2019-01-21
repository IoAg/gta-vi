/// MediaPlayer.h

#ifndef __MEDIAPLAYER_H__
#define __MEDIAPLAYER_H__

#include <QtWidgets>
#include <QTimer>
#include <QTime>
#include <QPainter>
#include <QThread>

#include "VideoExtractor.h"
#include "EquirectangularToFovVideo.h"

/// The following class tries to imitate the QMediaPlayer, but it comes with
/// some differentitaions. The functions that start in lower case imitate the 
/// Qt ones/ The functions that start with upper case are unique toi this 
/// implementation

class MediaPlayer : public QObject
{
    Q_OBJECT

signals:
    void positionChanged(qint64);
    ///< emits the current time of player in ms

private slots:
    void HandleTimerTick();
    ///< handle for timer tick

public:
    MediaPlayer(QObject *parent=0);

    ~MediaPlayer();

    enum State {PausedState, StoppedState, PlayingState};
    ///< Enumerator holding the player state.

    void setPosition(qint64 position);
    ///< Sets the position of the player in ms.

    bool setMedia(QUrl videoFile);
    ///< Sets the input file.

    void play();
    ///< Starts player.

    void pause();
    ///< Pauses player.

    State state() const;
    ///< Returns the current state of the player.

    qint64 duration();
    ///< Returns duration fo the video in ms.

    bool isVideoAvailable() const;
    ///< Returns true if video is available.

    void Paint(QPainter *painter, QSize size);
    ///< Paints the current image to the provided painter.

    void SetConverter(EquirectangularToFovVideo *pEqToFov);

private:
    QTimer          *m_pFpsTimer; // timer emulating the fps of the initial video
    QImage          m_image;
    QImage          m_tmpImage;
    QTime           m_time; // hold time since start of player
    int             m_startPlayFrame; // frame since we started playing video
    int             m_startPlayMs; // presentaion time of first frame since started playing video
    VideoParams     m_videoParams;
    double          m_frameDur; // duration of frame in ms
    State           m_state; // state of the player
    int             m_curFrame; // currently loaded frame
    bool            m_bVideoAv;
    EquirectangularToFovVideo *m_pEqToFov; // convert from equirectangular to field of view

    void LoadFrame(int frameNum);
    ///< Loads te provided frame and updates current frame state.
};

#endif /*__MEDIAPLAYER_H__*/
