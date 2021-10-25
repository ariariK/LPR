#ifndef LPR_DEMO_L_DETECT_H
#define LPR_DEMO_L_DETECT_H

#include <cstdio>

#include "opencv2/opencv.hpp"
#include "tensorflow/lite/interpreter.h"
#include "tensorflow/lite/kernels/register.h"
#include "tensorflow/lite/model.h"
#include "tensorflow/lite/optional_debug_tools.h"
#include "inference/utils.h"

namespace krlpr {

class l_detect
{
private:
    /* tensorflowlite data */
    bool verbose;
    int number_of_threads;
    std::unique_ptr<tflite::FlatBufferModel> model;
    std::unique_ptr<tflite::Interpreter> interpreter;
    /* opencv_dnn data */
    const float anchors[2][6] = { {26.13,9.58, 46.67,18.37, 77.69,25.90}, {84.59,46.55, 133.05,30.85, 176.99,64.33} };
	const float stride[3] = { 16.0, 32.0 };
	const int inpWidth = 640;
	const int inpHeight = 480;
	const int num_stage = 2;
	const int anchor_num = 3;
	float objThreshold;
	float confThreshold;
	float nmsThreshold;
	std::vector<std::string> classes;
	const std::string classesFile = "../lpr.names";
	int num_class;
	cv::dnn::Net net;
	void drawPred(int classId, float conf, int left, int top, int right, int bottom, cv::Mat& frame);
public:
    l_detect(std::string model_path);
    l_detect(std::string model_path, float obj_Threshold, float conf_Threshold, float nms_Threshold);
    ~l_detect();
    void run_inference(cv::Mat& src, std::vector<krlprutils::TargetBox>& res);
};

}   //namespace krlpr

#endif
