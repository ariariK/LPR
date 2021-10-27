#ifndef LPR_DEMO_UTILS_H
#define LPR_DEMO_UTILS_H

#include "opencv2/opencv.hpp"
#include <vector>
#include <string>
#include <stdint.h>

#define LOG(severity) (std::cerr << (#severity) << ": ")

#define LPR_CHECK(x)                                            \
  if (!(x)) {                                                   \
    fprintf(stderr, "Error at %s:%d\n", __FILE__, __LINE__);    \
    exit(1);                                                    \
  }

namespace krlpr{
namespace krlprutils {

class TargetBox
{
public:
    int x1;   // upper left point in frame
    int y1;
    int x2;   // low right point in frame
    int y2;

    cv::Mat lpr_src;          // lp img src
    std::string lpr_string;   // lp string
    int cate;
    float score;

    float getWidth() { return (x2 - x1); };
    float getHeight() { return (y2 - y1); };
    float area() { return getWidth() * getHeight(); };
};

std::string CTCGready(uint8_t *data);
bool LicenseMatch(std::string license);
int PredHandle(float *out[2], std::vector<TargetBox> &dstBoxes, 
                              const float scaleW, const float scaleH, const float thresh);
int NMSHandle(std::vector<TargetBox> &tmpBoxes, 
                             std::vector<TargetBox> &dstBoxes);
void hwc_to_chw(cv::InputArray &src, cv::OutputArray &dst);
int HWC2CHW(float *src, float *dst, int height, int width, int channel);
void DrawBoxes(cv::Mat &src, std::vector<TargetBox> boxes, cv::Scalar color);
std::vector<TargetBox> 
DecodeInfer(uint8_t *out_data, int stride, int input_height, int input_width, int out_height, int out_width, int out_channel, 
            int img_height, int img_width, int num_classes, std::vector<float> anchors, float threshold);
void nms(std::vector<TargetBox> &input_boxes, float NMS_THRESH);

}   //namespace krlprutils
}   //namespace krlpr

#endif
