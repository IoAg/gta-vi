// VideoExtractor.cpp

#include "VideoExtractor.h"

extern "C"
{
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
}

#include <stdio.h>
#include <fstream>
#include <sys/stat.h>
#include <cstdlib>

using namespace std;

#define MAX_FRAME_WIDTH 1920
#define MAX_CHARS_PER_LINE 512
#define TOKENS_PER_LINE 2
// compatibility with newer API
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(55,28,1)
#define av_frame_alloc avcodec_alloc_frame
#define av_frame_free avcodec_free_frame
#endif

// constants for saving frames and video data
const string basePath = "/tmp/gta-vi/";
const string frameBasename = "frame";
const string frameExt = ".ppm";
const string paramsFile = "video_parameters.txt";


// PUBLIC

/*static*/ bool VideoExtractor::IsVideoExtracted(string videoname)
{
    VideoParams videoParams;
    string paramsname = GetParamsname();
    if (!LoadVideoParams(paramsname, videoParams))
        return false;

    if (videoParams.videoname.compare(videoname) != 0)
        return false;

    return true;
}

/*static*/ bool VideoExtractor::ExtractFrames(string videoname)
{
  // Initalizing these to NULL prevents segfaults!
  AVFormatContext   *pFormatCtx = NULL;
  size_t            i;
  int               videoStream;
  AVCodecContext    *pCodecCtxOrig = NULL;
  AVCodecContext    *pCodecCtx = NULL;
  AVCodec           *pCodec = NULL;
  AVFrame           *pFrame = NULL;
  AVFrame           *pFrameRGB = NULL;
  AVPacket          packet;
  int               frameFinished;
  int               numBytes;
  uint8_t           *buffer = NULL;
  struct SwsContext *sws_ctx = NULL;
  VideoParams       videoParams; // parameters of the stored video
  videoParams.videoname = videoname;

  // Register all formats and codecs
  av_register_all();
  
  // Open video file
  if(avformat_open_input(&pFormatCtx, videoname.c_str(), NULL, NULL)!=0)
    return false; // Couldn't open file
  
  // Retrieve stream information
  if(avformat_find_stream_info(pFormatCtx, NULL)<0)
    return false; // Couldn't find stream information
  
  // Dump information about file onto standard error
  av_dump_format(pFormatCtx, 0, videoname.c_str(), 0);
  
  // Find the first video stream
  videoStream=-1;
  for(i=0; i<pFormatCtx->nb_streams; i++)
    if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO) {
      videoStream=i;
      break;
    }
  if(videoStream==-1)
    return false; // Didn't find a video stream
  
  // Get a pointer to the codec context for the video stream
  pCodecCtxOrig=pFormatCtx->streams[videoStream]->codec;
  // Get frame rate
  videoParams.fps_num = pFormatCtx->streams[videoStream]->avg_frame_rate.num;
  videoParams.fps_den = pFormatCtx->streams[videoStream]->avg_frame_rate.den;  
  // Find the decoder for the video stream
  pCodec=avcodec_find_decoder(pCodecCtxOrig->codec_id);
  if(pCodec==NULL) {
    fprintf(stderr, "Unsupported codec!\n");
    return false; // Codec not found
  }
  // Copy context
  pCodecCtx = avcodec_alloc_context3(pCodec);
  if(avcodec_copy_context(pCodecCtx, pCodecCtxOrig) != 0) {
    fprintf(stderr, "Couldn't copy codec context");
    return false; // Error copying codec context
  }

  // Open codec
  if(avcodec_open2(pCodecCtx, pCodec, NULL)<0)
    return false; // Could not open codec
  
  // Allocate video frame
  pFrame=av_frame_alloc();
  
  // Allocate an AVFrame structure
  pFrameRGB=av_frame_alloc();
  if(pFrameRGB==NULL)
    return false;

  // Determine required buffer size and allocate buffer
  numBytes=avpicture_get_size(PIX_FMT_RGB24, pCodecCtx->width,
			      pCodecCtx->height);
  buffer=(uint8_t *)av_malloc(numBytes*sizeof(uint8_t));
  
  int out_width = pCodecCtx->width;
  int out_height = pCodecCtx->height;
  while (out_width > MAX_FRAME_WIDTH)
  {
      out_width /= 2; // halving size makes extraction faster
      out_height = (int)((double)out_width*pCodecCtx->height/pCodecCtx->width);
  }
  videoParams.width = out_width;
  videoParams.height = out_height;
  //int out_height = 300;
  // Assign appropriate parts of buffer to image planes in pFrameRGB
  // Note that pFrameRGB is an AVFrame, but AVFrame is a superset
  // of AVPicture
  avpicture_fill((AVPicture *)pFrameRGB, buffer, PIX_FMT_RGB24,
		 out_width, out_height);
  
  // initialize SWS context for software scaling
  sws_ctx = sws_getContext(pCodecCtx->width,
			   pCodecCtx->height,
			   pCodecCtx->pix_fmt,
			   out_width,
			   out_height,
			   PIX_FMT_RGB24,
			   SWS_BILINEAR,
			   NULL,
			   NULL,
			   NULL
			   );

  // make sure the output directory exists
  if (!CreateBaseDir())
      return false;

  // Read frames and save first five frames to disk
  i=0;
  while(av_read_frame(pFormatCtx, &packet)>=0) {
    // Is this a packet from the video stream?
    if(packet.stream_index==videoStream) {
      // Decode video frame
      avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, &packet);
      
      // Did we get a video frame?
      if(frameFinished) {
	    // Convert the image from its native format to RGB
	    sws_scale(sws_ctx, (uint8_t const * const *)pFrame->data,
		    pFrame->linesize, 0, pCodecCtx->height,
		    pFrameRGB->data, pFrameRGB->linesize);
	
    	// Save the frame to disk
    	SaveFrame(pFrameRGB, out_width, out_height,i++);
      }
    }
    
    // Free the packet that was allocated by av_read_frame
    av_free_packet(&packet);
  }

  videoParams.numOfFrames = i;
  string paramsFile = GetParamsname();
  SaveVideoParams(videoParams, paramsFile);
  
  // Free the RGB image
  av_free(buffer);
  av_frame_free(&pFrameRGB);
  
  // Free the YUV frame
  av_frame_free(&pFrame);
  
  // Close the codecs
  avcodec_close(pCodecCtx);
  avcodec_close(pCodecCtxOrig);

  // Close the video file
  avformat_close_input(&pFormatCtx);
  
  return true;
}

// PRIVATE


/*static*/ string VideoExtractor::GetFramename(int frameNum)
{
    string frameName = basePath + frameBasename + to_string(frameNum) + frameExt;
    return frameName;
}

/*static*/ string VideoExtractor::GetParamsname()
{
    return basePath+paramsFile;
}

/*static*/ void VideoExtractor::SaveFrame(AVFrame *pFrame, int width, int height, int iFrame) {
  FILE *pFile;
  int  y;
  
  // Open file
  //sprintf(szFilename, "frame%d.ppm", iFrame);
  //pFile=fopen(szFilename, "wb");
  string framename = GetFramename(iFrame);
  pFile=fopen(framename.c_str(), "wb");
  if(pFile==NULL)
    return;
  
  // Write header
  fprintf(pFile, "P6\n%d %d\n255\n", width, height);
  
  // Write pixel data
  for(y=0; y<height; y++)
    fwrite(pFrame->data[0]+y*pFrame->linesize[0], 1, width*3, pFile);
  
  // Close file
  fclose(pFile);
}

// functions for storing and loading video parameters
/*static*/ void VideoExtractor::SaveVideoParams(VideoParams videoParams, string filename)
{
    ofstream ofile;
    ofile.open(filename.c_str());

    ofile << "videoname: " << videoParams.videoname << "\n";
    ofile << "fps_num: " << videoParams.fps_num << "\n";
    ofile << "fps_den: " << videoParams.fps_den << "\n";
    ofile << "width: " << videoParams.width << "\n";
    ofile << "height: " << videoParams.height << "\n";
    ofile << "numOfFrames: " << videoParams.numOfFrames << "\n";

    ofile.close();
}

/*static*/ bool VideoExtractor::LoadVideoParams(string filename, VideoParams &rVideoParams)
{
    ifstream ifile;
    ifile.open(filename.c_str());
    if (!ifile.is_open())
        return false;

    char buf[MAX_CHARS_PER_LINE];
    char *token[TOKENS_PER_LINE];

    while(!ifile.getline(buf, MAX_CHARS_PER_LINE).eof())
    {
        token[0] = strtok(buf, " ");
        token[1] = strtok(0, " ");
        if (string(token[0]).compare("videoname:") == 0)
            rVideoParams.videoname = token[1];
        else if (string(token[0]).compare("fps_num:") == 0)
            rVideoParams.fps_num = atoi(token[1]);
        else if (string(token[0]).compare("fps_den:") == 0)
            rVideoParams.fps_den = atoi(token[1]);
        else if (string(token[0]).compare("width:") == 0)
            rVideoParams.width = atoi(token[1]);
        else if (string(token[0]).compare("height:") == 0)
            rVideoParams.height = atoi(token[1]);
        else if (string(token[0]).compare("numOfFrames:") == 0)
            rVideoParams.numOfFrames = atoi(token[1]);

    }

    ifile.close();
    return true;
}

/*static*/ bool VideoExtractor::CreateBaseDir()
{
    struct stat info;
    // directory already exists
    if( stat(basePath.c_str(), &info) == 0 )
        return true;

    // otherwise create it
    string command = "mkdir -p " + basePath;
    //const int dir_err = mkdir(basePath.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH); // this command cannot create last level directory if all the precious directories do not exist
    const int dir_err = system(command.c_str());
    if (dir_err == 0)
        return true;

    return false;
}
