/// VideoExtractor.h
/// This class provides functionality for extracting the frames of a video.
/// Curently it extracts at a fixed directory within /tmp.

#ifndef __VIDEOEXTRACTOR_H__
#define __VIDEOEXTRACTOR_H__

#include <string>

extern "C"
{
    #include <libavformat/avformat.h>
}

using namespace std;

// structure holding video parameters
struct VideoParams
{
    string videoname;
    int         fps_num;
    int         fps_den;
    int         width;
    int         height;
    int         numOfFrames;

    double GetFrameRate()
    {
        if (fps_den <=0)
            return -1.0;

        return (double)fps_num/fps_den;
    }
};

class VideoExtractor
{
public:
    static bool IsVideoExtracted(string videoname);

    static bool ExtractFrames(string videoname);

    static string GetFramename(int frameNum);

    static string GetParamsname();

    static bool LoadVideoParams(string filename, VideoParams &rVideoParams);

private:
    VideoExtractor();
    VideoExtractor(const VideoExtractor &f);
    VideoExtractor &operator=(const VideoExtractor &f);

    static void SaveFrame(AVFrame *pFrame, int width, int height, int iFrame);

    static void SaveVideoParams(VideoParams videoParams, string filename);

    static bool CreateBaseDir();
};

#endif /* __VIDEOEXTRACTOR_H__ */
