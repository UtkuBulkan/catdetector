/*
 * Copyright 2018 SU Technology Ltd. All rights reserved.
 *
 * MIT License
 *
 * Copyright (c) 2018 SU Technology
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Some parts of this file has been inspired and influenced from this link :
 * https://www.learnopencv.com/deep-learning-based-object-detection
 * -and-instance-segmentation-using-mask-r-cnn-in-opencv-python-c/
 *
 */
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include "tensorflow/c/c_api.h"
#include <ObjectDetection.h>
#include "syslog_cpp.h"

#include "rdkafkacpp.h"
#include "kafkaproducer.h"

#include <openssl/md5.h>
#include <fcntl.h>
#include <sys/mman.h>

ObjectDetector::ObjectDetector() : confidence_threshold(0.9),
		mask_threshold(0.3),
		class_definition_file("../data/mscoco_labels.names"),
		colors_file("../data/colors.txt"),
		text_graph_file("../data/mask_rcnn_inception_v2_coco_2018_01_28.pbtxt"),
		model_weights_file("../data/frozen_inference_graph.pb")
{
	std::ifstream classes_file_stream(class_definition_file.c_str());
	std::ifstream colors_file_stream(colors_file.c_str());
	std::string line;

	logger << syslog::level::info << "ObjectDetector Constructor Begin" << std::endl;
	while (getline(classes_file_stream, line)) {
		classes.push_back(line);
		logger << syslog::level::debug << "Class Labels :" << line << std::endl;
	}

	while (getline(colors_file_stream, line)) {
		std::stringstream ss(line);
		double red, green, blue;
		ss >> red >> green >> blue;
		colors.push_back(cv::Scalar(red, green, blue, 255.0));
		logger << syslog::level::debug << "Colors.txt Colors :" << red << "," << green << "," << blue << std::endl;
	}

	// Load the network for the model
	logger << syslog::level::info << "ObjectDetector Loading Network" << std::endl;
	net = cv::dnn::readNetFromTensorflow(model_weights_file, text_graph_file);
	net.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
	net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
	logger << syslog::level::info << "ObjectDetector Network Loaded" << std::endl;
	logger << syslog::level::info << "ObjectDetector Constructor End" << std::endl;
}

ObjectDetector::~ObjectDetector() {
}

void ObjectDetector::draw_box(cv::Mat& frame, int classId, float confidence, cv::Rect box, cv::Mat& objectMask)
{
	std::string label = cv::format("%2.2f", confidence);
	std::vector<cv::Mat> contours;
	cv::Mat hierarchy, mask, coloredRoi;
	cv::Size labelSize;
	cv::Scalar color;
	int baseLine;

	logger << syslog::level::debug << "ObjectDetector drawBox Begin" << std::endl;
	cv::rectangle(frame, cv::Point(box.x, box.y), cv::Point(box.x+box.width, box.y+box.height),cv::Scalar(255, 178, 50), 3);
	if (!classes.empty())
	{
		CV_Assert(classId < (int)classes.size());
		label = classes[classId] + ":" + label;
	}
	labelSize = cv::getTextSize(label, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);
	box.y = cv::max(box.y, labelSize.height);
	cv::rectangle(frame, cv::Point(box.x, box.y - round(1.5*labelSize.height)), cv::Point(box.x + round(1.5*labelSize.width), box.y + baseLine), cv::Scalar(255, 255, 255), cv::FILLED);
	cv::putText(frame, label, cv::Point(box.x, box.y), cv::FONT_HERSHEY_SIMPLEX, 0.75, cv::Scalar(0,0,0),1);
	color = colors[classId%colors.size()];

	cv::resize(objectMask, objectMask, cv::Size(box.width, box.height));
	mask = (objectMask > mask_threshold);
	coloredRoi = (0.3 * color + 0.7 * frame(box));
	coloredRoi.convertTo(coloredRoi, CV_8UC3);

	mask.convertTo(mask, CV_8U);
	cv::findContours(mask, contours, hierarchy, cv::RETR_CCOMP, cv::CHAIN_APPROX_SIMPLE);
	cv::drawContours(coloredRoi, contours, -1, color, 5, cv::LINE_8, hierarchy, 100);
	coloredRoi.copyTo(frame(box), mask);
	logger << syslog::level::debug << "ObjectDetector drawBox End" << std::endl;
}

void ObjectDetector::generate_json(cv::Mat &frame, const int &classId, const int &framecount, std::string frame_md5, std::string video_md5)
{
	json local_j;

	local_j["class"] = classes[classId].c_str();
	local_j["frame"] = framecount;
	local_j["hash-frame"] = frame_md5;
	local_j["hash-video"] = video_md5;

	std::vector<uchar> buffer;
	#define MB 1024 * 1024
	buffer.resize(200*MB);
	cv::imencode(".png", frame, buffer);
	local_j["image"] = buffer;
	j.push_back(local_j);
}

void ObjectDetector::post_process(cv::Mat& frame, const std::vector<cv::Mat>& outs, int framecount, std::string hash_video)
{
	cv::Mat output_detections = outs[0], outMasks = outs[1];
	const int num_detections = output_detections.size[2];
	const int num_classes = outMasks.size[1];

	logger << syslog::level::info << "ObjectDetector postprocess begin" << std::endl;
	output_detections = output_detections.reshape(1, output_detections.total() / 7);
	logger << "Object Detector postprocess num_detections : " << num_detections << std::endl;
	logger << "Object Detector postprocess num_classes : " << num_classes << std::endl;
	for (int i = 0; i < num_detections; ++i)
	{
		float score = output_detections.at<float>(i, 2);
		if (score > confidence_threshold)
		{
			int classId = static_cast<int>(output_detections.at<float>(i, 1));
			int left = static_cast<int>(frame.cols * output_detections.at<float>(i, 3));
			int top = static_cast<int>(frame.rows * output_detections.at<float>(i, 4));
			int right = static_cast<int>(frame.cols * output_detections.at<float>(i, 5));
			int bottom = static_cast<int>(frame.rows * output_detections.at<float>(i, 6));

			left = cv::max(0, cv::min(left, frame.cols - 1));
			top = cv::max(0, cv::min(top, frame.rows - 1));
			right = cv::max(0, cv::min(right, frame.cols - 1));
			bottom = cv::max(0, cv::min(bottom, frame.rows - 1));
			cv::Rect box = cv::Rect(left, top, right - left + 1, bottom - top + 1);
			cv::Mat objectMask(outMasks.size[2], outMasks.size[3],CV_32F, outMasks.ptr<float>(i,classId));
			draw_box(frame, classId, score, box, objectMask);

			generate_json(frame, classId, framecount, "", hash_video);
		}
	}
	logger << syslog::level::info << "ObjectDetector postprocess end" << std::endl;
}

void ObjectDetector::process_frame(cv::Mat &frame, int framecount, std::string hash_video) {
	std::vector<std::string> outNames(2);
	std::vector<double> layersTimes;
	std::vector<cv::Mat> outs;
	std::string label;
	cv::Mat blob;
	double freq, t;

	logger << syslog::level::info << "ObjectDetector process_frame begin" << std::endl;
	//cv::dnn::blobFromImage(frame, blob);
	cv::dnn::blobFromImage(frame, blob, 1.0, cv::Size(frame.cols, frame.rows), cv::Scalar(), true, false);
	net.setInput(blob);
	outNames[0] = "detection_out_final";
	outNames[1] = "detection_masks";
	net.forward(outs, outNames);
	logger << "Number of outs : " << outs.size() << std::endl;

	post_process(frame, outs, framecount, hash_video);
	freq = cv::getTickFrequency() / 1000;
	t = net.getPerfProfile(layersTimes) / freq;
	label = cv::format("London South Bank University - Utku Bulkan\nFrame processing time: %.2f ms", t);
	cv::putText(frame, label, cv::Point(0, 15), cv::FONT_HERSHEY_SIMPLEX, 0.9, cv::Scalar(0, 0, 0));
	logger << syslog::level::info << "ObjectDetector process_frame end" << std::endl;
}

// Get the size of the file by its file descriptor
unsigned long get_size_by_fd(int fd)
{
    struct stat statbuf;
    if(fstat(fd, &statbuf) < 0) exit(-1);
    return statbuf.st_size;
}

std::string md5_hash(std::string filename)
{
    int file_descript;
    unsigned long file_size;
    char* file_buffer;
    unsigned char result[MD5_DIGEST_LENGTH];

    printf("using file:\t%s\n", filename.c_str());

    file_descript = open(filename.c_str(), O_RDONLY);
    if(file_descript < 0) return 0;

    file_size = get_size_by_fd(file_descript);
    printf("file size:\t%lu\n", file_size);

    file_buffer = (char *) mmap(0, file_size, PROT_READ, MAP_SHARED, file_descript, 0);
    MD5((unsigned char*) file_buffer, file_size, result);
    munmap(file_buffer, file_size);

    char md5string[2 * MD5_DIGEST_LENGTH + 1];

    memset(md5string, 0, 2 * MD5_DIGEST_LENGTH + 1);

    for(int i = 0; i < MD5_DIGEST_LENGTH; ++i)
    	sprintf(&md5string[i*2], "%02x", (unsigned int)result[i]);

    std::string result_str (reinterpret_cast<char*>(md5string));

    std::cout <<  "hash:" << result_str << std::endl;

    return result_str;
}

void ObjectDetector::loop() {
	cv::Mat frame;
	cv::VideoCapture capture;
	cv::VideoWriter outputVideo;

	logger <<  "Hello from TensorFlow C library version : " << TF_Version() << std::endl;
	logger << syslog::level::debug << "Opening file : " << filename.c_str() << std::endl;

	capture.open(filename);
	if ( !capture.isOpened	() ) {
		throw "Error opening file.\n";
	}

	capture >> frame;

	int ex = static_cast<int>(capture.get(cv::CAP_PROP_FOURCC));     // Get Codec Type- Int form
	//int codec = cv::VideoWriter::fourcc('M','P','4','2');
	cv::Size S = cv::Size((int) capture.get(cv::CAP_PROP_FRAME_WIDTH),    // Acquire input size
                  (int) capture.get(cv::CAP_PROP_FRAME_HEIGHT));


	outputVideo.open("./output.mp4", ex, capture.get(cv::CAP_PROP_FPS), S, true);

	cv::namedWindow("Camera1", cv::WINDOW_NORMAL);
	cv::resizeWindow("Camera1", 640, 480);

	int framecount = 0;

	std::string hash_video = md5_hash(filename);

	//doc.parse<0>(text);    // 0 means default parse flags

	while(1) {
		logger << syslog::level::info << "Frame count : " << framecount << std::endl;
		/*
		try {
			for(int i=0;i<24;i++)
				capture >> frame;
		} catch(cv::Exception ex) {
			std::cout << ex.what() << std::endl;
		} catch(...) {
			std::cout << "Unknown exception" << std::endl;
		}
		*/
		logger << syslog::level::debug << "Frame resolution : " << frame.rows << "x" << frame.cols << std::endl;

		//if (framecount % 24 == 0)
		{
			process_frame(frame, framecount, hash_video);
			cv::imshow("Camera1", frame);

			outputVideo << frame;
			/*std::cout << j << std::endl;*/

			/*std::ofstream myfile;
			std::string videodata_filename(hash_video + ".json");
			myfile.open (videodata_filename);
			myfile << j << std::endl;
			myfile.close();*/

			//video_analyser_kafka_producer(j.dump().c_str(), "TutorialTopic");
		}
		if(cv::waitKey(30) >= 0) break;
		capture >> frame;
		framecount++;
	}
}
