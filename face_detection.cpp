// The contents of this file are in the public domain. See LICENSE_FOR_EXAMPLE_PROGRAMS.txt
/*

    This example program shows how to find frontal human faces in an image and
    estimate their pose.  The pose takes the form of 68 landmarks.  These are
    points on the face such as the corners of the mouth, along the eyebrows, on
    the eyes, and so forth.  
    

    This example is essentially just a version of the face_landmark_detection_ex.cpp
    example modified to use OpenCV's VideoCapture object to read from a camera instead 
    of files.


    Finally, note that the face detector is fastest when compiled with at least
    SSE2 instructions enabled.  So if you are using a PC with an Intel or AMD
    chip then you should enable at least SSE2 instructions.  If you are using
    cmake to compile this program you can enable them by using one of the
    following commands when you create the build project:
        cmake path_to_dlib_root/examples -DUSE_SSE2_INSTRUCTIONS=ON
        cmake path_to_dlib_root/examples -DUSE_SSE4_INSTRUCTIONS=ON
        cmake path_to_dlib_root/examples -DUSE_AVX_INSTRUCTIONS=ON
    This will set the appropriate compiler options for GCC, clang, Visual
    Studio, or the Intel compiler.  If you are using another compiler then you
    need to consult your compiler's manual to determine how to enable these
    instructions.  Note that AVX is the fastest but requires a CPU from at least
    2011.  SSE4 is the next fastest and is supported by most current machines.  
*/

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

int outputImageWidth;
int outputImageHeight;
int logFrameNum = 30;

std::mutex mtx;

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

void capToFrame(cv::VideoCapture cap, cv::Mat& frame){
    cap >> frame;
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

void detectFace(cv::Mat frame, frontal_face_detector &detector, std::vector<cv::Rect> &cvFaces, std::vector<std::vector<cv::Rect>> &pastCvFaces){
    cv_image<bgr_pixel> cimg(frame);
    std::vector<rectangle> faces = detector(cimg);
    for(int i=0;i<faces.size();i++){
        mtx.lock();
        cvFaces.push_back(dlibRectangleToOpenCVMat(faces[i]));
        //cv::Rect cvFace = dlibRectangleToOpenCVMat(faces[i]);
        //cv::rectangle(frame,cv::Point(cvFace.x, cvFace.y),cv::Point(cvFace.x+cvFace.width,cvFace.y+cvFace.height),cv::Scalar(0,200,0),3,CV_AA);
        mtx.unlock();
    }
    std::sort(cvFaces.begin(), cvFaces.end(), less_than_x());

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
    //cout<<"max:"<<max<<",max2:"<<max2<<",cvFaces:"<<cvFaces.size()<<endl;
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
    /*
    for(int i=0;i<pastCvFaces.size();i++){
        for(int j=0;j<pastCvFaces[i].size();j++){
            cout<<i<<","<<j<<":"<<pastCvFaces[i][j]<<endl;
        }    
    }
    */
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
        cv::Rect rect(cornerX,cornerY,rectWidth,rectHeight);
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
    int rectWidth;
    int rectHeight;
    
    if(faces.size()==0){
        return inputImage;
    }else{
        rectWidth = outputImageWidth/2;
        rectHeight = outputImageHeight/2;

        faceImg = getFaceImg(inputImage,faces,rectWidth,rectHeight);

        int ptrX=0;
        int ptrY=0;
        
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
    }
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
            cap.open(0);
        }
        if (!cap.isOpened())
        {
            cerr << "Unable to connect to camera" << endl;
            return 1;
        }

        outputImageWidth = cap.get(CV_CAP_PROP_FRAME_WIDTH);
        outputImageHeight = cap.get(CV_CAP_PROP_FRAME_HEIGHT);

        image_window win;

        std::vector<std::vector<cv::Rect>> pastCvFaces(logFrameNum);
        // Load face detection and pose estimation models.
        frontal_face_detector detector = get_frontal_face_detector();
        shape_predictor pose_model;
        deserialize("shape_predictor_68_face_landmarks.dat") >> pose_model;

        // Grab and process frames until the main window is closed by the user.
        while(1)
        {
            // Grab a frame
            cv::Mat frame;
            std::thread th1(capToFrame, cap, std::ref(frame));
            th1.join();
            // Turn OpenCV's Mat into something dlib can deal with.  Note that this just
            // wraps the Mat object, it doesn't copy anything.  So cimg is only valid as
            // long as temp is valid.  Also don't do anything to temp that would cause it
            // to reallocate the memory which stores the image as that will make cimg
            // contain dangling pointers.  This basically means you shouldn't modify temp
            // while using cimg.
            
            std::vector<cv::Rect> cvFaces;
            std::thread th2(detectFace, frame, std::ref(detector), std::ref(cvFaces), std::ref(pastCvFaces));
            th2.join();
            /*
            for(int i=0; i<cvFaces.size(); i++){
                cv::rectangle(frame,cv::Point(cvFaces[i].x, cvFaces[i].y),cv::Point(cvFaces[i].x+cvFaces[i].width,cvFaces[i].y+cvFaces[i].height),cv::Scalar(0,200,0),3,CV_AA);
            }
            cv::imshow("outputImage",frame);
            */

            cv::Mat outputImage = makeOutputImage(frame,cvFaces);

            cv::imshow("outputImage",outputImage);
            int key = cv::waitKey(30);
            if(key == 113){ //qが押されたら終了
                break;
            }
            /*
            // Find the pose of each face.
            std::vector<full_object_detection> shapes;
            for (unsigned long i = 0; i < faces.size(); ++i)
                shapes.push_back(pose_model(cimg, faces[i]));

            // Display it all on the screen
            win.clear_overlay();
            win.set_image(cimg);
            win.add_overlay(render_face_detections(shapes));
            */
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