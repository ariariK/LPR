#include "inference/l_detect.h"
#include "inference/utils.h"

#if __has_include(<format>)
#include <format>
#endif
#include <sstream>
#include <iostream>
#include <fstream>
#include <cstring>

#define TFLITE              0
#define OPENCV_DNN          1
#define USE_FRAMEWORK       OPENCV_DNN

namespace krlpr {

l_detect::l_detect(std::string model_path)
{
    //init data
    verbose = false;
    number_of_threads = 1;
    //Load model
    model = tflite::FlatBufferModel::BuildFromFile(model_path.c_str());
    LPR_CHECK(model != nullptr);
    // Build the interpreter with the InterpreterBuilder.
    // Note: all Interpreters should be built with the InterpreterBuilder,
    // which allocates memory for the Intrepter and does various set up
    // tasks so that the Interpreter can read the provided model.
    tflite::ops::builtin::BuiltinOpResolver resolver;
    tflite::InterpreterBuilder builder(*model, resolver);
    builder(&interpreter);
    LPR_CHECK(interpreter != nullptr);
    // Set number of threads
    interpreter->SetNumThreads(number_of_threads);
    // Set allow fp32
    // interpreter->SetAllowFp16PrecisionForFp32(true);
    // Allocate tensor buffers.
    LPR_CHECK(interpreter->AllocateTensors() == kTfLiteOk);
    if(verbose) {
        printf("=== Pre-invoke Interpreter State ===\n");
        // tflite::PrintInterpreterState(interpreter.get());
    }
    const std::vector<int> inputs = interpreter->inputs();
    const std::vector<int> outputs = interpreter->outputs();

    if (verbose) {
        LOG(INFO) << "input index" << interpreter->inputs()[0] << std::endl;
        LOG(INFO) << "number of inputs: " << inputs.size() << "\n";
        LOG(INFO) << "number of outputs: " << outputs.size() << "\n";
    }
    this->objThreshold = 0.4;
	this->confThreshold = 0.6;
	this->nmsThreshold = 0.1;
    this->num_class = 2;
}

l_detect::l_detect(std::string model_path, float obj_Threshold, float conf_Threshold, float nms_Threshold)
{
    this->objThreshold = obj_Threshold;
	this->confThreshold = conf_Threshold;
	this->nmsThreshold = nms_Threshold;

	// std::ifstream ifs(this->classesFile.c_str());
	// std::string line;
	// while (std::getline(ifs, line)) this->classes.push_back(line);
	// this->num_class = this->classes.size();
    this->num_class = 2;
	this->net = cv::dnn::readNet(model_path);
}

l_detect::~l_detect()
{
}

void l_detect::drawPred(int classId, float conf, int left, int top, int right, int bottom, cv::Mat& frame)   // Draw the predicted bounding box
{
	//Draw a rectangle displaying the bounding box
	cv::rectangle(frame, cv::Point(left, top), cv::Point(right, bottom), cv::Scalar(0, 0, 255), 3);

	//Get the label for the class name and its confidence
    #ifdef __cpp_lib_format
        // Code with std::format
        // std::string label = std::format("%.2f", conf);
    #else
    // Code without std::format, or just #error if you only
    // want to support compilers and standard libraries with std::format
    #endif
	// label = this->classes[classId] + ":" + label;

	//Display the label at the top of the bounding box
	int baseLine;
	// cv::Size labelSize = cv::getTextSize(label, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);
	// top = std::max(top, labelSize.height);
	//rectangle(frame, Point(left, top - int(1.5 * labelSize.height)), Point(left + int(1.5 * labelSize.width), top + baseLine), Scalar(0, 255, 0), FILLED);
	// cv::putText(frame, label, cv::Point(left, top), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 255, 0), 1);
}
typedef cv::Point3_<float> Pixel;
void normalize(Pixel &pixel){
    pixel.x = (pixel.x / 255.0);
    pixel.y = (pixel.y / 255.0);
    pixel.z = (pixel.z / 255.0);
}

auto matPreprocess(cv::Mat src, uint width, uint height) -> cv::Mat{
    // convert to float; BGR -> RGB
    cv::Mat dst;
    src.convertTo(dst, CV_32FC3);
    // cv::cvtColor(dst, dst, cv::COLOR_BGR2RGB);

    // normalize to -1 & 1
    // Pixel* pixel = dst.ptr<Pixel>(0,0);
    // const Pixel* endPixel = pixel + dst.cols * dst.rows;
    // for (; pixel != endPixel; pixel++)
    //     normalize(*pixel);

    // resize image as model input
    cv::resize(dst, dst, cv::Size(width, height));
    cv::Mat a;
    krlprutils::hwc_to_chw(dst, a);
    return a;
}

template<typename T>
auto cvtTensor(TfLiteTensor* tensor) -> std::vector<T>;

auto cvtTensor(TfLiteTensor* tensor) -> std::vector<float>{
    int nelem = 1;
    for(int i=0; i<tensor->dims->size; ++i)
        nelem *= tensor->dims->data[i];
    std::vector<float> data(tensor->data.f, tensor->data.f+nelem);
    return data;
}

void l_detect::run_inference(cv::Mat& src, std::vector<krlprutils::TargetBox>& res)
{
    #if USE_FRAMEWORK == TFLITE
        // TfLiteTensor* input_tensor = interpreter->tensor(interpreter->inputs()[0]);
        // TfLiteTensor* output_box = interpreter->tensor(interpreter->outputs()[0]);
        // TfLiteTensor* output_score = interpreter->tensor(interpreter->outputs()[1]);

        // const uint HEIGHT = input_tensor->dims->data[2];
        // const uint WIDTH = input_tensor->dims->data[3];
        // const uint CHANNEL = input_tensor->dims->data[4];
        // std::cout << "weight" << WIDTH << "height" << HEIGHT << std::endl;
        // // read image file
        // cv::Mat img = src;
        // cv::Mat inputImg = matPreprocess(img, WIDTH, HEIGHT);

        // // flatten rgb image to input layer.
        // float* inputImg_ptr = inputImg.ptr<float>(0);
        // memcpy(input_tensor->data.f, inputImg.ptr<float>(0),
        //     WIDTH * HEIGHT * CHANNEL * sizeof(float));

        // // compute model instance
        // // std::chrono::time_point<std::chrono::system_clock> start, end;
        // // std::chrono::duration<double> elapsed_seconds;
        // // start = std::chrono::system_clock::now();
        // interpreter->Invoke();
        // // end = std::chrono::system_clock::now();
        // // elapsed_seconds = end - start;
        // // printf("s: %.10f\n" ,elapsed_seconds.count());

        // std::vector<float> box_vec = cvtTensor(output_box);
        // std::vector<float> score_vec = cvtTensor(output_score);

        // std::vector<size_t> result_id;
        // auto it = std::find_if(std::begin(score_vec), std::end(score_vec),
        //                     [](float i){return i > 0.2;});
        // while (it != std::end(score_vec)) {
        //     result_id.emplace_back(std::distance(std::begin(score_vec), it));
        //     it = std::find_if(std::next(it), std::end(score_vec),
        //                     [](float i){return i > 0.2;});
        // }

        // std::vector<cv::Rect> rects;
        // std::vector<float> scores;
        // for(size_t tmp:result_id){
        //     const int cx = box_vec[4*tmp];
        //     const int cy = box_vec[4*tmp+1];
        //     const int w = box_vec[4*tmp+2];
        //     const int h = box_vec[4*tmp+3];
        //     const int xmin = ((cx-(w/2.f))/WIDTH) * img.cols;
        //     const int ymin = ((cy-(h/2.f))/HEIGHT) * img.rows;
        //     const int xmax = ((cx+(w/2.f))/WIDTH) * img.cols;
        //     const int ymax = ((cy+(h/2.f))/HEIGHT) * img.rows;
        //     rects.emplace_back(cv::Rect(xmin, ymin, xmax-xmin, ymax-ymin));
        //     scores.emplace_back(score_vec[tmp]);
        // }

        // std::vector<int> ids;
        // cv::dnn::NMSBoxes(rects, scores, 0.2, 0.2, ids);
        // for(int tmp: ids)
        //     cv::rectangle(img, rects[tmp], cv::Scalar(0, 255, 0), 3);
        // std::cout << rects[ids[0]] << std::endl;
        // cv::imwrite("output.jpg", img);
        // if(verbose) {
        //     std::cout << "l_img size: " << l_imgs.size() << std::endl;
        // }
        // if(l_imgs.size() == 0) {
        //     return res;
        // }
        // Fill input buffers
        // TODO use multi thread to fill input buffers
        if(verbose) {
            std::cout << "start fill input buffers" << std::endl;
        }
        // auto size = l_imgs.size();
        // for(auto i = 0; i < size; i++) {
        //     uint8_t *input = interpreter->typed_input_tensor<uint8_t>(i);
        //     memcpy(input, l_imgs[i].data, 24*94);
        // }
        // int input_index = this->interpreter->inputs()[0];       //signal input fill
        // uint8_t *input = this->interpreter->typed_input_tensor<uint8_t>(input_index);
        // memcpy(input, l_imgs[0].ptr<uint8_t>(0), 24*94);
        int input_index = this->interpreter->inputs()[0];       //signal input fill
        float *input = this->interpreter->typed_input_tensor<float>(input_index);
        // memcpy(input, (float*)img.data, 3*480*640*sizeof(float));
        cv::Mat img = src;
        cv::Mat inputImg = matPreprocess(img, 640, 480);
        // cv::imwrite("output1.jpg", inputImg);
        // flatten rgb image to input layer.
        float* inputImg_ptr = inputImg.ptr<float>(0);
        memcpy(input, inputImg.ptr<float>(0), 3*480*640*sizeof(float));
        std::cout << inputImg.size() << std::endl;
        // memset(input, 0, 3*480*640*sizeof(float));
        // Run inference
        if(verbose) {
            std::cout << "start run inference" << std::endl;
        }
        LPR_CHECK(interpreter->Invoke() == kTfLiteOk);
        if(verbose) {
            printf("\n\n=== Post-invoke Interpreter State ===\n");
            // tflite::PrintInterpreterState(interpreter.get());
        }
        // Read output buffers(1,30,40,17)
        float *outputs[2] = {nullptr, nullptr};
        // outputs[0] = (float *)malloc(1*30*40*17*sizeof(float));
        // outputs[1] = (float *)malloc(1*20*15*17*sizeof(float));
        // krlprutils::HWC2CHW(interpreter->typed_output_tensor<float>(0), outputs[0], 30, 40, 17);
        // krlprutils::HWC2CHW(interpreter->typed_output_tensor<float>(1), outputs[1], 15, 20, 17);
        outputs[0] = interpreter->typed_output_tensor<float>(0);           //get output 1
        // outputs[1] = interpreter->typed_output_tensor<float>(1);           //get output 2
        // for (int j = 0; j < 10; j++) {
        //     for (int i = 0; i < 17; i++) {
        //         std::cout << (outputs[0])[i+j*17] << ",";
        //     }
        //     std::cout << std::endl;
        // }
        // cv::Mat a(1200, 17, CV_32FC1, outputs[0]);
        // cv::Mat b(300, 17, CV_32FC1, outputs[1]);
        // std::cout << a.size() << std::endl;
        // std::cout << a.channels() << std::endl;
        // std::cout << a.type() << std::endl;
        // std::cout << b.size() << std::endl;
        // std::cout << b.channels() << std::endl;
        // std::cout << b.type() << std::endl;
        // std::vector<cv::Mat> outs;
        // cv::Mat concat_mat;
        // cv::vconcat(a, b, concat_mat);
        // // cv::threshold(-concat_mat, concat_mat, 0, 0, cv::THRESH_TRUNC);
        // // concat_mat = -concat_mat;
        // outs.push_back(concat_mat);
        cv::Mat concat_mat(1500, 17, CV_32FC1, outputs[0]);
        std::cout << concat_mat.size() << std::endl;
        for( size_t nrow = 0; nrow < 2; nrow++) {  
            for(size_t ncol = 0; ncol < concat_mat.cols; ncol++) {
                std::cout << concat_mat.at<float>(nrow, ncol) << ",";
            }
            std::cout << std::endl;
        }
        /////generate proposals
        std::vector<int> classIds, classIds_1;
        std::vector<float> confidences, confidences_1;
        std::vector<cv::Rect> boxes, boxes_1;
        float ratioh = (float)src.rows / this->inpHeight, ratiow = (float)src.cols / this->inpWidth;
        int n = 0, q = 0, i = 0, j = 0, nout = this->anchor_num * 5 + this->num_class, row_ind = 0;
        float* pdata = (float*)concat_mat.ptr<float>(0);
        for (n = 0; n < this->num_stage; n++)   ///stage
        {
            int num_grid_x = (int)(this->inpWidth / this->stride[n]);
            int num_grid_y = (int)(this->inpHeight / this->stride[n]);
            for (i = 0; i < num_grid_y; i++)
            {
                for (j = 0; j < num_grid_x; j++)
                {
                    cv::Mat scores = concat_mat.row(row_ind).colRange(this->anchor_num * 5, concat_mat.cols);
                    cv::Point classIdPoint;
                    double max_class_socre;
                    // Get the value and location of the maximum score
                    cv::minMaxLoc(scores, 0, &max_class_socre, 0, &classIdPoint);
                    for (q = 0; q < this->anchor_num; q++)    ///anchor
                    {
                        const float anchor_w = this->anchors[n][q * 2];
                        const float anchor_h = this->anchors[n][q * 2 + 1];
                        float box_score = pdata[4 * this->anchor_num + q];
                        if (box_score > this->objThreshold && max_class_socre > this->confThreshold)
                        {
                            float cx = (pdata[4 * q] * 2.f - 0.5f + j) * this->stride[n];  ///cx
                            float cy = (pdata[4 * q+ 1] * 2.f - 0.5f + i) * this->stride[n];   ///cy
                            float w = powf(pdata[4 * q + 2] * 2.f, 2.f) * anchor_w;   ///w
                            float h = powf(pdata[4 * q + 3] * 2.f, 2.f) * anchor_h;  ///h
                            int left = (cx - 0.5*w)*ratiow;
                            int top = (cy - 0.5*h)*ratioh;   ///锟斤拷锟疥还原锟斤拷原图锟斤拷
                            classIds.push_back(classIdPoint.x);
                            confidences.push_back(box_score * max_class_socre);
                            boxes.push_back(cv::Rect(left, top, (int)(w*ratiow), (int)(h*ratioh)));
                        }
                    }
                    row_ind++;
                    pdata += nout;
                }
            }
        }

        // Perform non maximum suppression to eliminate redundant overlapping boxes with
        // lower confidences
        // std::cout << "NMS" << std::endl;
        // for(auto r : boxes) {
        //     cv::rectangle(src, r, cv::Scalar(0, 0, 255), 3);
        // }
        // cv::imwrite("output.jpg",src);
        std::vector<int> indices;
        // for(int i = 0; i < boxes.size(); i++) {
        //     if(0 <= boxes[i].x && 0 <= boxes[i].width && boxes[i].x + boxes[i].width <= src.cols && 0 <= boxes[i].y && 0 <= boxes[i].height && boxes[i].y + boxes[i].height <= src.rows) {
        //         boxes_1.push_back(boxes[i]);
        //         // classIds_1.push_back(classIds[i]);
        //         confidences_1.push_back(confidences[i]);   
        //     }
        // }
        // std::vector<krlpr::krlprutils::TargetBox> nms_boxes, rb;
        // for(int i = 0; i < boxes.size(); i++) {
        //     krlprutils::TargetBox tb;
        //     tb.x1 = boxes[i].x;
        //     tb.y1 = boxes[i].y;
        //     tb.x2 = boxes[i].x + boxes[i].width;
        //     tb.y2 = boxes[i].y + boxes[i].height;
        //     tb.score = confidences[i];
        //     std::cout << tb.score << std::endl;
        //     if (tb.score > 0.45)
        //         nms_boxes.push_back(tb);
        // }
        // krlprutils::NMSHandle(nms_boxes, rb);
        // std::cout << nms_boxes.size() << std::endl;
        // for(auto b : rb) {
        //     cv::rectangle(src, cv::Point(b.x1, b.y1), cv::Point(b.x2, b.y2), cv::Scalar(0, 0, 255), 3);
        // }
        // cv::imwrite("output.jpg", src);
        cv::dnn::NMSBoxes(boxes, confidences, this->confThreshold, this->nmsThreshold, indices);
        std::cout << indices.size() << std::endl;
        for (size_t i = 0; i < indices.size(); ++i)
        {
            int idx = indices[i];
            cv::Rect box = boxes[idx];
            krlprutils::TargetBox t_box;
            t_box.x1 = box.x;
            t_box.y1 = box.y;
            t_box.x2 = box.x + box.width;
            t_box.y2 = box.y + box.height;
            // t_box.lpr_src = cv::Mat(src, box);
            res.push_back(t_box);
            cv::rectangle(src, box, cv::Scalar(0, 0, 255), 3);
            std::cout << box << std::endl;
            // this->drawPred(classIds[idx], confidences[idx], box.x, box.y,
            //     box.x + box.width, box.y + box.height, src);
        }
        cv::imwrite("output.jpg", src);
        // std::cout << interpreter->outputs().size() << std::endl;
        // std::cout << interpreter->output_tensor(1)->name << std::endl;
        // output tensor result
        // for(int i = 0; i < 30; i++) {
        //     for(int j = 0; j < 40; j++) {
        //         for(int k = 0; k < 17; k++) {
        //             std::cout << (outputs[0])[i*40*17+j*17+k] << ",";
        //         }
        //         std::cout << std::endl;
        //     }
        // }
        // for(int i = 0, j = 0; i < 30*40*17; i++) {
        //     std::cout << (outputs[0])[i] << ",";
        //     if(j == 16) {
        //         std::cout << std::endl;
        //         j = 0;
        //     } else {
        //         j++;
        //     }
        // }
        // preHandle result
        // std::vector<krlprutils::TargetBox> dstBoxes, rstBoxes;
        // krlprutils::PredHandle(outputs, dstBoxes, (float)src.cols / 640.0, (float)src.rows / 480.0, 0.4);
        // krlprutils::NMSHandle(dstBoxes, rstBoxes);
        // std::cout << rstBoxes.size() << std::endl;
        // for(auto b : rstBoxes) {
        //     cv::rectangle(src, cv::Point(b.x1, b.y1), cv::Point(b.x2, b.y2), cv::Scalar(0, 0, 255), 3);
        // }
        // cv::imwrite("output.jpg", src);
        // // std::vector<float> anchors {26.13,9.58, 46.67,18.37, 77.69,25.90, 84.59,46.55, 133.05,30.85, 176.99,64.33};
        // // std::vector<krlprutils::TargetBox> result = krlprutils::DecodeInfer(outputs[0], 640/40, 480, 640, 30, 40, 17, 221, 295, 2, anchors, 0.25);
        // // krlprutils::nms(result, 0.25);
        // // std::cout << result.size() << std::endl;
        // auto max_box = rstBoxes[0];
        // for(auto box : rstBoxes) {
        //     std::cout << "x1:" << box.x1 << " y1:" << box.y1 << " x2:" << box.x2 << " y2:" << box.y2 << box.score << std::endl;
        //     if(box.score > max_box.score) {
        //         max_box = box;
        //     }
        // }
        // std::cout << "max score: x1:" << max_box.x1 << " y1:" << max_box.y1 << " x2:" << max_box.x2 << " y2:" << max_box.y2 << max_box.score << std::endl;
        // DrawBoxes(src);
    #endif
    #if USE_FRAMEWORK == OPENCV_DNN
        cv::Mat blob;
        cv::dnn::blobFromImage(src, blob, 1 / 255.0, cv::Size(this->inpWidth, this->inpHeight));
        this->net.setInput(blob);
        std::vector<cv::Mat> net_outs, outs;
        // /* clock statistics running time */
        // clock_t start, end;
        // // clock start
        // start = clock();
        this->net.forward(net_outs, this->net.getUnconnectedOutLayersNames());
        // clock end
        // end = clock();
        // double seconds  =(double)(end - start)/CLOCKS_PER_SEC;
        // fprintf(stderr, "Use time is: %.8f\n", seconds);
        net_outs[0] = net_outs[0].reshape(1, (int)net_outs[0].total() / 17);
        net_outs[1] = net_outs[1].reshape(1, (int)net_outs[1].total() / 17);
        // cout << net_outs[0].size() << endl;
        // cout << net_outs[1].size() << endl;
        cv::Mat concat_mat;
        cv::vconcat(net_outs[0], net_outs[1], concat_mat);
        // cout << concat_mat.size() << endl;
        // for( size_t nrow = 0; nrow < concat_mat.rows; nrow++) {  
        //     for(size_t ncol = 0; ncol < concat_mat.cols; ncol++) {
        //         std::cout << concat_mat.at<float>(nrow, ncol) << ",";
        //     }
        //     break;
        // }
        outs.push_back(concat_mat);
        // cout << outs[0].cols << endl;
        /////generate proposals
        std::vector<int> classIds;
        std::vector<float> confidences;
        std::vector<cv::Rect> boxes;
        float ratioh = (float)src.rows / this->inpHeight, ratiow = (float)src.cols / this->inpWidth;
        int n = 0, q = 0, i = 0, j = 0, nout = this->anchor_num * 5 + this->num_class, row_ind = 0;
        float* pdata = (float*)outs[0].data;
        for (n = 0; n < this->num_stage; n++)   ///stage
        {
            int num_grid_x = (int)(this->inpWidth / this->stride[n]);
            int num_grid_y = (int)(this->inpHeight / this->stride[n]);
            for (i = 0; i < num_grid_y; i++)
            {
                for (j = 0; j < num_grid_x; j++)
                {
                    cv::Mat scores = outs[0].row(row_ind).colRange(this->anchor_num * 5, outs[0].cols);
                    cv::Point classIdPoint;
                    double max_class_socre;
                    // Get the value and location of the maximum score
                    cv::minMaxLoc(scores, 0, &max_class_socre, 0, &classIdPoint);
                    for (q = 0; q < this->anchor_num; q++)    ///anchor
                    {
                        const float anchor_w = this->anchors[n][q * 2];
                        const float anchor_h = this->anchors[n][q * 2 + 1];
                        float box_score = pdata[4 * this->anchor_num + q];
                        if (box_score > this->objThreshold && max_class_socre > this->confThreshold)
                        {
                            float cx = (pdata[4 * q] * 2.f - 0.5f + j) * this->stride[n];  ///cx
                            float cy = (pdata[4 * q+ 1] * 2.f - 0.5f + i) * this->stride[n];   ///cy
                            float w = powf(pdata[4 * q + 2] * 2.f, 2.f) * anchor_w;   ///w
                            float h = powf(pdata[4 * q + 3] * 2.f, 2.f) * anchor_h;  ///h

                            int left = (cx - 0.5*w)*ratiow;
                            int top = (cy - 0.5*h)*ratioh;   ///锟斤拷锟疥还原锟斤拷原图锟斤拷

                            classIds.push_back(classIdPoint.x);
                            confidences.push_back(box_score * max_class_socre);
                            boxes.push_back(cv::Rect(left, top, (int)(w*ratiow), (int)(h*ratioh)));
                        }
                    }
                    row_ind++;
                    pdata += nout;
                }
            }
        }

        // Perform non maximum suppression to eliminate redundant overlapping boxes with
        // lower confidences
        std::vector<int> indices;
        cv::dnn::NMSBoxes(boxes, confidences, this->confThreshold, this->nmsThreshold, indices);
        // cout << indices.size() << endl;
        for (size_t i = 0; i < indices.size(); ++i)
        {
            int idx = indices[i];
            cv::Rect box = boxes[idx];
            krlprutils::TargetBox t_box;
            t_box.x1 = box.x;
            t_box.y1 = box.y;
            t_box.x2 = box.x + box.width;
            t_box.y2 = box.y + box.height;
            t_box.lpr_src = cv::Mat(src, box);
            res.push_back(t_box);
            // this->drawPred(classIds[idx], confidences[idx], box.x, box.y,
            //     box.x + box.width, box.y + box.height, src);
        }
        // cv::imwrite("output.jpg",src);
    #endif
}

}   //namespace krlpr
