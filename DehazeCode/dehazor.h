#ifndef DEHAZOR_H
#define DEHAZOR_H
#include <opencv2/opencv.hpp>
#include <iostream>
#include"filter.h"

struct ARadiation
{
    int r;
    int g;
    int b;
};

class Dehazor
{
public:
    Dehazor(cv::Mat &img, int windowSize, float eps, float t, int max_a);
    Dehazor(cv::Mat &img);
    inline int getMinFliterWindowSize(){ return _minFliterWindowSize;}
    inline int getGuideFliterWindoeSize(){ return _guideFliterWindowSize;}
    inline int getMaxAtmosphericRadiation() {return _max_A;}

    inline void setMinFliterWindowSize(int s){ _minFliterWindowSize =s;}
    inline void setGuideFliterWindowSize(int s){ _guideFliterWindowSize =s;}
    inline void setMaxAtmosphericRadiation(int a) {  _max_A =a;}

    void process();

private:
    cv::Mat rawImage;
    cv::Mat rawImage_b;
    cv::Mat rawImage_g;
    cv::Mat rawImage_r;
    cv::Mat darkChannelImage;
    cv::Mat transmission_b;
    cv::Mat transmission_g;
    cv::Mat transmission_r;
    cv::Mat dehazeImage;
    std::vector<cv::Mat > channelLayers;

    int _minFliterWindowSize;
    int _guideFliterWindowSize;
    float _max_A;
    float _eps;
    float _t0;
    ARadiation _A;
//    Filter filter;

    void init();

    void GenerateDarkImage();
    void GenerateAtmosphericRadiation();
    void GenereteTransmmision();
    void GenerateDehazeImage();
};

#endif // DEHAZOR_H
