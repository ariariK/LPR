#ifndef LPR_DEMO_L_OCR_H
#define LPR_DEMO_L_OCR_H

#include <cstdio>
#include "utils.h"
#include "opencv2/opencv.hpp"
#include "tensorflow/lite/interpreter.h"
#include "tensorflow/lite/kernels/register.h"
#include "tensorflow/lite/model.h"
#include "tensorflow/lite/optional_debug_tools.h"

namespace krlpr {

class l_ocr
{
private:
    /* data */
    bool verbose;
    int number_of_threads;
    std::unique_ptr<tflite::FlatBufferModel> model;
    std::unique_ptr<tflite::Interpreter> interpreter;
public:
    l_ocr(std::string model_path);
    ~l_ocr();
    std::vector<std::string> run_inference(std::vector<cv::Mat> l_imgs);
    /* get lpr string and draw on the frame */
    int run_inference(std::vector<krlprutils::TargetBox> &boxes, cv::Mat& src);
};

}   //namespace krlpr

#endif
