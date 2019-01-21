/// VideoWidget.h

#ifndef __VIDEOWIDGET_H__
#define __VIDEOWIDGET_H__

#include <QtWidgets>
#include <QMutex>
#include "PaintGaze.h"
#include "MediaPlayer.h"
#include "EquirectangularToFovVideo.h"

// Forward declare
class Arff;

class VideoWidget : public QWidget
{
    Q_OBJECT

signals:
    void SendTime(int time);
    ///< emits the time shift from m_qMediaplayer in us.

public slots:
    void HandleTime(int time, QObject *pSender);
    ///< Handles the signal for changing the position on the video.

    void HandleToggleView();
    ///< It toggles the displayed video if FOV data are available

private slots:
    void HandleTimeChanged(qint64 time);
    ///< Takes care of hadling the signal from m_qMediaPlayer and transmits it
    ///< as being its own.

public:
    VideoWidget(QWidget *parent=0);

    ~VideoWidget();

    QSize minimumSizeHint() const Q_DECL_OVERRIDE;
    
    void SetCurrentTime(int currentTime);
    ///< Sets the time of the video in us.

    bool SetMedia(QString pathToFile);
    ///< Sets media to the provided local path.

    void SetGazePaint(PaintGaze *pPaintGaze);
    
    void Play();
    ///< Starts playing video.

    void Pause();
    ///< Pauses video.

    void TogglePlayer();
    ///< Toggles the state of the video from stopped to playing.

    qint64 Duration();
    ///< Returns the video duration in us.
    ///< \b Available only after starting playing the video.

    void ConvertToFov(Arff *pArff);
    ///< By calling this function we signal that the video should be converted
    ///< from 360 degrees equirectangular to field of view.

protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    ///< Called on update(), repaint() and then passes painter to surface in order to paint.

    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
    ///< Update area on resize.

private:
    MediaPlayer             m_mediaPlayer;
    PaintGaze               *m_pPaintGaze; // pointer to gaze painter
    EquirectangularToFovVideo    *m_pEqToFov;
    qint64                  m_previousTime;
    bool                    m_isPaused; // used for correcting time offset when pausing
    QMutex                  m_mutex; // mutex for updating QMediaPlayer time
    bool                    m_toggled; // toggle the diplayed frame
};

#endif /*__MEDIAPLAYER_H__*/
