 #include "colorcorrect.h"

ColorCorrect::ColorCorrect()
{
}

void ColorCorrect::AutoColor(cv::Mat &image, float s1, float s2)
{
    CV_Assert(image.type() == CV_8UC1 || image.type() == CV_8UC3);
    if(image.type() == CV_8UC1)
        AutoColor_single(image,s1,s2);
    if(image.type() == CV_8UC3)
    {
        std::vector<cv::Mat> images;
        cv::split(image,images);
        for(int i=0;i <images.size();++i)
            AutoColor_single(images[i],s1,s2);
    }
}

void ColorCorrect::GenerateHistogram(std::vector<int> &histo,const cv::Mat &image)
{
    CV_Assert(image.type() == CV_8UC1);
    histo.resize(256,0);
    for(int i =0;i <image.rows;++i)
    {
        for(int j =0;j< image.cols;++j)
        {
            ++histo[image.at<uchar>(i,j)];
        }
    }

    for(int i =1;i< histo.size();++i)
        histo[i] =histo[i] +histo[i-1];
}

void ColorCorrect::AutoColor_single(cv::Mat &image, float s1, float s2)
{
    if ((image.depth() != CV_8UC1 ) > FLT_EPSILON)
        image.convertTo(image, CV_8UC1);

    int count =image.cols*image.rows;
    std::vector<int> histo;

    GenerateHistogram(histo,image);

    int max =254,min =0;

    while (histo[min] < count*s1) {
        ++min;
    }

    if(min >0)
        --min;

    while (histo[max] < count*(1 - s2)) {
        ++max;
    }

    if(max <255)
        ++max;

    for(int i =0;i <image.rows;++i)
    {
        for(int j =0;j <image.cols;++j)
        {
            if(image.at<uchar>(i,j) >max)
                image.at<uchar>(i,j) =max;
            if(image.at<uchar>(i,j) <min)
                image.at<uchar>(i,j) =min;
            image.at<uchar>(i,j) = (image.at<uchar>(i,j) - min)*255/(max -min);
        }
    }
}
