#include <iostream>
#include<fstream>
#include <opencv2/opencv.hpp>
#include <darkimagedehazor.h>
#include <medianfliterdehazor.h>
#include <ctime>
#include"filter.h"
using namespace std;
using namespace cv;

int main()
{
    time_t start = clock();
    DarkImageDehazor darkChannelDehaze(std::string("C:\\hr\\experiment\\tempimage\\images\\IMG_30155.JPG"),1,0.01,220);
    darkChannelDehaze.Process();
    time_t darkChannelfinish =clock();
    std::cout <<"darkChannelDehaze cost: "<<(darkChannelfinish - start)<<"ms"<<std::endl;
    MedianFliterDehazor medianDehaze(std::string("C:\\hr\\experiment\\tempimage\\images\\IMG_30155.JPG"),220,1.3);
    medianDehaze.Process();
    std::cout <<"medianDehaze cost: "<<(clock() - darkChannelfinish)<<"ms"<<std::endl;
    waitKey();
    return 0;
}
