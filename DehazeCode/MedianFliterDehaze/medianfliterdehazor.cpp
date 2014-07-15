#include "medianfliterdehazor.h"
#include "colorcorrect.h"
#include "time.h"


MedianFliterDehazor::MedianFliterDehazor(std::string &imagePath, uchar max_A, float eps)
    :_eps(eps),_max_A(max_A)
{

    rawImage =cv::imread(imagePath,CV_LOAD_IMAGE_COLOR);
    Init();
}

void MedianFliterDehazor::Process()
{
    std::cout <<"medianfilter dehaze start"<<std::endl;
    std::cout <<"windowsize: "<<_windowSize<<std::endl;
    std::cout <<"rows : "<<rawImage.rows<<" cols:"<<rawImage.cols<<std::endl;
    GenerateAverageImage();
    GenerateDarkImage();
    GenerateAtmosphericRadiation();
    GenerateDehazeImage();

}

void MedianFliterDehazor::Init()
{
//    std::cout <<"init"<<std::endl;
//    std::cout <<"rows : "<<rawImage.rows<<" cols:"<<rawImage.cols<<std::endl;
    //init windowsize for fliter
    _windowSize =((rawImage.rows >rawImage.cols)?rawImage.rows:rawImage.cols);
    _windowSize *=0.05;
    if(_windowSize <50)
        _windowSize =50;


    averageImage =cv::Mat(rawImage.rows,rawImage.cols,CV_8UC1,cv::Scalar(0));
    subDarkImage =cv::Mat(rawImage.rows,rawImage.cols,CV_8UC1,cv::Scalar(0));
    darkImage =cv::Mat(rawImage.rows,rawImage.cols,CV_8UC1,cv::Scalar(0));

    cv::split(rawImage,rawImages);

}

void MedianFliterDehazor::GenerateAverageImage()
{
    std::cout <<"GenerateAverageImage "<<std::endl;
    float sum =0;
    for(int i =0;i <rawImage.rows;++i)
    {
        for(int j =0;j <rawImage.cols;++j)
        {
            uchar temp =((rawImages[0].at<uchar>(i,j) <rawImages[1].at<uchar>(i,j))?
                        rawImages[0].at<uchar>(i,j):rawImages[1].at<uchar>(i,j));
            subDarkImage.at<uchar>(i,j) =((temp <rawImages[2].at<uchar>(i,j))?temp:rawImages[2].at<uchar>(i,j));
            sum +=subDarkImage.at<uchar>(i,j);
        }
    }
    float temp =rawImage.rows*rawImage.cols*255.0;
    _avgM =sum/temp;
    cv::boxFilter(subDarkImage,averageImage,CV_8UC1,cv::Size(_windowSize,_windowSize));
}

void MedianFliterDehazor::GenerateDarkImage()
{
    time_t darkstart =clock();
    std::cout <<"GenerateDarkImage "<<std::endl;
    for(int i =0;i <rawImage.rows;++i)
        for(int j =0;j <rawImage.cols;++j)
        {
            float temp1 =_eps*_avgM;
            if(temp1 >0.9) temp1 =0.9;
            uchar temp2 =temp1*averageImage.at<uchar>(i,j);
            darkImage.at<uchar>(i,j) =(temp2 <subDarkImage.at<uchar>(i,j)?temp2:subDarkImage.at<uchar>(i,j));
        }
//    std::cout<<darkImage<<std::endl;
    cv::imwrite("C:\\hr\\experiment\\tempimage\\images\\nonuniform_median_dark.jpg",darkImage);
    std::cout <<"generate darkimage cost :"<<clock()-darkstart<<std::endl;
}

void MedianFliterDehazor::GenerateAtmosphericRadiation()
{
    time_t startA =clock();
    std::cout <<"GenerateAtmosphericRadiation "<<std::endl;
    int count = darkImage.cols*darkImage.rows;
    int edgeValue =255;
    std::vector<int> darkHisto;
    ColorCorrect:: GenerateHistogram(darkHisto,darkImage);
    while (darkHisto[edgeValue] >count*(1- 0.001)) {
        --edgeValue;
    }
    --edgeValue;


    int c=0,b=0;
    int tempB =0,tempG =0,tempR =0;

    for(int i =0;i< darkImage.rows;++i)
    {
        for(int j =0;j< darkImage.cols;++j)
        {
            if(darkImage.at<uchar>(i,j)<0)
                b++;
            if(darkImage.at<uchar>(i,j) >= edgeValue)
            {
                tempB +=(int)(rawImages[0].at<uchar>(i,j));
                tempG +=(int)(rawImages[1].at<uchar>(i,j));
                tempR +=(int)(rawImages[2].at<uchar>(i,j));
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
    std::cout <<"generate A cost :"<<clock()-startA<<std::endl;
}

void MedianFliterDehazor::GenerateDehazeImage()
{
    time_t startdehazeimage=clock();
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
             int temp_b = _A.b*(rawImages[0].at<uchar>(i,j) -darkImage.at<uchar>(i,j));
             int temp_g = _A.g*(rawImages[1].at<uchar>(i,j) -darkImage.at<uchar>(i,j));
             int temp_r = _A.r*(rawImages[2].at<uchar>(i,j) -darkImage.at<uchar>(i,j));

            if(_A.b !=darkImage.at<uchar>(i,j))
                temp_b /=(_A.b -darkImage.at<uchar>(i,j));
            if(_A.g !=darkImage.at<uchar>(i,j))
                temp_g /=(_A.g -darkImage.at<uchar>(i,j));
            if(_A.r !=darkImage.at<uchar>(i,j))
                temp_r /=(_A.r -darkImage.at<uchar>(i,j));

            temp_b = ((temp_b>_max_A)?rawImages[0].at<uchar>(i,j):temp_b);
            temp_g = ((temp_g>_max_A)?rawImages[1].at<uchar>(i,j):temp_g);
            temp_r = ((temp_r>_max_A)?rawImages[2].at<uchar>(i,j):temp_r);

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
    cv::imwrite("C:\\hr\\experiment\\tempimage\\images\\1.bmp_median_dehaze.jpg",dehazeImage);
    //    cv::imshow("dehaze",dehazeImage);
    std::cout <<"generate dehazeimage cost :"<<clock()-startdehazeimage<<std::endl;
    std::cout <<"dehaze finished"<<std::endl;

}


