#include "colorcorrect.h"
#include <map>

ColorCorrect::ColorCorrect()
{
}

void ColorCorrect::AutoColor(cv::Mat &image, float s1, float s2)
{
    CV_Assert(image.channels() == 1 || image.channels() == 3);
    if(image.type() !=CV_8UC1 ||image.type() !=CV_8UC3 )
    {
        if(image.channels() ==1)
            image.convertTo(image,CV_8UC1);
        if(image.channels() ==3)
            image.convertTo(image,CV_8UC3);
    }

    if(image.type() == CV_8UC1)
        AutoColor_single(image,s1,s2);
    if(image.type() == CV_8UC3)
    {
        std::vector<cv::Mat> images;
        cv::split(image,images);
        for(int i=0;i <images.size();++i)
            AutoColor_single(images[i],s1,s2);
        cv::merge(images,image);
    }

}

void ColorCorrect::AutoContract(cv::Mat &image, float s1, float s2)
{
    CV_Assert(image.channels() == 1 || image.channels() == 3);
    if(image.type() !=CV_8UC1 ||image.type() !=CV_8UC3 )
    {
        if(image.channels() ==1)
            image.convertTo(image,CV_8UC1);
        if(image.channels() ==3)
            image.convertTo(image,CV_8UC3);
    }
    if(image.channels() ==1)
    {
        ContractEnhancement(image,s1,s2,1);
    }
    else
    {
        //caculate min and max
        uchar min,max;
        std::vector<cv::Mat> channelimages;
        std::vector<uchar> mins,maxs;
        cv::split(image,channelimages);
        for(int i=0;i <channelimages.size();++i)
        {
            int count =image.cols*image.rows;
            std::vector<int> histo;
            GenerateHistogram(histo,channelimages[i]);
            int tmax =254,tmin =0;

            while (histo[tmin] < count*s1) {
                ++tmin;
            }
            if(tmin >0)
                --tmin;

            while (histo[tmax] > count*(1 - s2)) {
                --tmax;
            }
            if(tmax <255)
                ++tmax;
            mins.push_back(tmin);
            maxs.push_back(tmax);
        }
        min =((mins[0]<mins[1])?mins[0]:mins[1]);
        min =((min<mins[2])?min:mins[2]);
        max =((maxs[0]>maxs[1])?maxs[0]:maxs[1]);
        max =((max>maxs[2])?max:maxs[2]);
        //caculate map
        std::map<uchar,uchar> mapping;
        for(int i =0;i<256;++i)
        {
            if(i <min)
                mapping[i] =0;
            else if(i >max)
                mapping[i] =255;
            else
                mapping[i] = ((i -min)*255/(max-min));
        }
        //caculate autocontract image
        for(int i=0;i< image.rows;++i)
        {
            for(int j =0;j< image.cols;++j)
            {
                channelimages[0].at<uchar>(i,j) =mapping[channelimages[0].at<uchar>(i,j)];
                channelimages[1].at<uchar>(i,j) =mapping[channelimages[1].at<uchar>(i,j)];
                channelimages[2].at<uchar>(i,j) =mapping[channelimages[2].at<uchar>(i,j)];
            }
        }
        cv::merge(channelimages,image);
    }
}

void ColorCorrect::ContractEnhancement(cv::Mat &image, float s1, float s2,float eps)
{
    CV_Assert(image.channels() == 1 );
    if(image.type() !=CV_8UC1  )
    {
        if(image.channels() ==1)
            image.convertTo(image,CV_8UC1);
    }

    int count =image.cols*image.rows;
    std::vector<int> histo;

    GenerateHistogram(histo,image);

    int max =254,min =0;

    while (histo[min] < count*s1) {
        ++min;
    }

    if(min >0)
        --min;

    while (histo[max] > count*(1 - s2)) {
        --max;
    }

    if(max <255)
        ++max;

    min *=eps;
    std::cout <<"min: "<<min<<std::endl;
    for(int i =0;i <image.rows;++i)
    {
        for(int j =0;j <image.cols;++j)
        {
            if(image.at<uchar>(i,j) >max)
                image.at<uchar>(i,j) =max;
            if(image.at<uchar>(i,j) <min)
                image.at<uchar>(i,j) =min;

            image.at<uchar>(i,j) = ((image.at<uchar>(i,j) - min)*max)/(max -min);
        }
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

    while (histo[max] > count*(1 - s2)) {
        --max;
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
            image.at<uchar>(i,j) = ((image.at<uchar>(i,j) - min)*255)/(max -min);
        }
    }

}
