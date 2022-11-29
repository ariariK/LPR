#include "inference_ncnn/l_ocr.h"
#include "inference_ncnn/utils.h"

#include <cstring>

namespace krlpr {

    l_ocr::l_ocr(std::string model_path)
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
        // Allocate tensor buffers.
        LPR_CHECK(interpreter->AllocateTensors() == kTfLiteOk);
        if(verbose) {
            printf("=== Pre-invoke Interpreter State ===\n");
            // tflite::PrintInterpreterState(interpreter.get());
        }
    }

    l_ocr::~l_ocr()
    {
    }

    std::vector<std::string> l_ocr::run_inference(std::vector<cv::Mat> l_imgs)
    {
        std::vector<std::string> res;

        if(verbose) {
            std::cout << "l_img size: " << l_imgs.size() << std::endl;
        }
        if(l_imgs.size() == 0) {
            return res;
        }
        // Fill input buffers
        // TODO use multi thread to fill input buffers
        if(verbose) {
            std::cout << "start fill input buffers" << std::endl;
        }
        // auto size = l_imgs.size();
        // int input_index = this->interpreter->inputs()[0];
        // uint8_t *input = interpreter->typed_input_tensor<uint8_t>(input_index);
        // for(auto i = 0; i < size; i++) {
        //     input += 24*94*i;
        //     memcpy(input, l_imgs[i].ptr<uint8_t>(0), 24*94);
        // }
        int input_index = this->interpreter->inputs()[0];       //signal input fill
        uint8_t *input = this->interpreter->typed_input_tensor<uint8_t>(input_index);
        memcpy(input, l_imgs[0].ptr<uint8_t>(0), 24*94);
        // Run inference
        if(verbose) {
            std::cout << "start run inference" << std::endl;
        }
        LPR_CHECK(interpreter->Invoke() == kTfLiteOk);
        if(verbose) {
            printf("\n\n=== Post-invoke Interpreter State ===\n");
            // tflite::PrintInterpreterState(interpreter.get());
        }
        // Read output buffers
        // uint8_t *output = interpreter->typed_output_tensor<uint8_t>(0);
        // for(auto i = 0; i < size; i++) {    //multi outputs
        //     output += 24*68*i;
        //     res.push_back(krlprutils::CTCGready(output));
        // }
        uint8_t *output = interpreter->typed_output_tensor<uint8_t>(0);     //signal output
        res.push_back(krlprutils::CTCGready(output));

        return res;
    }

    void l_ocr::run_inference(std::vector<krlprutils::TargetBox> &boxes, cv::Mat& src)
    {
        // const int batch_size = 5;
        // int boxes_size = boxes.size();
        // int input_index = this->interpreter->inputs()[0];                               // input
        // uint8_t *input = this->interpreter->typed_input_tensor<uint8_t>(input_index);
        // // get lp string
        // int size = boxes_size/batch_size;
        // if(boxes_size%batch_size) {
        //     size++;
        // }
        // for(int i = 0; i < size; i++) {
        //     int cur_batch = boxes_size - i * batch_size;
        //     // fill inputs
        //     for(int j = 0; j < cur_batch; j++) {
        //         cv::Mat gray;
        //         cv::cvtColor(boxes[j + i * batch_size].lpr_src, gray, cv::COLOR_BGR2GRAY);
        //         cv::Mat gray_resize;
        //         cv::resize(gray, gray_resize, cv::Size(94, 24));
        //         memcpy(input + j*94*24, gray_resize.ptr<uint8_t>(0), 24*94);
        //     }
        //     if(cur_batch < 5) {
        //         memset(input + cur_batch*94*24, 0, (5 - cur_batch)*94*24);
        //     }
        //     // run
        //     LPR_CHECK(interpreter->Invoke() == kTfLiteOk);
        //     uint8_t *output = interpreter->typed_output_tensor<uint8_t>(0);                 // output
        //     // decode and get result
        //     for(int j = 0; j < cur_batch; j++) {
        //         int index = j + i * batch_size;
        //         boxes[index].lpr_string = krlprutils::CTCGready(output + j*68*24);
        //         std::cout << boxes[index].lpr_string << std::endl;
        //         if(krlprutils::LicenseMatch(boxes[index].lpr_string)) {
        //             cv::rectangle(src, cv::Point(boxes[index].x1, boxes[index].y1), cv::Point(boxes[index].x2, boxes[index].y2), cv::Scalar(0, 0, 255), 3);
        //             int baseLine;
        //             cv::Size labelSize = cv::getTextSize(boxes[index].lpr_string, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);
        //             int top = std::max(top, labelSize.height);
        //             cv::putText(src, boxes[index].lpr_string, cv::Point(boxes[index].x1, boxes[index].y1), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 255, 0), 1);
        //         } else {
        //             boxes.erase(boxes.begin() + index);     // delete error lp
        //         }
        //     }
        // }
        for(int i = 0; i < boxes.size(); i++) {
            cv::Mat gray;
            cv::cvtColor(boxes[i].lpr_src, gray, cv::COLOR_BGR2GRAY);
            cv::Mat gray_resize;
            cv::resize(gray, gray_resize, cv::Size(94, 32));
            int input_index = this->interpreter->inputs()[1];       //signal input fill
            uint8_t *input = this->interpreter->typed_input_tensor<uint8_t>(input_index);
            memcpy(input, gray_resize.ptr<uint8_t>(0), 32*94);
            // one line or double line
            input_index = this->interpreter->inputs()[0];
            bool *is_double;
            is_double = this->interpreter->typed_input_tensor< bool >(input_index);
            if(boxes[i].cate) {
                *is_double = true;
            } else {
                *is_double = false;
            }
            // Run inference
            if(verbose) {
                std::cout << "start run inference" << std::endl;
            }
            LPR_CHECK(interpreter->Invoke() == kTfLiteOk);
            if(verbose) {
                printf("\n\n=== Post-invoke Interpreter State ===\n");
                // tflite::PrintInterpreterState(interpreter.get());
            }
            // Read output buffers
            // uint8_t *output = interpreter->typed_output_tensor<uint8_t>(0);
            // for(auto i = 0; i < size; i++) {    //multi outputs
            //     output += 24*68*i;
            //     res.push_back(krlprutils::CTCGready(output));
            // }
            uint8_t *output = interpreter->typed_output_tensor<uint8_t>(0);     //signal output
            boxes[i].lpr_string = krlprutils::CTCGready(output);
            if(krlprutils::LicenseMatch(boxes[i].lpr_string)) {
            #if true   // add. by ariari : 2022.05.20 - begin
                // move to preview
                cv::rectangle(src, cv::Point(boxes[i].x1, boxes[i].y1), cv::Point(boxes[i].x2, boxes[i].y2), cv::Scalar(0, 0, 255), 3);
                int baseLine;
                cv::Size labelSize = cv::getTextSize(boxes[i].lpr_string, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);
                int top = std::max(top, labelSize.height);
                // rem. by ariari : 2022.11.22 - 한글깨짐
                //cv::putText(src, boxes[i].lpr_string, cv::Point(boxes[i].x1, boxes[i].y1), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 255, 0), 1);
            #endif  // add. by ariari : 2022.05.20 - end
            } else {
                boxes.erase(boxes.begin() + i);     // delete error lp
            }
            // uint8_t *output = interpreter->typed_output_tensor<uint8_t>(0);     //signal output
            // boxes[i].lpr_string = krlprutils::CTCGready(output);
            // cv::rectangle(src, cv::Point(boxes[i].x1, boxes[i].y1), cv::Point(boxes[i].x2, boxes[i].y2), cv::Scalar(0, 0, 255), 3);
            // int baseLine;
            // cv::Size labelSize = cv::getTextSize(boxes[i].lpr_string, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);
            // int top = std::max(top, labelSize.height);
            // cv::putText(src, boxes[i].lpr_string, cv::Point(boxes[i].x1, boxes[i].y1), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 255, 0), 1);
        }

    }

}   //namespace krlpr
