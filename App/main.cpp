#include <iostream>
#include<fstream>
#include <opencv2/opencv.hpp>
#include <darkimagedehazor.h>
#include <medianfliterdehazor.h>
#include "colorcorrect.h"
#include <ctime>
#include <vector>
#include"filter.h"
using namespace std;
using namespace cv;

void test()
{
    cv::Mat m = cv::imread(std::string("C:\\Users\\hr\\Pictures\\dogs\\11801583719e5366e5o.jpg"));
    std::vector <cv::Mat> m_splits;
    cv::split(m,m_splits);
    cv::Mat dst= cv::Mat(m.rows,m.cols,CV_8UC1);
    Filter::MaxFilter_2D(m_splits[1],dst,31);
//    std::cout << m_splits[1]<<std::endl;
//    std::cout << dst<<std::endl;
    cv::imshow("raw",m_splits[1]);
    cv::imshow("max",dst);
    std::cout <<m_splits[1].cols<<std::endl;
    std::cout <<dst.cols<<std::endl;
}


int main()
{
    test();
//    time_t start = clock();
//    DarkImageDehazor darkChannelDehaze(std::string("C:\\hr\\experiment\\images\\1.bmp"),1,0.01,220);
//    DarkImageDehazor darkChannelDehaze(std::string("C:\\hr\\experiment\\images\\IMG_30155.jpg"),1,0.01,220);
//    DarkImageDehazor darkChannelDehaze(std::string("C:\\hr\\experiment\\images\\nonuniform.JPG"),1,0.01,220);

//    darkChannelDehaze.Process();
//    time_t darkChannelfinish =clock();
//    std::cout <<"darkChannelDehaze cost: "<<(darkChannelfinish - start)<<"ms"<<std::endl;
//    MedianFliterDehazor medianDehaze(std::string("C:\\hr\\experiment\\tempimage\\images\\1.bmp"),220,1.3);
//    medianDehaze.Process();
//    std::cout <<"medianDehaze cost: "<<(clock() - darkChannelfinish)<<"ms"<<std::endl;
//    cv::Mat mat =cv::imread("C:\\hr\\experiment\\tempimage\\images\\1.bmp");
//    ColorCorrect::AutoColor(mat,0.01,0.01);
//    cv::imwrite("C:\\hr\\experiment\\tempimage\\images\\1.bmp_autocolor.jpg",mat);
    waitKey();
    return 0;
}


