#ifndef __ZK_DECTOR2_H__
#define __ZK_DECTOR2_H__

#include <iostream>
#include "opencv2/opencv.hpp"
#include <mtcnn/mtcnn.h>
#include <flow/flow.h>
#include <utils/util.h>
#include <net/mx_shuffle_predict_2.h>

using namespace cv;
using namespace std;

namespace detect {

	class Detector
	{
	public:
		Detector();
		Detector(int method); // method: 1-˫��������
		int detectSpoofing(vector<Mat>& rgb_cameraFrames, vector<Mat>& ir_cameraFrames);
		int detectSpoofing2(vector<Mat>& rgb_cameraFrames, vector<Mat>& ir_cameraFrames);
		int detectSpoofing3(vector<Mat>& rgb_cameraFrames, vector<Mat>& ir_cameraFrames);
		int detectSpoofing4(vector<Mat>& rgb_cameraFrames);
		int detectSpoofing5(vector<Mat>& rgb_cameraFrames, vector<Mat>& ir_cameraFrames);
		int detectSpoofing6(vector<Mat>& rgb_cameraFrames, vector<Mat>& ir_cameraFrames);
		vector<FaceInfo> detectFace(Mat& img);
		int predict(Mat& img);
		int predict(PredictorHandle _predictor, Mat& img);
		int predict(PredictorHandle _predictor, vector<Mat>& motion2colors);
		int predict(vector<Mat>& motion2colors);
		bool getStartSampling();
		void setStartSampling(bool startSampling);
		bool getSamplePositiveData();
		void setSamplePositiveData(bool samplePositiveData);
		Mat getShowImgRGB();
		Mat getShowImgIR();
		Mat getMotion2color();
		void gatherVideoDataSet(Mat& rgb_img, Mat& nir_img);
		// type��0-rgb����ͼ��1-nir����ͼ��
		void gatherFlowDataSet(Mat& motion2color, int type);
		// type��0-ֻ����nir������1-ͬʱ����rgb��nir������2-ֻ����rgb����
		void getFlowMotion2Colors(vector<Mat>& rgb_cameraFrames, vector<Mat>& ir_cameraFrames, int type, FaceInfo maxFaceInfo_rgb, FaceInfo maxFaceInfo_ir, vector<Mat>& rgb_motion2colors, vector<Mat>& ir_motion2colors);

	private:
		PredictorHandle predictor = 0;
		PredictorHandle predictor_rgb = 0;
		MTCNN faceDetector;
		Rect detectFaceArea;
		float mtcnn_factor = 0.709f;
		float mtcnn_threshold[3] = { 0.7f, 0.6f, 0.6f };
		int mtcnn_minSize = 72; //minSize��Ӧ��С�����ߴ�:~(w x h��;72->~50x70, 96->~58x80, 120->~80x120, 240->~160x200
		int predict_img_min_width = 48;
		int predict_img_min_height = 64;
		bool _startSampling = false;
		bool _samplePositiveData = true;
		Mat _showImg_rgb;
		Mat _showImg_ir;
		Mat _motion2color;
		VideoWriter _vWriter_rgb_p;
		VideoWriter _vWriter_nir_p;
		VideoWriter _vWriter_rgb_n;
		VideoWriter _vWriter_nir_n;
		int _positiveFrameCount = 0;
		int _negativeFrameCount = 0;
		float _frameRate = 25.0;//��Ƶ��֡��
		Size _videoSize = Size(640, 480);
	};
	
}


#endif //__FLOW_H__
