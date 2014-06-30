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
    transmission_b   = cv::Mat(rawImage.rows,rawImage.cols,CV_32F, cv::Scalar(0));
    transmission_g   = cv::Mat(rawImage.rows,rawImage.cols,CV_32F, cv::Scalar(0));
    transmission_r   = cv::Mat(rawImage.rows,rawImage.cols,CV_32F, cv::Scalar(0));

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
    cv::Mat trans_t_b(transmission_b.rows,transmission_b.cols, CV_32F);
    cv::Mat trans_t_g(transmission_g.rows,transmission_g.cols, CV_32F);
    cv::Mat trans_t_r(transmission_r.rows,transmission_r.cols, CV_32F);

    for(int i =0;i< trans_t_b.rows;++i)
    {
        for(int j =0;j< trans_t_b.cols;++j)
        {
            float dark = static_cast<float>(darkChannelImage.at<uchar>(i,j));
            float temp_b =(1.0 - _eps*(dark/_A.b));
            float temp_g =(1.0 - _eps*(dark/_A.g));
            float temp_r =(1.0 - _eps*(dark/_A.r));

            if(temp_b <0)
                trans_t_b.at<float>(i,j) =0;
            else if(temp_b >1)
                trans_t_b.at<uchar>(i,j) =1;
            else
                trans_t_b.at<float>(i,j) =temp_b;

            if(temp_g <0)
                trans_t_g.at<float>(i,j) =0;
            else if(temp_g >1)
                trans_t_g.at<uchar>(i,j) =1;
            else
                trans_t_g.at<float>(i,j) =temp_g;

            if(temp_r <0)
                trans_t_r.at<float>(i,j) =0;
            else if(temp_r >1)
                trans_t_r.at<uchar>(i,j) =1;
            else
                trans_t_r.at<float>(i,j) =temp_r;
        }
    }
    Filter::GuideFilter_Single(rawImage_b,trans_t_b,transmission_b,_minFliterWindowSize*4,_eps);
    Filter::GuideFilter_Single(rawImage_b,trans_t_g,transmission_g,_minFliterWindowSize*4,_eps);
    Filter::GuideFilter_Single(rawImage_b,trans_t_r,transmission_r,_minFliterWindowSize*4,_eps);
    //    filter.guideFilter_single(rawImage_b,trans_t,transmission,_minFliterWindowSize*4,_eps);
    cv::imshow("transmission",transmission_b);

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
            float t_b = transmission_b.at<float>(i,j);
            float t_g = transmission_g.at<float>(i,j);
            float t_r = transmission_r.at<float>(i,j);

            if(t_b <_t0)
                t_b= _t0;
            if(t_g <_t0)
                t_g= _t0;
            if(t_r <_t0)
                t_r= _t0;

            int temp_b = (int)((rawImage_b.at<uchar>(i,j)-_A.b)/t_b +_A.b);
            int temp_g = (int)((rawImage_g.at<uchar>(i,j)-_A.g)/t_g +_A.g);
            int temp_r = (int)((rawImage_r.at<uchar>(i,j)-_A.r)/t_r +_A.r);

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
    cv::imwrite("E:\\tempimage\\images\\dehazes.jpg",dehazeImage);
    cv::imshow("dehaze",dehazeImage);

    std::cout <<"dehaze finished"<<std::endl;
}
