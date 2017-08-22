#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>

using namespace std;
using namespace cv;

int outputImageWidth = 1080;
int outputImageHeight = 720;

vector<Mat> getFaceImg(Mat inputImage,
                        vector<Rect> faces,
                        int rectWidth,
                        int rectHeight)
{
    vector<Mat> faceImg;
    // 検出した対象を囲む矩形を元画像に描画
    for (int i = 0; i < faces.size(); i++){
        float cornerX = faces[i].x+(faces[i].width*0.5)-(rectWidth*0.5);
        float cornerY = faces[i].y+(faces[i].height*0.5)-(rectHeight*0.5);
        if(cornerX<0){
            cornerX=0;
        }
        if(cornerY<0){
            cornerY=0;
        }
        if(cornerX+rectWidth>outputImageWidth){
            cornerX = outputImageWidth - rectWidth;
        }
        if(cornerY+rectHeight>outputImageHeight){
            cornerY = outputImageHeight - rectHeight;
        }
        //Point corner1(faces[i].x+(faces[i].width*0.5)-(rectWidth*0.5),(faces[i].y+(faces[i].height*0.5)-(rectHeight*0.5)));
        //Point corner2((faces[i].x+(faces[i].width*0.5)+(rectWidth*0.5)),(faces[i].y+(faces[i].height*0.5)+(rectHeight*0.5)));
        Rect rect(cornerX,cornerY,rectWidth,rectHeight);
        Mat rectImg(inputImage,rect);
        faceImg.push_back(rectImg);
    }
    return faceImg;
}

void PinP_tr(const cv::Mat srcImg, const cv::Mat smallImg, const int tx, const int ty)
 {
    //前景画像の変形行列
    cv::Mat mat = (cv::Mat_<double>(2,3)<<1.0, 0.0, tx, 0.0, 1.0, ty);

    //アフィン変換の実行
    cv::warpAffine(smallImg, srcImg, mat, srcImg.size(), CV_INTER_LINEAR, cv::BORDER_TRANSPARENT);
 }

Mat makeOutputImage(Mat inputImage,
                vector<Rect> faces,
                CascadeClassifier cascade)
{
    // 指定した識別器を用いて画像から対象を検出
    cascade.detectMultiScale(inputImage, faces, 1.1,3,0,Size(20,20));
    for (int i = 0; i < faces.size(); i++){
        rectangle(inputImage, Point(faces[i].x,faces[i].y),Point(faces[i].x + faces[i].width,faces[i].y + faces[i].height),Scalar(0,200,0),3,CV_AA);
    }

    vector<Mat> faceImg;
    Mat backgroundImg(cv::Size(outputImageWidth, outputImageHeight), CV_8UC3, cv::Scalar::all(255));
    Mat outputImage;
    int rectWidth;
    int rectHeight;
    
    if(faces.size()==0 || faces.size()>6){
        return inputImage;
    }else{
        //人数により切り取る範囲を変更
        if(faces.size()==1){
            rectWidth = outputImageWidth;
            rectHeight = outputImageHeight;
            faceImg = getFaceImg(inputImage,faces,rectWidth,rectHeight);
        }else if(faces.size()==2){
            rectWidth = outputImageWidth/2;
            rectHeight = outputImageHeight;
            faceImg = getFaceImg(inputImage,faces,rectWidth,rectHeight);
        }else if(faces.size()==3){
            rectWidth = outputImageWidth/3;
            rectHeight = outputImageHeight;
            faceImg = getFaceImg(inputImage,faces,rectWidth,rectHeight);
        }else if(faces.size()==4){
            rectWidth = outputImageWidth/2;
            rectHeight = outputImageHeight/2;
            faceImg = getFaceImg(inputImage,faces,rectWidth,rectHeight);
        }else if(faces.size()==5){
            rectWidth = outputImageWidth/3;
            rectHeight = outputImageHeight/2;
            faceImg = getFaceImg(inputImage,faces,rectWidth,rectHeight);
        }else if(faces.size()==6){
            rectWidth = outputImageWidth/3;
            rectHeight = outputImageHeight/2;
            faceImg = getFaceImg(inputImage,faces,rectWidth,rectHeight);
        }
        //cout<<"num_of_people"<<faces.size()<<endl;

        int ptrX=0;
        int ptrY=0;
        /*
        while(ptr.y<outputImageHeight){
            while(ptr.x<outputImageWidth){
                paste(backgroundImg,faceImg[i])
            }
        }*/
        for(int i=0;i<faceImg.size();i++){
            PinP_tr(backgroundImg,faceImg[i],ptrX,ptrY);
            if(ptrX<outputImageWidth){
                ptrX+=rectWidth;
            }else{
                ptrX=0;
                ptrY+=rectHeight;
            }
        }
        return backgroundImg;
        /*
        // 顔周辺領域の矩形切り抜き
        vector<Mat> faceImg;

        // 検出した対象を囲む矩形を元画像に描画
        for (int i = 0; i < faces.size(); i++){
            float cornerX = faces[i].x+(faces[i].width*0.5)-(rectWidth*0.5);
            float cornerY = faces[i].y+(faces[i].height*0.5)-(rectHeight*0.5);
            if(cornerX<0){
                cornerX=0;
            }
            if(cornerY<0){
                cornerY=0;
            }
            if(cornerX+rectWidth>outputImageWidth){
                cornerX = outputImageWidth - rectWidth;
            }
            if(cornerY+rectHeight>outputImageHeight){
                cornerY = outputImageHeight - rectHeight;
            }
            //Point corner1(faces[i].x+(faces[i].width*0.5)-(rectWidth*0.5),(faces[i].y+(faces[i].height*0.5)-(rectHeight*0.5)));
            //Point corner2((faces[i].x+(faces[i].width*0.5)+(rectWidth*0.5)),(faces[i].y+(faces[i].height*0.5)+(rectHeight*0.5)));
            Rect rect(cornerX,cornerY,rectWidth,rectHeight);
            Mat rectImg(inputImage,rect);
            faceImg.push_back(rectImg);
        }
        */
    }
}

int main() {
    int channel = 1;

    int fourcc   = cv::VideoWriter::fourcc('M', 'J', 'P', 'G');
    double fps   = 30.0;
    bool isColor = true;

    if(channel == 0){
        CascadeClassifier cascade;
        cascade.load("/usr/local/share/OpenCV/haarcascades/haarcascade_frontalface_alt2.xml");
        vector<Rect> faces;

        Mat image = imread("./media/dempa.jpg");

        // 指定した識別器を用いて画像から対象を検出
        cascade.detectMultiScale(image, faces, 1.1,3,0,Size(20,20));  
        // 検出した対象を囲む矩形を元画像に描画
        for (int i = 0; i < faces.size(); i++){
            rectangle(image, Point(faces[i].x,faces[i].y),Point(faces[i].x + faces[i].width,faces[i].y + faces[i].height),Scalar(0,200,0),3,CV_AA);
        }
        imshow("Detect Faces", image);
        waitKey(0);
        return 0;
    }

    if(channel==1){
        // カメラの起動
        VideoCapture cap(0);

        CascadeClassifier cascade;
        //検出器の種類選択
        //Haar：精度が高い
        //haarcascade_frontalface_default.xmlが一番検出度が高いように思える。その分、誤検出も多い。
        //cascade.load("/usr/local/share/OpenCV/haarcascades/haarcascade_frontalface_default.xml");
        //cv::VideoWriter writer("output.avi", fourcc, fps, cv::Size(outputImageWidth,outputImageHeight), isColor);
        //cascade.load("/usr/local/share/OpenCV/haarcascades/haarcascade_upperbody.xml");
        //cv::VideoWriter writer("upperbodyTracker.mp4", fourcc, fps, cv::Size(outputImageWidth,outputImageHeight), isColor);
        //lbp：高速
        cascade.load("/usr/local/share/OpenCV/lbpcascades/lbpcascade_frontalface_improved.xml");
        vector<Rect> faces;
        Mat frame;
        cout<<"1"<<endl;

        while (1) {
            // カメラから取得した画像をフレームに落とし込む
            cap >> frame;
            cout<<"2"<<endl;            
            /*
            // 指定した識別器を用いて画像から対象を検出
            cascade.detectMultiScale(frame, faces, 1.1,3,0,Size(20,20));
            // 検出した対象を囲む矩形を元画像に描画
            
            for (int i = 0; i < faces.size(); i++){
                rectangle(frame, Point(faces[i].x,faces[i].y),Point(faces[i].x + faces[i].width,faces[i].y + faces[i].height),Scalar(0,200,0),3,CV_AA);
            }
            */

            Mat faceImage = makeOutputImage(frame,
                                            faces,
                                            cascade);
            //cout<<faceImage.cols<<","<<faceImage.rows<<endl;
            imshow("Detect Faces", faceImage);
            //writer << faceImage;
            cout<<"3"<<endl;
            int key = waitKey(30);
            if (key == 113) {//qが押されたら終了
                break;
            }
        }
        return 0;
    }
}




















