#include "medianfliterdehazor.h"
#include "colorcorrect.h"

MedianFliterDehazor::MedianFliterDehazor(std::string &imagePath, uchar max_A, float eps)
    :_eps(eps),_max_A(max_A)
{

    rawImage =cv::imread(imagePath,CV_LOAD_IMAGE_COLOR);
    Init();
}

void MedianFliterDehazor::Process()
{
    std::cout <<"medianfilter dehaze start"<<std::endl;
    GenerateAverageImage();
    GenerateDarkImage();
    GenerateAtmosphericRadiation();
    GenerateDehazeImage();

}

void MedianFliterDehazor::Init()
{
    std::cout <<"init"<<std::endl;
    std::cout <<"rows : "<<rawImage.rows<<" cols:"<<rawImage.cols<<std::endl;
    //init windowsize for fliter
    _windowSize =((rawImage.rows >rawImage.cols)?rawImage.rows:rawImage.cols);
    _windowSize *=0.05;
    if(_windowSize <50)
        _windowSize =50;
    std::cout <<"windowsize: "<<_windowSize<<std::endl;

    averageImage =cv::Mat(rawImage.rows,rawImage.cols,CV_8UC1,cv::Scalar(0));
    subDarkImage =cv::Mat(rawImage.rows,rawImage.cols,CV_8UC1,cv::Scalar(0));
    darkImage =cv::Mat(rawImage.rows,rawImage.cols,CV_8UC1,cv::Scalar(0));

    cv::split(rawImage,rawImages);
//    rawImage_b =rawImages[0];
}

void MedianFliterDehazor::GenerateAverageImage()
{
    std::cout <<"GenerateAverageImage "<<std::endl;
    int sum =0;
    for(int i =0;i <rawImage.rows;++i)
    {
        for(int j =0;j <rawImage.cols;++j)
        {
            uchar temp =((rawImages[0].at<uchar>(i,j) >rawImages[1].at<uchar>(i,j))?
                        rawImages[0].at<uchar>(i,j):rawImages[1].at<uchar>(i,j));
            subDarkImage.at<uchar>(i,j) =((temp >rawImages[2].at<uchar>(i,j))?temp:rawImages[2].at<uchar>(i,j));
            sum +=subDarkImage.at<uchar>(i,j);
        }
    }
    _avgM =sum/rawImage.rows*rawImage.cols*255;
    cv::boxFilter(subDarkImage,averageImage,CV_8UC1,cv::Size(_windowSize,_windowSize));
}

void MedianFliterDehazor::GenerateDarkImage()
{
    std::cout <<"GenerateDarkImage "<<std::endl;
    for(int i =0;i <rawImage.rows;++i)
        for(int j =0;j <rawImage.cols;++j)
        {
            float temp1 =_eps*_avgM;
            if(temp1 >0.9) temp1 =0.9;
            uchar temp2 =temp1*averageImage.at<uchar>(i,j);
            darkImage.at<uchar>(i,j) =(temp2 <subDarkImage.at<uchar>(i,j)?temp2:subDarkImage.at<uchar>(i,j));
        }
}

void MedianFliterDehazor::GenerateAtmosphericRadiation()
{
    std::cout <<"GenerateAtmosphericRadiation "<<std::endl;
    int count = darkImage.cols*darkImage.rows;
    int edgeValue =255;
    std::vector<int> darkHisto;
    ColorCorrect:: GenerateHistogram(darkHisto,darkImage);
    while (darkHisto[edgeValue] >count*(1- 0.001)) {
        --edgeValue;
    }
    --edgeValue;

    std::cout <<edgeValue<<std::endl;
    int c=0;
    int tempB =0,tempG =0,tempR =0;

    for(int i =0;i< darkImage.rows;++i)
    {
        for(int j =0;j< darkImage.cols;++j)
        {
            if(darkImage.at<uchar>(i,j) >= edgeValue)
            {
                tempB +=(int)(rawImages[0].at<uchar>(i,j));
                tempG +=(int)(rawImages[1].at<uchar>(i,j));
                tempR +=(int)(rawImages[2].at<uchar>(i,j));
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

void MedianFliterDehazor::GenerateDehazeImage()
{
    std::cout <<"GenerateDehazeImage"<<std::endl;
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
            std::cout <<j<<std::endl;
            int temp_b = _A.b*(rawImages[0].at<uchar>(i,j) -darkImage.at<uchar>(i,j)/(_A.b -darkImage.at<uchar>(i,j)));
            int temp_g = _A.g*(rawImages[1].at<uchar>(i,j) -darkImage.at<uchar>(i,j)/(_A.g -darkImage.at<uchar>(i,j)));
            int temp_r = _A.r*(rawImages[2].at<uchar>(i,j) -darkImage.at<uchar>(i,j)/(_A.r -darkImage.at<uchar>(i,j)));
//            std::cout <<"1"<<std::endl;
            temp_b = ((temp_b>_max_A)?rawImages[0].at<uchar>(i,j):temp_b);
            temp_g = ((temp_g>_max_A)?rawImages[1].at<uchar>(i,j):temp_g);
            temp_r = ((temp_r>_max_A)?rawImages[2].at<uchar>(i,j):temp_r);
//            std::cout <<"2"<<std::endl;
            temp_b = ((temp_b <0)?0:temp_b);
            temp_g = ((temp_g <0)?0:temp_g);
            temp_r = ((temp_r <0)?0:temp_r);
//            std::cout <<"3"<<std::endl;
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
    ColorCorrect::AutoColor(dehazeImage,0.01,0.01);
    cv::imwrite("C:\\hr\\experiment\\tempimage\\images\\nonuniform_median_dehaze.jpg",dehazeImage);
    //    cv::imshow("dehaze",dehazeImage);

    std::cout <<"dehaze finished"<<std::endl;
}


