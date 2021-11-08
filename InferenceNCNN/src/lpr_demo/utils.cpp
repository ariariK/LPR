#include "inference_ncnn/utils.h"
#include <algorithm>
#include <iostream>
#include <map>
#include <string>
#include <regex>
#include <cmath>
#include <cassert>

namespace krlpr{
namespace krlprutils {

//输出节点数
const int numOutput = 2;
//anchor num
const int numAnchor = 3;
//类别数目
const int numCategory = 2;
//NMS阈值
const int nmsThresh = 0.1;

//模型输入尺寸大小
const int inputWidth = 640;
const int inputHeight = 480;

//anchor box w h
std::vector<float> bias {26.13,9.58, 46.67,18.37, 77.69,25.90, 84.59,46.55, 133.05,30.85, 176.99,64.33};
std::vector<float> anchor {26.13,9.58, 46.67,18.37, 77.69,25.90, 84.59,46.55, 133.05,30.85, 176.99,64.33};
// std::vector<float> anchor {84.59,46.55, 133.05,30.85, 176.99,64.33, 26.13,9.58, 46.67,18.37, 77.69,25.90};

std::map<int, std::string > hangul_dict = {
        { 0, "0" },{ 1, "1" },{ 2, "2" },{ 3, "3" },{ 4, "4" },{ 5, "5" },
        { 6, "6" },{ 7, "7" },{ 8, "8" },{ 9, "9" },
        { 10, "가" },{ 11, "나" },{ 12, "다" },{ 13, "라" },{ 14, "마" },
        { 15, "거" },{ 16, "너" },{ 17, "더" },{ 18, "러" },{ 19, "머" },
        { 20, "버" },{ 21, "서" },{ 22, "어" },{ 23, "저" },{ 24, "고" },
        { 25, "노" },{ 26, "도" },{ 27, "로" },{ 28, "모" },{ 29, "보" },
        { 30, "소" },{ 31, "오" },{ 32, "조" },{ 33, "구" },{ 34, "누" },
        { 35, "두" },{ 36, "루" },{ 37, "무" },{ 38, "부" },{ 39, "수" },
        { 40, "우" },{ 41, "주" },{ 42, "하" },{ 43, "허" },{ 44, "호" },
        { 45, "바" },{ 46, "사" },{ 47, "아" },{ 48, "자" },{ 49, "배" },
        { 50, "서울" },{ 51, "부산" },{ 52, "대구" },{ 53, "인천" },{ 54, "광주" },
        { 55, "대전" },{ 56, "울산" },{ 57, "세종" },{ 58, "경기" },{ 59, "강원" },
        { 60, "충북" },{ 61, "충남" },{ 62, "전북" },{ 63, "전남" },{ 64, "경북" },
        { 65, "경남" },{ 66, "제주" },
    };

template <typename T>
size_t ArgMax(T *start, size_t size) {
  size_t max_index = 0;

  for(auto i = 0; i < size; i++) {
    if(start[i] > start[max_index]) {
      max_index = i;
    }
  }

  return max_index;
}

inline float fast_exp(float x) {
    union {
        uint32_t i;
        float f;
    } v{};
    v.i = (1 << 23) * (1.4426950409 * x + 126.93490512f);
    return v.f;
}

inline float sigmoid(float x) {
    return 1.0f / (1.0f + fast_exp(-x));
}

std::string CTCGready(uint8_t *data)
{
    uint8_t *p = data;
    
    std::string res;
    if(p == nullptr) {
        return res;
    }
    int arg_max[24] = {0};
    for(int i = 0; i < 24; i++) {
        arg_max[i] = ArgMax(p, 68);
        p += 68;
        if(i > 0) {
            if((arg_max[i] != arg_max[i-1]) && (arg_max[i-1] != 67)) {
                res += hangul_dict[arg_max[i-1]];
            }
        }
    }

    return res;
}

bool LicenseMatch(std::string license)
{
  /* not work */
  // std::locale old;
  // std::locale::global(std::locale("en_US.UTF-8"));

  // std::regex pattern("^(\\w\\w\\d{2}|\\d{2,3})\\D\\d{4}$", std::regex_constants::extended);
  // bool result = std::regex_match(license, pattern);

  // std::locale::global(old);

  // return result;
  return std::regex_match(license, std::regex("^(\\W{3}\\W{3}\\d{2}|\\d{2,3})\\W{3}\\d{4}$"));
}

int GetCategory(const float *values, int index, int &category, float &score)
{
    float tmp = 0;
    float objScore  = values[4 * numAnchor + index];

    for (int i = 0; i < numCategory; i++) {
        float clsScore = values[4 * numAnchor + numAnchor + i];
        clsScore *= objScore;

        if(clsScore > tmp) {
            score = clsScore;
            category = i;

            tmp = clsScore;
        }
    }
    
    return 0;
}

// (1, 30, 40, 17) (1, 15, 20, 17)
int PredHandle(float *out[2], std::vector<TargetBox> &dstBoxes, 
                              const float scaleW, const float scaleH, const float thresh)
{
    int channel[] = {17, 17};
    int height[] = {30, 15};
    int width[] = {40, 20};

    for (int i = 0; i < numOutput; i++) {
        int stride;
        int outW, outH, outC;

        outH = height[i];
        outW = width[i];
        outC = channel[i];

        // std::cout << "ih/oh: " << inputHeight / outH << std::endl;
        // std::cout << "iw/ow: " << inputWidth / outW << std::endl;
        assert(inputHeight / outH == inputWidth / outW);
        stride = inputHeight / outH;

        for (int h = 0; h < outH; h++) {
            const float* values = out[i] + h*outC*outW;

            for (int w = 0; w < outW; w++) {
                // for(int ii = 0; ii < 17; ii++) {
                //     std::cout << values[ii] << ",";
                // }
                // std::cout << std::endl;
                for (int b = 0; b < numAnchor; b++) {                    
                    //float objScore = values[4 * numAnchor + b];
                    TargetBox tmpBox;
                    int category = -1;
                    float score = -1;

                    GetCategory(values, b, category, score);

                    if (score > thresh) {
                        float bcx, bcy, bw, bh;

                        bcx = ((values[b * 4 + 0] * 2. - 0.5) + w) * stride;
                        bcy = ((values[b * 4 + 1] * 2. - 0.5) + h) * stride;
                        bw = pow((values[b * 4 + 2] * 2.), 2) * anchor[(i * numAnchor * 2) + b * 2 + 0];
                        bh = pow((values[b * 4 + 3] * 2.), 2) * anchor[(i * numAnchor * 2) + b * 2 + 1];
                        
                        tmpBox.x1 = (bcx - 0.5 * bw) * scaleW;
                        tmpBox.y1 = (bcy - 0.5 * bh) * scaleH;
                        tmpBox.x2 = (bcx + 0.5 * bw) * scaleW;
                        tmpBox.y2 = (bcy + 0.5 * bh) * scaleH;
                        tmpBox.score = score;
                        tmpBox.cate = category;

                        dstBoxes.push_back(tmpBox);
                    }
                }
                values += outC;
            }
        }
        // std::cout << "\n\n";
    }
    return 0;
}

float intersection_area(const TargetBox &a, const TargetBox &b)
{
    if (a.x1 > b.x2 || a.x2 < b.x1 || a.y1 > b.y2 || a.y2 < b.y1)
    {
        // no intersection
        return 0.f;
    }

    float inter_width = std::min(a.x2, b.x2) - std::max(a.x1, b.x1);
    float inter_height = std::min(a.y2, b.y2) - std::max(a.y1, b.y1);

    return inter_width * inter_height;
}

bool scoreSort(TargetBox a, TargetBox b) 
{ 
    return (a.score > b.score); 
}

//NMS deal 
int NMSHandle(std::vector<TargetBox> &tmpBoxes, 
                             std::vector<TargetBox> &dstBoxes)
{
    std::vector<int> picked;
    
    sort(tmpBoxes.begin(), tmpBoxes.end(), scoreSort);

    for (int i = 0; i < tmpBoxes.size(); i++) {
        int keep = 1;
        for (int j = 0; j < picked.size(); j++) {
            //交集
            float inter_area = intersection_area(tmpBoxes[i], tmpBoxes[picked[j]]);
            //并集
            float union_area = tmpBoxes[i].area() + tmpBoxes[picked[j]].area() - inter_area;
            float IoU = inter_area / union_area;

            if(IoU > nmsThresh && tmpBoxes[i].cate == tmpBoxes[picked[j]].cate) {
                keep = 0;
                break;
            }
        }

        if (keep) {
            picked.push_back(i);
        }
    }
    
    for (int i = 0; i < picked.size(); i++) {
        dstBoxes.push_back(tmpBoxes[picked[i]]);
    }

    return 0;
}

void hwc_to_chw(cv::InputArray &src, cv::OutputArray &dst) {
    const int src_h = src.rows();
    const int src_w = src.cols();
    const int src_c = src.channels();

    cv::Mat hw_c = src.getMat().reshape(1, src_h * src_w);

    const std::array<int,3> dims = {src_c, src_h, src_w};                         
    dst.create(3, &dims[0], CV_MAKETYPE(src.depth(), 1));                         
    cv::Mat dst_1d = dst.getMat().reshape(1, {src_c, src_h, src_w});              

    cv::transpose(hw_c, dst_1d);                                                  
} 

/*
 * change mat shape from hwc to chw
 */
int HWC2CHW(float *src, float *dst, int height, int width, int channel)
{
    size_t h = 0, w = 0, c = 0, count = 0;

    // for(c = 0; c < channel; c++) {
    //     for(w = 0; w < width; w++) {
    //         for(h = 0; h < height; h++) {
    //             printf("%d,", src[h+w*height+c*height*width]);
    //         }
    //         printf("\n");
    //     }
    //     printf("-------------------------------------\n");
    // }
    // printf("****************************************\n");
    for(h = 0; h < height; h++) {
        for(w = 0; w < width; w++) {
            for(c = 0; c < channel; c++) {
                dst[count] = src[h+w*height+c*height*width];
                std::cout << src[h+w*height+c*height*width];
                count++;
            }
            printf("\n");
        }
        printf("-------------------------------------\n");
    }
    return 0;
}

void DrawBoxes(cv::Mat &src, std::vector<TargetBox> boxes, cv::Scalar color)
{
    for(auto box : boxes) {
        cv::rectangle(src, cv::Rect(cv::Point(box.x1, box.y1), cv::Point(box.x2, box.y2)), cv::Scalar(0, 255, 0));
    }
}

std::vector<TargetBox> 
DecodeInfer(uint8_t *out_data, int stride, int input_height, int input_width, 
            int out_height, int out_width, int out_channel, int img_height, 
            int img_width, int num_classes, std::vector<float> anchors, 
            float threshold)
{
    std::vector<TargetBox> result;
    int grid_size_y = int(sqrt(out_height));
    int grid_size_x = int(sqrt(out_width));

    uint8_t *mat_data[out_channel];
    for (int i = 0; i < out_channel; i++) {
        mat_data[i] = out_data + i*(out_channel*out_width);
    }
    float cx, cy, w, h;
    for (int shift_y = 0; shift_y < grid_size_y; shift_y++) {
        for (int shift_x = 0; shift_x < grid_size_x; shift_x++) {
            // int loc = shift_x + shift_y * grid_size;
            for (int i = 0; i < 3; i++) {
                uint8_t *record = mat_data[i];
                uint8_t *cls_ptr = record + 5;
                for (int cls = 0; cls < num_classes; cls++) {
                    float score = sigmoid(cls_ptr[cls]) * sigmoid(record[4]);
                    if (score > threshold) {
                        cx = (sigmoid((float)record[0]) * 2.f - 0.5f + (float) shift_x) * (float) stride;
                        cy = (sigmoid((float)record[1]) * 2.f - 0.5f + (float) shift_y) * (float) stride;
                        w = pow(sigmoid((float)record[2]) * 2.f, 2) * anchors[i*2+0];
                        h = pow(sigmoid((float)record[3]) * 2.f, 2) * anchors[i*2+1];
                        printf("[grid size=%d, stride = %d]x y w h %d %d %d %d\n",grid_size_x,stride,record[0],record[1],record[2],record[3]);
                        TargetBox box;
                        box.x1 = std::max(0, std::min(img_width, int((cx - w / 2.f) * (float) img_width / (float) input_width)));
                        box.y1 = std::max(0, std::min(img_height, int((cy - h / 2.f) * (float) img_height / (float) input_height)));
                        box.x2 = std::max(0, std::min(img_width, int((cx + w / 2.f) * (float) img_width / (float) input_width)));
                        box.y2 = std::max(0, std::min(img_height, int((cy + h / 2.f) * (float) img_height / (float) input_height)));
                        box.score = score;
                        box.cate = cls;
                        result.push_back(box);
                    }
                }
            }
            for (auto &ptr:mat_data) {
                ptr += (num_classes + 5);
            }
        }
    }
    return result;
}

void nms(std::vector<TargetBox> &input_boxes, float NMS_THRESH) {
    std::sort(input_boxes.begin(), input_boxes.end(), [](TargetBox a, TargetBox b) { return a.score > b.score; });
    std::vector<float> vArea(input_boxes.size());
    for (int i = 0; i < int(input_boxes.size()); ++i) {
        vArea[i] = (input_boxes.at(i).x2 - input_boxes.at(i).x1 + 1)
                   * (input_boxes.at(i).y2 - input_boxes.at(i).y1 + 1);
    }
    for (int i = 0; i < int(input_boxes.size()); ++i) {
        for (int j = i + 1; j < int(input_boxes.size());) {
            float xx1 = std::max(input_boxes[i].x1, input_boxes[j].x1);
            float yy1 = std::max(input_boxes[i].y1, input_boxes[j].y1);
            float xx2 = std::min(input_boxes[i].x2, input_boxes[j].x2);
            float yy2 = std::min(input_boxes[i].y2, input_boxes[j].y2);
            float w = std::max(float(0), xx2 - xx1 + 1);
            float h = std::max(float(0), yy2 - yy1 + 1);
            float inter = w * h;
            float ovr = inter / (vArea[i] + vArea[j] - inter);
            if (ovr >= NMS_THRESH) {
                input_boxes.erase(input_boxes.begin() + j);
                vArea.erase(vArea.begin() + j);
            } else {
                j++;
            }
        }
    }
}

}   //namespace krlprutils
}   //namespace krlpr
