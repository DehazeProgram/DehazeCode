#ifndef MEDIANFLITERDEHAZOR_H
#define MEDIANFLITERDEHAZOR_H


#include <iostream>
#include <string>
#include <opencv2/opencv.hpp>

struct ARadiation_MF
{
    int r;
    int g;
    int b;
};

class MedianFliterDehazor
{
public:
    MedianFliterDehazor(std::string &imagePath,uchar max_A,float eps);
    void Process();

private:
    float _eps;
    float _avgM;
    int _windowSize;
    uchar _max_A;
    ARadiation_MF _A;
    cv::Mat rawImage;
    cv::Mat darkImage;
    cv::Mat averageImage;
    cv::Mat subDarkImage;
    cv::Mat dehazeImage;
    std::vector<cv::Mat> rawImages;

    void Init();
    void GenerateAverageImage();
    void GenerateDarkImage();
    void GenerateAtmosphericRadiation();
    void GenerateDehazeImage();

};

#endif // MEDIANFLITERDEHAZOR_H
