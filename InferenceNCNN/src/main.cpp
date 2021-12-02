#include "inference_ncnn/l_ocr.h"
#include "inference_ncnn/l_detect.h"
#include "inference_ncnn/utils.h"
#include "ipcs.h"
#include "sha256.h"

#include "opencv2/opencv.hpp"
#include <time.h>
#include <cstdio>
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <unistd.h>
#include <string>
#include <sqlite3.h>
#include <chrono>

#define __USE_GNU
#include <sched.h>
#include <ctype.h>
#include <string.h>
#include <pthread.h>
#include <queue>
#include <sys/time.h>

#define LOG_NAME	"[LPR]"

#define THREAD_MAX_NUM                                                      3
#define DB_NAME "/oem/db/lpr.db"

using namespace std;
using namespace chrono;

pthread_mutex_t mutex;                                                      // pthread_mutex_t for thread save(frame_queue, boxes_queue)
krlpr::l_ocr 	*lpr_ocr;                                                   // lpr_ocr class
krlpr::l_detect *lpr_detect;                                                // lpr_detect class
std::queue< std::vector<krlpr::krlprutils::TargetBox> > boxes_queue;        // a queue transfer lpr_detect result from l_detect thread to l_ocr thread
std::queue<cv::Mat> frame_queue;                                            // a queue transfer read frame from l_detect thread to l_ocr thread
int is_running;                                                             // control thread is running or stop(true is running, false is stop)

sqlite3 *db;                                                                // sqlite db handler
string  msg;
int coolTime = 60;  // def: 60초

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

    char status[32];    // 수배종류
    char carNo[32];     // 차량정보
    // RECT
    int x;              // rect[0]
    int y;              // rect[1]    
    int endX;           // rect[2]
    int endY;           // rect[3]
};
struct message_lpdr{
    long msg_type;
    struct lpdr_data data;
};
struct message_lpdr msq_lpdr;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//#define USE_Q_FOR_FILELIST
struct file_info{
    std::string fileOrg;
    std::string fileLpd;
};
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

int code_value = 0;
static int callback(
    void *bIsWanted,
    int argc,
    char **argv, 
    char **azColName)
{    
    // table name : wanted
    // col1 : carNo
    // col2 : code
    // col3 : reserved

    bool *b = (bool*)bIsWanted;
    *b = true;
    
    for (int i = 0; i < argc; i++)
    {
        //printf("[%d]%s = %s\n", i, azColName[i], argv[i] ? argv[i] : "NULL");
        if (strncmp(azColName[i], "code", 4) == 0)
        {
            code_value = atoi(argv[i]);
        }
    }
    
    //printf("\n");
    
    return 0;
}

bool checkCoolTime(string carNo)
{
    static string preNo;
    static system_clock::time_point start;
    static system_clock::time_point end;

    if(preNo.compare(carNo) == 0)   // same
    {
        end = system_clock::now();
        nanoseconds nano = end - start;
        int64_t elapsed_sec = nano.count()/1000000000;
        if(elapsed_sec < coolTime) // wait...
        {
            INFO_LOG(string("Wait~~~Cool time"));
            return false;
        }
    }

    start = system_clock::now();
    preNo = carNo;

    return true;

}

bool isWanted(const string& str)
{  
    bool bIsWanted = 0;
    char *err_msg = 0;
    sqlite3_stmt *res;

    code_value = 0; // init

    // get target value from db
    string encrypt = sha256(str);
    //cout << "encrypt : " << encrypt << endl;

    char sql_buf[128] = {0, };
    sprintf(sql_buf, "SELECT * FROM wanted where carNo=\'%s\';", encrypt.c_str());
    int rc = sqlite3_exec(db, sql_buf, callback, &bIsWanted, &err_msg);
    if (rc != SQLITE_OK )
    {
        //fprintf(stderr, "Failed to select data\n");
        //fprintf(stderr, "SQL error: %s\n", err_msg);

        ERR_LOG(string("Failed to select data"));
        msg = string_format("SQL error: %s", err_msg);
        ERR_LOG(msg);

        sqlite3_free(err_msg);

        return false;
    } 

    //return bIsWanted;
    return bIsWanted ? checkCoolTime(str) : false;
}

static string convertWantedType(int code)
{
    string strType="수배종류";
    switch(code)
    {
        case 1:
            strType="도난";
            break;
        case 2:
            strType="무적";
            break;
        case 3:
            strType="범죄";
            break;
        case 5:
            strType="기타";
            break;
        case 7:
            strType="번호판분실";
            break;
        case 9:
            strType="...";
            break;
    }

    return strType;
}

int deleteFile(string fname)
{
    if (remove(fname.c_str()) != 0)
    {
        //cout << "[POP_Queue]Error deleting file : " << fInfo.fileOrg << endl;
        msg = string_format("[POP_Queue]Error deleting file : %s", fname.c_str());
        ERR_LOG(msg);
    }
    else
    {
        //cout << "[POP_Queue]File successfully deleted : " << fInfo.fileOrg << endl;
        msg = string_format("[POP_Queue]File successfully deleted : %s", fname.c_str());
        INFO_LOG(msg);
    }

    return 1;
}

void* thread_lpr(void* arg)
{
    cv::Mat frame_gray;
    cv::Mat frame_bgr;
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

    Ipcs *Sm_Lpr = new Ipcs(KEY_NUM_SM_LPR, MEM_SIZE_SM_LPR);
    Sm_Lpr->SharedMemoryCreate();    

    Ipcs *Mq_Lpdr = new Ipcs(KEY_NUM_MQ_LPDR, 0);
    Mq_Lpdr->MessageQueueCreate();
    //////////////////////////////////////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    INFO_LOG(string("####################################################################")); 
    msg = string_format("sqlite3 version = %s", sqlite3_libversion());
    INFO_LOG(msg);
    INFO_LOG(string("####################################################################")); 
    int rc = sqlite3_open(DB_NAME, &db);
    msg = string_format("DB Open(%d) : %s", rc, DB_NAME);
    INFO_LOG(msg);
    //////////////////////////////////////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef USE_Q_FOR_FILELIST
    std::queue<file_info> fileQueue;
#endif
    //////////////////////////////////////////////////////////////////////////////////////////////////////////

    while(is_running) 
    {
        boxes.clear();
#if true   // real code
        
        // get image
        if (Sm_Res->SharedMemoryRead((char *)&msq.data) < 0)
        {
            Sm_Res->SharedMemoryInit(); 
            EMERG_LOG(string("[Error!!!]SharedMemoryRead()"));
            break;
        }
        capWidth    = msq.data.capWidth;
        capHeight   = msq.data.capHeight;
    
        if (Sm_Grab->SharedMemoryRead((char *)buffer) < 0)
        {
            Sm_Grab->SharedMemoryInit();
            EMERG_LOG(string("[Error!!!]SharedMemoryRead()"));
            break;
        }

        frame_gray = cv::Mat(msq.data.capHeight, msq.data.capWidth, CV_8UC1, (char *)buffer);
        cvtColor(frame_gray, frame_bgr, cv::COLOR_GRAY2BGR);

#else   // false = for test(using image)
        // from image(for test)
        INFO_LOG(string("################## DEBUG MODE ##############################"));
        frame_bgr = cv::imread("/oem/test_data/images.jpg");
        cv::cvtColor(frame_bgr, frame_gray, cv::COLOR_BGR2GRAY);
#endif        
        // run detect
        start_lpd = clock();
        lpr_detect->run_inference(frame_bgr, boxes);            // fill data an run ai model, get result
        end_lpd = clock();
        //fprintf(stderr, "[V1115]Thread[ldetect] boxes: size=%d\n", boxes.size());
        msg = string_format("Thread[ldetect] boxes: size=%d", boxes.size());
        DEBUG_LOG(msg);

        mseconds_lpd  =(double)(end_lpd - start_lpd)/CLOCKS_PER_SEC;
        //fprintf(stderr, "Thread[ldetect] Use time is: %.8f\n", mseconds_lpd);
        msg = string_format("Thread[ldetect] Use time is: %.8f", mseconds_lpd);
        DEBUG_LOG(msg);

        // update : 번호판 확인
        if (boxes.size()) 
        {
            mseconds_lpd  = 0;
            mseconds_lpr  = 0;

            start_lpr = clock();
            lpr_ocr->run_inference(boxes, frame_gray);   // run lp ocr and draw result in the frame
            end_lpr = clock();
            //fprintf(stderr, "Thread[locr] boxes: size=%d\n", boxes.size());
            msg = string_format("Thread[locr] boxes: size=%d", boxes.size());
            DEBUG_LOG(msg);

            // matched
            int ib = -1; // index of box
            if (boxes.size())
            {
                bool bIsWanted = false;
                for (int i=0; i < boxes.size();i++)
                { 
                    int len = boxes[i].lpr_string.size();
                    if(len < 4) continue;
                    // extract to digit part(4자리 숫자)
                    std::string digit = boxes[i].lpr_string.substr(len-4, len-1);

                    //fprintf(stderr, "license plate number[%d] : %s([flag:%d]digit = %s)\n", i, boxes[i].lpr_string.c_str(), isNumber(digit), digit.c_str());
                    msg = string_format("license plate number[%d] : %s([flag:%d]digit = %s)", i, boxes[i].lpr_string.c_str(), isNumber(digit), digit.c_str());
                    INFO_LOG(msg);
                    //fprintf(stderr, "x1:%d, y1:%d, x2:%d, y2:%d\n", boxes[i].x1, boxes[i].y1, boxes[i].x2, boxes[i].y2);
                    //std::cout << "lpr_string : " << boxes[i].lpr_string << std::endl;
                    if(isNumber(digit))
                    {
                        // 수배차량 조회
                        bIsWanted = isWanted(boxes[i].lpr_string.c_str());
                        if(bIsWanted)
                        {
                            //std::cout << "수배차량 : " << boxes[i].lpr_string.c_str() << std::endl;
                            msg = string_format("수배차량 : %s", boxes[i].lpr_string.c_str());
                        }
                        else
                        {
                            //std::cout << "일반차량 : " << boxes[i].lpr_string.c_str() << std::endl;
                            msg = string_format("일반차량 : %s", boxes[i].lpr_string.c_str());
                        }
                        INFO_LOG(msg);

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
                }   // for

                //////////////////////////////////////////////////////////////////////////////////////////////
                // 1. copy to /userdata/result/timestamp_0.jpg
                // 2. copy to /userdata/result/timestamp_1.jpg
                // 3. updata message queue
                //////////////////////////////////////////////////////////////////////////////////////////////
                file_info fInfo;
                int qsize = Mq_Lpdr->MessageQueueQNum();
                // message queue full
                // old data : delete
                // new data : add
                if(bIsWanted && ib > -1 && qsize >= MQ_LPDR_MAX_QSIZE)   // delete(old)
                {
                    Mq_Lpdr->MessageQueueRead((char *)&msq_lpdr);
                    string carNo(msq_lpdr.data.carNo);
                    fInfo.fileOrg = "/userdata/result/" + to_string(msq_lpdr.data.timestamp) + "_" + carNo + "_0.jpg";
                    fInfo.fileLpd = "/userdata/result/" + to_string(msq_lpdr.data.timestamp) + "_" + carNo + "_1.jpg";

                    // deleteFile()
                    deleteFile(fInfo.fileOrg);
                    deleteFile(fInfo.fileLpd);
                }

                // add. new item
                //if(ib > -1 && qsize < MQ_LPDR_MAX_QSIZE)
                if(ib > -1)
                {
                    std::string fname_tmp;
                    time_t msecs_time = getTimemsec();

                    fInfo.fileOrg = "/userdata/result/" + to_string(msecs_time) + "_" + boxes[ib].lpr_string + "_0.jpg";
                    fname_tmp = "/oem/Screen_shot/0_tmp.jpg";
                    
                    cv::imwrite(fInfo.fileOrg.c_str(), frame_gray);
                    cv::imwrite(fname_tmp.c_str(), frame_gray);

                    cv::Rect rect(boxes[ib].x1, boxes[ib].y1, boxes[ib].x2-boxes[ib].x1, boxes[ib].y2-boxes[ib].y1);
                    cv::Mat lpd = frame_gray(rect);
                    fInfo.fileLpd = "/userdata/result/" + to_string(msecs_time) + "_" + boxes[ib].lpr_string + "_1.jpg";
                    fname_tmp = "/oem/Screen_shot/1_tmp.jpg";

                    cv::imwrite(fInfo.fileLpd.c_str(), lpd);
                    cv::imwrite(fname_tmp.c_str(), lpd);

                    // 수배정보에 관계없이 기록
                    msq_lpdr.msg_type = 1;
                    msq_lpdr.data.timestamp = msecs_time;
                    memset(msq_lpdr.data.status, 0, sizeof(msq_lpdr.data.status));
                    memset(msq_lpdr.data.carNo, 0, sizeof(msq_lpdr.data.carNo));
                    string status = convertWantedType(code_value);  // from database(table:wanted, col[0,1,2], col[1]:code)
                    strncpy(msq_lpdr.data.status, status.c_str(), status.size());   
                    strncpy(msq_lpdr.data.carNo, boxes[ib].lpr_string.c_str(), boxes[ib].lpr_string.size());
                    msq_lpdr.data.x = boxes[ib].x1;
                    msq_lpdr.data.y = boxes[ib].y1;
                    msq_lpdr.data.endX = boxes[ib].x2;
                    msq_lpdr.data.endY = boxes[ib].y2;
                   
                    //cout << "code_value : " << code_value << endl;
                    //cout << "msq_lpdr.data.status : " << msq_lpdr.data.status << endl;
                    //cout << "msq_lpdr.data.carNo : " << msq_lpdr.data.carNo << endl;
                    
                    msg = string_format("DB - carNo:%s, status:%s, x:%d, y:%d, endX:%d, endY:%d", msq_lpdr.data.carNo, msq_lpdr.data.status, msq_lpdr.data.x, msq_lpdr.data.y, msq_lpdr.data.endX, msq_lpdr.data.endY);
                    INFO_LOG(msg);

#ifdef USE_Q_FOR_FILELIST
                    fileQueue.push(fInfo);
#endif
                    // add. 2021.11.23
                    // 수배대상에 대해서만 소켓통신 준비
                    if(bIsWanted)
                    {
                        // 중복체크
                        //if(checkCoolTime(msq_lpdr.data.carNo))
                        {
                            msg = string_format("MessageQueueWrite - carNo:%s, status:%s", msq_lpdr.data.carNo, msq_lpdr.data.status);
                            INFO_LOG(msg);
                            cout << "수배차량 : " << msq_lpdr.data.carNo << ", status = " << msq_lpdr.data.status << endl;
                            Mq_Lpdr->MessageQueueWrite((char *)&msq_lpdr);
                        }
                    }
                    else
                    {
                        // file delete
                        deleteFile(fInfo.fileOrg);
                        deleteFile(fInfo.fileLpd);
                    }

                    #if false
                    // "33머 9999", "size = 9"
                    //cout << "lpr result : " << boxes[ib].lpr_string.c_str() << ", len = " << strlen(boxes[ib].lpr_string.c_str()) << endl;
                    cout << "lpr result : " << msq_lpdr.data.carNo << ", len = " << strlen(msq_lpdr.data.carNo) << endl;
                    cout << "[0] : " << msq_lpdr.data.carNo[0] << endl;
                    cout << "[1] : " << msq_lpdr.data.carNo[1] << endl;
                    cout << "[2] : " << msq_lpdr.data.carNo[2] << endl;
                    cout << "[3] : " << msq_lpdr.data.carNo[3] << endl;
                    cout << "[4] : " << msq_lpdr.data.carNo[4] << endl;
                    cout << "[5] : " << msq_lpdr.data.carNo[5] << endl;
                    cout << "[6] : " << msq_lpdr.data.carNo[6] << endl;
                    cout << "[7] : " << msq_lpdr.data.carNo[7] << endl;
                    cout << "[8] : " << msq_lpdr.data.carNo[8] << endl;
                    cout << "[=] : " << "######################################################" << endl;
                    cout << "[2,3,4] : " << msq_lpdr.data.carNo[2] << msq_lpdr.data.carNo[3] << msq_lpdr.data.carNo[4] << endl;
                    cout << "[=] : " << "######################################################" << endl;
                    printf("[0] : %d, %c\n", msq_lpdr.data.carNo[0], msq_lpdr.data.carNo[0]);
                    printf("[1] : %d, %c\n", msq_lpdr.data.carNo[1], msq_lpdr.data.carNo[1]);
                    printf("[2] : %d, %c\n", msq_lpdr.data.carNo[2], msq_lpdr.data.carNo[2]);
                    printf("[3] : %d, %c\n", msq_lpdr.data.carNo[3], msq_lpdr.data.carNo[3]);
                    printf("[4] : %d, %c\n", msq_lpdr.data.carNo[4], msq_lpdr.data.carNo[4]);
                    printf("[5] : %d, %c\n", msq_lpdr.data.carNo[5], msq_lpdr.data.carNo[5]);
                    printf("[6] : %d, %c\n", msq_lpdr.data.carNo[6], msq_lpdr.data.carNo[6]);
                    printf("[7] : %d, %c\n", msq_lpdr.data.carNo[7], msq_lpdr.data.carNo[7]);
                    printf("[8] : %d, %c\n", msq_lpdr.data.carNo[8], msq_lpdr.data.carNo[8]);
                    #endif

                    //Sm_Lpr->SharedMemoryWrite((char*)msq_lpdr.data.carNo, strlen(msq_lpdr.data.carNo));
                    Sm_Lpr->SharedMemoryWrite((char*)msq_lpdr.data.carNo, MEM_SIZE_SM_LPR); // to preview 하단 검출결과 표시 용

                    // rename
                    rename("/oem/Screen_shot/0_tmp.jpg", "/oem/Screen_shot/0.jpg");
                    rename("/oem/Screen_shot/1_tmp.jpg", "/oem/Screen_shot/1.jpg");
                }
                //////////////////////////////////////////////////////////////////////////////////////////////
                
            }
            mseconds_lpr  =(double)(end_lpr - start_lpr)/CLOCKS_PER_SEC;
            //fprintf(stderr, "Thread[locr] Use time is: %.8f\n\n", mseconds_lpr);
            msg = string_format("Thread[locr] Use time is: %.8f", mseconds_lpr);
            DEBUG_LOG(msg);

            cv::imwrite("/oem/Screen_shot/detect.jpg", frame_gray);
        }

    }

    sqlite3_close(db);

    SAFE_DELETE(Sm_Grab);
    SAFE_DELETE(Sm_Res);
    SAFE_DELETE(Sm_Lpr);
    SAFE_DELETE(Mq_Lpdr);

    INFO_LOG(string("EXIT!!!, LPR Thread!!!")); 

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
    while ((res=getopt(argc, argv, "t:d:p:o:h")) != -1) {
		switch(res) {
        case 't':
            coolTime = stoi(optarg);
        break;
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

    openlog(LOG_NAME, LOG_PID, LOG_USER);        
    msg = "LOG_LEVEL = " + to_string(LOG_LEVEL);
	INFO_LOG(msg);

    // config params
	INFO_LOG(string("#################################################################################################"));
	INFO_LOG(string("# InferenceNCNN Configs                                                                              #"));
	INFO_LOG(string("#################################################################################################"));
    msg = string_format("coolTime : %d", coolTime);
    INFO_LOG(msg);
    msg = string_format("strDetectModelBin : %s", strDetectModelBin.c_str());
    INFO_LOG(msg);
    msg = string_format("strDetectModelParam : %s", strDetectModelParam.c_str());
    INFO_LOG(msg);
    msg = string_format("strOCRModel : %s", strOCRModel.c_str());
    INFO_LOG(msg);
    INFO_LOG(string("#################################################################################################"));

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

    closelog();

	return 0;
}
