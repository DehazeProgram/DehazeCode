#include "darkimagedehazor.h"
#include "filter.h"
#include "colorcorrect.h"
#include "time.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>


DarkImageDehazor::DarkImageDehazor(std::string &imagePath,  float eps, float t, int max_a)
    :_eps(eps),_max_A(max_a),_t0(t)
{
    rawImage =cv::imread(imagePath,CV_LOAD_IMAGE_COLOR);
    Init();
}


DarkImageDehazor::DarkImageDehazor(cv::Mat &img):rawImage(img)
{
    Init();
}

void DarkImageDehazor::Init()
{
    _minFliterWindowSize =((rawImage.cols>rawImage.rows)?rawImage.cols:rawImage.rows);
    _minFliterWindowSize *=0.018;


    darkChannelImage = cv::Mat(rawImage.rows,rawImage.cols,CV_8UC1,cv::Scalar(0));
    dehazeImage      = cv::Mat(rawImage.rows,rawImage.cols,CV_8UC1,cv::Scalar(0));
    transmission   = cv::Mat(rawImage.rows,rawImage.cols,CV_32FC1, cv::Scalar(0));

    cv::split(rawImage,channelLayers);
    

}

void DarkImageDehazor::Process()
{
//    std::vector<int> dst;
//    std::vector<int> src;
//    std::deque<int> max;
//    src.push_back(5);
//    src.push_back(3);
//    src.push_back(45);
//    src.push_back(67);
//    src.push_back(77);
//    src.push_back(456);
//    src.push_back(35);
//    src.push_back(67);
//    src.push_back(22);
//    src.push_back(55);
//    src.push_back(97);
//    src.push_back(66);
//    src.push_back(344);
//    src.push_back(673);
//    src.push_back(673);
//    src.push_back(673);
//    src.push_back(673);
//    Filter::MaxFilter_1D(src,dst,4,max);

//    for (int i =0;i <dst.size();++i)
//    {
//        std::cout <<dst[i]<<std::endl;
//    }

    std::cout <<"start darkchanneldehaze!"<<std::endl;
    std::cout <<"WindowSize: "<<_minFliterWindowSize<<std::endl;
    std::cout <<"row: "<<rawImage.rows<<" cols: "<<rawImage.cols<<std::endl;
    GenerateDarkImage();
    GenerateAtmosphericRadiation();
    GenereteTransmmision();
    GenerateDehazeImage();
}

void DarkImageDehazor::GenerateDarkImage()
{
    Filter::DarkImageFilter(rawImage,_minFliterWindowSize,darkChannelImage);
    std::cout <<"darkImage generated! "<<  std::endl;
    //    cv::imshow("darkimg",darkChannelImage);
    cv::imwrite("C:\\hr\\experiment\\tempimage\\images\\darkChannelImage.jpg",darkChannelImage);

}

void DarkImageDehazor::GenerateAtmosphericRadiation()
{
    int count = darkChannelImage.cols*darkChannelImage.rows;
    int edgeValue =255;
    std::vector<int> darkHisto;
    ColorCorrect:: GenerateHistogram(darkHisto,darkChannelImage);
    while (darkHisto[edgeValue] >count*(1- 0.001)) {
        --edgeValue;
    }
    --edgeValue;

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
    tempB /=c;
    tempG /=c;
    tempR /=c;

    _A.b = ((tempB > _max_A)?_max_A:tempB);
    _A.g = ((tempG > _max_A)?_max_A:tempG);
    _A.r = ((tempR > _max_A)?_max_A:tempR);

    std::cout <<"A generated!"<<std::endl;

}

void DarkImageDehazor::GenereteTransmmision()
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
    Filter::GuideFilter_Single(channelLayers[0],trans_t,transmission,_minFliterWindowSize*4,_eps);

    //        cv::imshow("transmission",transmission);

    //just for imwrite
    cv::Mat_<float> mt(rawImage.rows,rawImage.cols,255);
    cv::Mat_<float> n(rawImage.rows,rawImage.cols,1.0/255.0);
    cv::Mat m;
    cv::multiply(transmission,mt,m);//transmission 是小数 这一步将其转化为（0，255）
    cv::imwrite("C:\\hr\\experiment\\dehazeimage\\transmission.jpg",m);
    ColorCorrect::ContractEnhancement(m,0.15,0.05,0.3);
    cv::imwrite("C:\\hr\\experiment\\dehazeimage\\transmission1.jpg",m);

    m.convertTo(m,CV_32F);
    cv::multiply(n,m,transmission);//对比的拉伸之后的图像归一化
    std::cout <<"t generated!"<<std::endl;

}

void DarkImageDehazor::GenerateDehazeImage()
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

            int temp_b = (int)((channelLayers[0].at<uchar>(i,j)-_A.b)/t +_A.b);
            int temp_g = (int)((channelLayers[1].at<uchar>(i,j)-_A.g)/t +_A.g);
            int temp_r = (int)((channelLayers[2].at<uchar>(i,j)-_A.r)/t +_A.r);

            temp_b = ((temp_b>_max_A)?channelLayers[0].at<uchar>(i,j):temp_b);
            temp_g = ((temp_g>_max_A)?channelLayers[1].at<uchar>(i,j):temp_g);
            temp_r = ((temp_r>_max_A)?channelLayers[2].at<uchar>(i,j):temp_r);

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
    //    std::cout <<"haha"<<std::endl;
    //    ColorCorrect::AutoContract(dehazeImage,0.01,0.01);
    //    ColorCorrect::AutoColor(dehazeImage,0.05,0.05);
    //    cv::imwrite("C:\\hr\\experiment\\tempimage\\images\\1.bmp_darkchannel_dehaze1.jpg",dehazeImage);
    cv::imwrite("C:\\hr\\experiment\\dehazeimage\\IMG_30154_dehaze2.jpg",dehazeImage);
    //    cv::imshow("dehaze",dehazeImage);

    std::cout <<"dehaze finished"<<std::endl;
}
