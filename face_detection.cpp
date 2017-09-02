#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <dlib/opencv.h>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing/render_face_detections.h>
#include <dlib/image_processing.h>
#include <dlib/gui_widgets.h>

using namespace dlib;
using namespace std;

int inputImageWidth;
int inputImageHeight;

// ここのパラメータは目視で決めます。使用するディスプレイのサイズによって変更する。CamTwistでウィンドウ内部のみ選択する。
int outputImageWidth = 1250;
int outputImageHeight =768;
int logFrameNum = 30;
int personNum = 4; //画面に描画する人数

std::mutex mtx;
std::mutex mtx2;

struct less_than_x
{
    inline bool operator() (const cv::Rect& rect1, const cv::Rect& rect2)
    {
        return (rect1.x < rect2.x);
    }
};

cv::Rect dlibRectangleToOpenCVMat(dlib::rectangle r){
    return cv::Rect(r.left(), r.top(), r.width(), r.height());
}

void moveItr(std::vector<std::vector<cv::Rect>> &pastCvFaces){
    auto itrB = pastCvFaces.begin();
    auto itrE = pastCvFaces.end();
    auto tmp = itrE-1;
    if(itrE-itrB!=logFrameNum){
        while(1){
            *itrE = *tmp;
            --itrE;
            if(itrE==pastCvFaces.begin()){
                break;
            }
            --tmp;
        }
    }else if(itrE-itrB==logFrameNum){
        while(1){
            --itrE;
            if(itrE==pastCvFaces.begin()){
                break;
            }
            --tmp;
            *itrE = *tmp;
        }
    }
    *itrB={};
}

// vectorからindex番目の要素を削除する
template<typename T>
void remove(std::vector<T>& vector, unsigned int index)
{
    vector.erase(vector.begin() + index);
}

//上下の黒帯を削除
cv::Mat modifyFrame(cv::Mat frame){
    cv::Rect rect(0,inputImageHeight/4,inputImageWidth,inputImageHeight/2);
    cv::Mat smallFrame(frame,rect);
    return smallFrame;
}

void detectFace(cv::Mat &frame, frontal_face_detector &detector, std::vector<cv::Rect> &cvFaces, std::vector<std::vector<cv::Rect>> &pastCvFaces, int detectCount){
    while(1){
        mtx.lock();
        std::vector<cv::Rect> cvFacesTmp;
        cv::Mat modFrame = modifyFrame(frame);
        cv_image<bgr_pixel> cimg(modFrame);
        std::vector<rectangle> faces = detector(cimg);
        for(int i=0;i<faces.size();i++){
            cv::Rect facesTmp = dlibRectangleToOpenCVMat(faces[i]);
            cvFacesTmp.push_back(facesTmp);
        }
        cvFaces = cvFacesTmp;
        std::sort(cvFaces.begin(), cvFaces.end(), less_than_x());
        cout<<"cvFacespreSize"<<cvFaces.size()<<endl;

        int max=0; //直近フレームの人数
        int maxNum=0; //人数最大カウントしている場合でのフレーム番号
        for(int i= 0;i<(int)(pastCvFaces.size()/2);i++){
            if(pastCvFaces[i].size()>max){
                max = pastCvFaces[i].size();
                maxNum = i;
            }
        }
        int max2 = 0; //直近より前フレームの人数
        for(int i= (int)(pastCvFaces.size()/2);i<pastCvFaces.size();i++){
            if(pastCvFaces[i].size()>max2){
                max2 = pastCvFaces[i].size();
            }
        }
        moveItr(std::ref(pastCvFaces));
        if(max>=max2){ //人が減っていない場合、増減は検出誤差とみなす
            if(cvFaces.size()<max){
                cvFaces = pastCvFaces[maxNum+1]; //上でmoveItrして一つずれているため+1する
            }else{
                pastCvFaces[0] = cvFaces;
            }
        }else{ //人が減っているとみなす
            pastCvFaces[0] = cvFaces;
        }
        detectCount++;
        cout<<"detect"<<detectCount<<endl;
        mtx.unlock();
    }
}

std::vector<cv::Mat> getFaceImg(cv::Mat inputImage,
                        std::vector<cv::Rect> faces,
                        int rectWidth,
                        int rectHeight)
{
    std::vector<cv::Mat> faceImg;
    // 検出した対象を囲む矩形を元画像に描画
    for (int i = 0; i < faces.size(); i++){
        float cornerX = faces[i].x+(faces[i].width*0.5)-(rectWidth*0.5);
        //float cornerY = faces[i].y+(faces[i].height*0.5)-(rectHeight*0.5);
        if(cornerX<0){
            cornerX=0;
        }
        if(cornerX+rectWidth>inputImageWidth){
            cornerX = inputImageWidth - rectWidth;
        }

        //Panacast2の解像度はHDで1920*540。ただし上端と下端に黒い帯が入っているので、中央の540px分の高さだけ取り出したい
        cv::Rect rect(cornerX,inputImageHeight/4,rectWidth,rectHeight);
        cv::Mat rectImg(inputImage,rect);
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

cv::Mat makeOutputImage(cv::Mat inputImage, std::vector<cv::Rect> faces)
{
    std::vector<cv::Mat> faceImg;
    cv::Mat backgroundImg(cv::Size(outputImageWidth, outputImageHeight), CV_8UC3, cv::Scalar::all(255));
    cv::Mat outputImage;
    int rectHeight = inputImageHeight/2;
    int rectWidth = rectHeight/3;
        
    if(faces.size()!=0){
        faceImg = getFaceImg(inputImage,faces,rectWidth,rectHeight);
        float resizeFactor = (float)outputImageHeight/rectHeight;
        
        //4人描画したい場合は4
        int movePtr = outputImageWidth/personNum;
        int ptrX=0;
        int ptrY=0;
        for(int i=0;i<faceImg.size();i++){    
            cv::Mat tmp;
            cv::resize(faceImg[i],faceImg[i],cv::Size(),resizeFactor,resizeFactor);
            PinP_tr(backgroundImg,faceImg[i],ptrX,ptrY);
            if(ptrX<outputImageWidth-rectWidth){
                ptrX+=movePtr;
            }else{
                ptrX=0;
                ptrY+=rectHeight;
            }
        }
    }
    return backgroundImg;
}

int main(int argc, char* argv[])
{
    try
    {
        string videoStr = "video";
        string cameraStr = "camera";
        string inputStr = std::string(argv[1]);
        
        cv::VideoCapture cap;
        if(inputStr == videoStr){
            cap.open(argv[2]);
        }else if(inputStr == cameraStr){
            int num = std::stoi(argv[2]);
            cap.open(num);
        }
        if (!cap.isOpened())
        {
            cerr << "Unable to connect to camera" << endl;
            return 1;
        }

        inputImageWidth = cap.get(CV_CAP_PROP_FRAME_WIDTH);
        inputImageHeight = cap.get(CV_CAP_PROP_FRAME_HEIGHT);

        image_window win;

        std::vector<std::vector<cv::Rect>> pastCvFaces(logFrameNum);
        // Load face detection and pose estimation models.
        frontal_face_detector detector = get_frontal_face_detector();
        shape_predictor pose_model;
        deserialize("shape_predictor_68_face_landmarks.dat") >> pose_model;

        int count = 0;
        int capCount = 0;
        int detectCount = 0;

        cv::Mat frame;
        std::vector<cv::Rect> cvFaces;

        cv::Mat outputImage;

        while(1){
            
            cap>>frame;
            
            if(count == 0){
                std::thread th(detectFace, std::ref(frame), std::ref(detector), std::ref(cvFaces), std::ref(pastCvFaces), detectCount);
                th.detach();
                count++;
            }  

            outputImage = makeOutputImage(frame, cvFaces);
            cv::imshow("outputImage", outputImage);
            int key3 = cv::waitKey(1);
            if(key3 == 113){ //qが押されたら終了
               break;
            }

            capCount++;
            cout<<"cap"<<capCount<<endl;
        }
    }

    catch(serialization_error& e)
    {
        cout << "You need dlib's default face landmarking model file to run this example." << endl;
        cout << "You can get it from the following URL: " << endl;
        cout << "   http://dlib.net/files/shape_predictor_68_face_landmarks.dat.bz2" << endl;
        cout << endl << e.what() << endl;
    }

    catch(exception& e)
    {
        cout << e.what() << endl;
    }

    return 0;

}