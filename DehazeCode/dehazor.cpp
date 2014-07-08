#include "dehazor.h"
#include "filter.h"
#include "colorcorrect.h"
#include "time.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>


Dehazor::Dehazor(cv::Mat &img, int windowSize,  float eps, float t, int max_a)
    :rawImage(img),_minFliterWindowSize(windowSize),
      _eps(eps),_max_A(max_a),_t0(t)
{
    init();
}

Dehazor::Dehazor(cv::Mat &img):rawImage(img)
{
    init();
}

void Dehazor::init()
{
    std::cout <<"row: "<<rawImage.rows<<" cols: "<<rawImage.cols<<std::endl;
    darkChannelImage = cv::Mat(rawImage.rows,rawImage.cols,CV_8UC1,cv::Scalar(0));
    dehazeImage      = cv::Mat(rawImage.rows,rawImage.cols,CV_8UC1,cv::Scalar(0));
    transmission   = cv::Mat(rawImage.rows,rawImage.cols,CV_32F, cv::Scalar(0));

    cv::split(rawImage,channelLayers);
    rawImage_b =channelLayers[0];
    rawImage_g =channelLayers[1];
    rawImage_r =channelLayers[2];

}

void Dehazor::process()
{
    GenerateDarkImage();
    GenerateAtmosphericRadiation();
    GenereteTransmmision();
    GenerateDehazeImage();
}

void Dehazor::GenerateDarkImage()
{


    Filter::DarkImageFilter(rawImage,_minFliterWindowSize,darkChannelImage);
    std::cout <<"darkImage generated! "<<  std::endl;
    cv::imshow("darkimg",darkChannelImage);
}

void Dehazor::GenerateAtmosphericRadiation()
{
    int count = darkChannelImage.cols*darkChannelImage.rows;
    int edgeValue =255;
    std::vector<int> darkHisto;
    ColorCorrect:: GenerateHistogram(darkHisto,darkChannelImage);
    while (darkHisto[edgeValue] >count*(1- 0.001)) {
        --edgeValue;
    }
    --edgeValue;

    std::cout <<edgeValue<<std::endl;
    int c=0;
    int tempB =0,tempG =0,tempR =0;
    for(int i =0;i< darkChannelImage.rows;++i)
    {
        for(int j =0;j< darkChannelImage.cols;++j)
        {
            if(darkChannelImage.at<uchar>(i,j) >= edgeValue)
            {
                tempB +=(int)(channelLayers[0].at<uchar>(i,j));
                tempG +=(int)(channelLayers[1].at<uchar>(i,j));
                tempR +=(int)(channelLayers[2].at<uchar>(i,j));
                ++c;
            }
        }
    }
    std::cout <<c<<std::endl;
    tempB /=c;
    tempG /=c;
    tempR /=c;

    _A.b = ((tempB > _max_A)?_max_A:tempB);
    _A.g = ((tempG > _max_A)?_max_A:tempG);
    _A.r = ((tempR > _max_A)?_max_A:tempR);

    std::cout <<"A generated!"<<std::endl;

}

void Dehazor::GenereteTransmmision()
{
    cv::Mat_<float> trans_t= cv::Mat(rawImage.rows,rawImage.cols,CV_32F, cv::Scalar(0));

    for(int i =0;i< transmission.rows;++i)
    {
        for(int j =0;j< transmission.cols;++j)
        {
            float dark = static_cast<float>(darkChannelImage.at<uchar>(i,j));
            float temp_b =static_cast<float>(1.0 - _eps*(dark/_A.b));
            float temp_g =static_cast<float>(1.0 - _eps*(dark/_A.g));
            float temp_r =static_cast<float>(1.0 - _eps*(dark/_A.r));
            float t;

            if(temp_b <0) temp_b =0;
            else if(temp_b >1) temp_b =1;

            if(temp_g <0) temp_g =0;
            else if(temp_g >1) temp_g =1;

            if(temp_r <0) temp_r =0;
            else if(temp_r >1) temp_r =1;

            t =((temp_b>temp_g)?temp_b:temp_g);
            t =((t >temp_r)?t:temp_r);
            trans_t.at<float>(i,j) =t;
        }
    }
    Filter::GuideFilter_Single(rawImage_b,trans_t,transmission,_minFliterWindowSize*4,_eps);
    cv::imshow("transmission",transmission);

    std::cout <<"t generated!"<<std::endl;

}

void Dehazor::GenerateDehazeImage()
{
    int row = rawImage.rows;
    int col = rawImage.cols;
    cv::Mat dehaze_b = cv::Mat(row,col,CV_8UC1);
    cv::Mat dehaze_g = cv::Mat(row,col,CV_8UC1);
    cv::Mat dehaze_r = cv::Mat(row,col,CV_8UC1);
    std::cout <<_A.b<<" "<<_A.g<<" "<<_A.r<<std::endl;
    for(int i =0;i <row;++i)
    {
        for(int j=0;j< col;++j)
        {
            float t = transmission.at<float>(i,j);

            if(t <_t0)
                t= _t0;

            int temp_b = (int)((rawImage_b.at<uchar>(i,j)-_A.b)/t +_A.b);
            int temp_g = (int)((rawImage_g.at<uchar>(i,j)-_A.g)/t +_A.g);
            int temp_r = (int)((rawImage_r.at<uchar>(i,j)-_A.r)/t +_A.r);

            temp_b = ((temp_b>_max_A)?rawImage_b.at<uchar>(i,j):temp_b);
            temp_g = ((temp_g>_max_A)?rawImage_g.at<uchar>(i,j):temp_g);
            temp_r = ((temp_r>_max_A)?rawImage_r.at<uchar>(i,j):temp_r);

            temp_b = ((temp_b <0)?0:temp_b);
            temp_g = ((temp_g <0)?0:temp_g);
            temp_r = ((temp_r <0)?0:temp_r);

            dehaze_b.at<uchar>(i,j) = temp_b;
            dehaze_g.at<uchar>(i,j) = temp_g;
            dehaze_r.at<uchar>(i,j) = temp_r;
        }
    }

    std::vector<cv::Mat> dehazes;
    dehazes.push_back(dehaze_b);
    dehazes.push_back(dehaze_g);
    dehazes.push_back(dehaze_r);

    cv::merge(dehazes,dehazeImage);
//    ColorCorrect::AutoColor(dehazeImage,0.001,0.001);
    cv::imwrite("C:\\hr\\experiment\\tempimage\\images\\nonuniform_dehaze.jpg",dehazeImage);
    cv::imshow("dehaze",dehazeImage);

    std::cout <<"dehaze finished"<<std::endl;
}