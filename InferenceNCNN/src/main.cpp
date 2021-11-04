#include "inference_ncnn/l_ocr.h"
#include "inference_ncnn/l_detect.h"
#include "inference_ncnn/utils.h"
#include "ipcs.h"

#include "opencv2/opencv.hpp"
#include <time.h>
#include <cstdio>
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <unistd.h>
#include <string>

#define __USE_GNU
#include <sched.h>
#include <ctype.h>
#include <string.h>
#include <pthread.h>
#include <queue>
#include <sys/time.h>

#define THREAD_MAX_NUM                                                      3

pthread_mutex_t mutex;                                                      // pthread_mutex_t for thread save(frame_queue, boxes_queue)
krlpr::l_ocr 	*lpr_ocr;                                                   // lpr_ocr class
krlpr::l_detect *lpr_detect;                                                // lpr_detect class
std::queue< std::vector<krlpr::krlprutils::TargetBox> > boxes_queue;        // a queue transfer lpr_detect result from l_detect thread to l_ocr thread
std::queue<cv::Mat> frame_queue;                                            // a queue transfer read frame from l_detect thread to l_ocr thread
int is_running;                                                             // control thread is running or stop(true is running, false is stop)

using namespace std;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// IPCs
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char buffer[MEM_SIZE_SM] = {0,};
int capWidth    = 0;
int capHeight   = 0;

struct grab_data{
    int capWidth;
    int capHeight;
};
struct message{
    long msg_type;
    struct grab_data data;
};
struct message msq;

struct lpdr_data{
    long timestamp;
    char carNo[64];
};
struct message_lpdr{
    long msg_type;
    struct lpdr_data data;
};
struct message_lpdr msq_lpdr;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

time_t getTimemsec()
{
    struct timeval time_now{};
    gettimeofday(&time_now, nullptr);
    time_t msecs_time = (time_now.tv_sec * 1000) + (time_now.tv_usec / 1000);

    return msecs_time;
}

bool isNumber(const string& str)
{
    for (char const &c : str) {
        if (std::isdigit(c) == 0) return false;
    }
    return true;
}

void* thread_lpr(void* arg)
{
    cv::Mat frame;
    std::vector<krlpr::krlprutils::TargetBox> boxes;

    /* clock statistics running time */
    double mseconds_lpd, mseconds_lpr;
    clock_t start_lpd, end_lpd;
    clock_t start_lpr, end_lpr;

    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    Ipcs *Sm_Grab = new Ipcs(KEY_NUM_SM, MEM_SIZE_SM);
    Sm_Grab->SharedMemoryInit();

    Ipcs *Sm_Res = new Ipcs(KEY_NUM_SM_RES, MEM_SIZE_SM_RES);
    Sm_Res->SharedMemoryInit();    

    Ipcs *Mq_Lpdr = new Ipcs(KEY_NUM_MQ_LPDR, 0);
    Mq_Lpdr->MessageQueueCreate();
    //////////////////////////////////////////////////////////////////////////////////////////////////////////

    while(is_running) 
    {
        boxes.clear();
#if true   // false = for test(using image)
        // get image
        Sm_Res->SharedMemoryRead((char *)&msq.data);
        capWidth    = msq.data.capWidth;
        capHeight   = msq.data.capHeight;
    
        if (Sm_Grab->SharedMemoryRead((char *)buffer) < 0)
        {
            Sm_Grab->SharedMemoryInit();
            fprintf(stderr, "[Error!!!]SharedMemoryRead()\n");  
        }
        cv::Mat img = cv::Mat(msq.data.capHeight, msq.data.capWidth, CV_8UC1, (char *)buffer);
        cvtColor(img, frame, cv::COLOR_GRAY2RGB);
#else   // false = for test(using image)
        // from image(for test)
        frame = cv::imread("/oem/test_data/images.jpg");
#endif        
        // run detect
        start_lpd = clock();
        lpr_detect->run_inference(frame, boxes);            // fill data an run ai model, get result
        end_lpd = clock();
        fprintf(stderr, "Thread[ldetect] boxes: size=%d\n", boxes.size());

        mseconds_lpd  =(double)(end_lpd - start_lpd)/CLOCKS_PER_SEC;
        fprintf(stderr, "Thread[ldetect] Use time is: %.8f\n", mseconds_lpd);

        // update : 번호판 확인
        if (boxes.size()) 
        {
            mseconds_lpd  = 0;
            mseconds_lpr  = 0;

            start_lpr = clock();
            lpr_ocr->run_inference(boxes, frame);   // run lp ocr and draw result in the frame
            end_lpr = clock();
            fprintf(stderr, "Thread[locr] boxes: size=%d\n", boxes.size());

            // matched
            int ib = -1; // index of box
            if (boxes.size())
            {
                for (int i=0; i < boxes.size();i++)
                { 
                    int len = boxes[i].lpr_string.size();
                    if(len < 4) continue;
                    // extract to digit part(4자리 숫자)
                    std::string digit = boxes[i].lpr_string.substr(len-4, len-1);

                    fprintf(stderr, "license plate number[%d] : %s([flag:%d]digit = %s)\n", i, boxes[i].lpr_string.c_str(), isNumber(digit), digit.c_str());
                    //fprintf(stderr, "x1:%d, y1:%d, x2:%d, y2:%d\n", boxes[i].x1, boxes[i].y1, boxes[i].x2, boxes[i].y2);
                    //std::cout << "lpr_string : " << boxes[i].lpr_string << std::endl;
                    if(isNumber(digit))
                    {
                        ib = i;
                        break;  // big one
                    }

#if false
                    std::cout << "lpr_string length = " << boxes[i].lpr_string.size() << std::endl;
                    std::cout << "lpr_string[8] = " << boxes[i].lpr_string.at(8) << std::endl;
                    std::cout << "lpr_string[7] = " << boxes[i].lpr_string.at(7) << std::endl;
                    std::cout << "lpr_string[6] = " << boxes[i].lpr_string.at(6) << std::endl;
                    std::cout << "lpr_string[5] = " << boxes[i].lpr_string.at(5) << std::endl;
                    std::cout << "lpr_string[4] = " << boxes[i].lpr_string.at(4) << std::endl;
                    std::cout << "lpr_string[3] = " << boxes[i].lpr_string.at(3) << std::endl;
                    std::cout << "lpr_string[2] = " << boxes[i].lpr_string.at(2) << std::endl;
                    std::cout << "lpr_string[1] = " << boxes[i].lpr_string.at(1) << std::endl;
                    std::cout << "lpr_string[0] = " << boxes[i].lpr_string.at(0) << std::endl;
#endif
                }

                //////////////////////////////////////////////////////////////////////////////////////////////
                // 1. copy to /userdata/result/timestamp_0.jpg
                // 2. copy to /userdata/result/timestamp_1.jpg
                // 3. updata message queue
                //////////////////////////////////////////////////////////////////////////////////////////////
                int qsize = Mq_Lpdr->MessageQueueQNum();
                if(ib > -1 && qsize < MQ_LPDR_MAX_QSIZE)
                {
                    time_t msecs_time = getTimemsec();
                    std::string fname;
                    fname = "/userdata/result/" + to_string(msecs_time) + "_" + boxes[ib].lpr_string + "_0.jpg";
                    cv::imwrite(fname.c_str(), frame);

                    cv::Rect rect(boxes[ib].x1, boxes[ib].y1, boxes[ib].x2-boxes[ib].x1, boxes[ib].y2-boxes[ib].y1);
                    cv::Mat lpd = frame(rect);
                    //fname = "/userdata/result/" + to_string(msecs_time) + "_1.jpg";
                    fname = "/userdata/result/" + to_string(msecs_time) + "_" + boxes[ib].lpr_string + "_1.jpg";
                    cv::imwrite(fname.c_str(), lpd);

                    msq_lpdr.msg_type = 1;
                    msq_lpdr.data.timestamp = msecs_time;
                    memset(msq_lpdr.data.carNo, 0, sizeof(msq_lpdr.data.carNo));
                    strncpy(msq_lpdr.data.carNo, boxes[ib].lpr_string.c_str(), boxes[ib].lpr_string.size());
                    Mq_Lpdr->MessageQueueWrite((char *)&msq_lpdr);
                }
                //////////////////////////////////////////////////////////////////////////////////////////////
                
            }
            mseconds_lpr  =(double)(end_lpr - start_lpr)/CLOCKS_PER_SEC;
            fprintf(stderr, "Thread[locr] Use time is: %.8f\n\n", mseconds_lpr);

            cv::imwrite("/oem/Screen_shot/detect.jpg", frame);
        }

    }

    SAFE_DELETE(Sm_Grab);
    SAFE_DELETE(Sm_Res);
    SAFE_DELETE(Mq_Lpdr);

    return nullptr;
}

int main(int argc, char** argv) {
    int tid[THREAD_MAX_NUM];
    int CPU_NUM = sysconf(_SC_NPROCESSORS_CONF);

    // User parameters
    int res;
    string strDetectModelBin = "";
    string strDetectModelParam = "";
    string strOCRModel = "";
    while ((res=getopt(argc, argv, "d:p:o:h")) != -1) {
		switch(res) {
		case 'd':   // lpd ai model(file full path)
			strDetectModelBin = optarg;
		break;

        case 'p':   // lpd ai model(file full path)
			strDetectModelParam = optarg;
		break;
			
		case 'o':   // lpd ai model(file full path)
			strOCRModel = optarg;
		break;
			
		case 'h':
      std::cout << " [Usage]: " << argv[0] << " [-h]\n"
	              << " [-d lpd ai model(file full path)] "
                  << " [-o lpd ai model(file full path)] "
                  << endl;
			break;
		}
	}

        

    // LPD Model
    lpr_detect 	= new krlpr::l_detect(strDetectModelParam, strDetectModelBin);         //Load lpr model

    // LPR Model
    lpr_ocr 	= new krlpr::l_ocr(strOCRModel);         //Load lp ocr model
   
#if true
#if false
    msq_lpdr.msg_type = 1;
    MessageQueueCreate_lpdr();
    MessageQueueInit();
    SharedMemoryInit();
#else
    
#endif
    

    pthread_mutex_init(&mutex, NULL);       // init mutex
    pthread_t thread[THREAD_MAX_NUM];
    fprintf(stdout, "System has %i processor(s). \n", CPU_NUM);

    cpu_set_t mask_ldetect, mask_locr, mask_lpr;  //CPU core set
    CPU_ZERO(&mask_ldetect);
    CPU_ZERO(&mask_locr);
    CPU_ZERO(&mask_lpr);
    /* set cpu mask */
    CPU_SET(4, &mask_ldetect);
    CPU_SET(5, &mask_locr);
    CPU_SET(5, &mask_lpr);
    
    /* create thread */
    is_running = true;
    //pthread_create(&thread[0], NULL, thread_ldetect, NULL);
    //pthread_create(&thread[1], NULL, thread_locr, NULL);
    pthread_create(&thread[2], NULL, thread_lpr, NULL);

    /* set thread cpu Affinity */
    //if (pthread_setaffinity_np(thread[0], sizeof(mask_ldetect), &mask_ldetect) != 0) {
    //    printf("warning: could not set CPU detect affinity, continuing...\n");
    //}
    //if (pthread_setaffinity_np(thread[1], sizeof(mask_locr), &mask_locr) != 0) {
    //    printf("warning: could not set CPU ocr affinity, continuing...\n");
    //}
    if (pthread_setaffinity_np(thread[2], sizeof(mask_lpr), &mask_lpr) != 0) {
        printf("warning: could not set CPU ocr affinity, continuing...\n");
    }

    //blocking main thread until the two thread closed
    //for(int i = 0; i < THREAD_MAX_NUM; i++) {
    for(int i = 2; i < THREAD_MAX_NUM; i++) {
        pthread_join(thread[i], NULL);
    }
#else   // test
	for(int i=0; i<100; i++)
	{
		cv::Mat src 	= cv::imread("/oem/test_data/images.jpg");
        	clock_t start_d, end_d;
        	clock_t start_r, end_r;

		std::vector<krlpr::krlprutils::TargetBox> boxes;    	// result
        	start_d = clock();
		lpr_detect->run_inference(src, boxes);            	// fill data an run ai model, get result
		end_d = clock();
		
		std::cout << boxes.size() << std::endl;
        	start_r = clock();
		lpr_ocr->run_inference(boxes, src);
		end_r = clock();
        
        	double seconds_d  =(double)(end_d - start_d)/CLOCKS_PER_SEC;
        	double seconds_r  =(double)(end_r - start_r)/CLOCKS_PER_SEC;
        	
		    fprintf(stderr, "LPD Use time is: %.8f\n", seconds_d);
        	fprintf(stderr, "LPR Use time is: %.8f\n", seconds_r);
	
		cv::imwrite("./output.jpg", src);
	}
#endif

	return 0;
}
