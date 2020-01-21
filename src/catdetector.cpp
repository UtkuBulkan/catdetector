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
#include "camera_manager.h"

#include <syslog.h>

int main(int argc, char *argv[])
{
	setlogmask (LOG_UPTO (LOG_NOTICE));
	openlog ("catdetector", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
	std::string filename, uuid;
	ObjectDetector *obj = ObjectDetector::GenerateDetector("Yolo");

	if (argc > 1) {
		syslog(LOG_NOTICE, "Number of args : %d", argc);
		std::vector<std::string> all_args;

		all_args.assign(argv + 1, argv + argc);
		filename = all_args[0];
		uuid = all_args[1];
		syslog(LOG_NOTICE, "Size of args : %d, Filename : %s, UUID : %s", (int) all_args.size(), filename.c_str(), uuid.c_str());
	} else{
		filename = "./demo.mp4";
	}
	Camera *camera = new Camera(filename, uuid);
	camera->set_models({obj});

	camera->process_frame();

	closelog ();
}
