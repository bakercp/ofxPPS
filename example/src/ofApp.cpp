// =============================================================================
//
// Copyright (c) 2016 Christopher Baker <http://christopherbaker.net>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// =============================================================================


#include "ofApp.h"


void ofApp::setup()
{
    int num;
    pps_handle_t handle[4];
    int avail_mode[4];
    int i = 0;
    int ret;

    int argc = 2;
    char* argv[] = {{"cmd"}, {"/dev/pps0"} };

    /* Check the command line */
//    if (argc < 2)
//        usage(argv[0]);

    for (i = 1; i < argc && i <= 4; i++) {
        ret = PPS::findSource(argv[i], &handle[i - 1], &avail_mode[i - 1]);
        if (ret < 0)
            return;//exit(EXIT_FAILURE);
    }

    num = i - 1;
    printf("ok, found %d source(s), now start fetching data...\n", num);

    /* loop, printing the most recent timestamp every second or so */
    while (1) {
        for (i = 0; i < num; i++) {
            ret = fetchSource(i, &handle[i], &avail_mode[i]);
            if (ret < 0 && errno != ETIMEDOUT)
                return; //exit(EXIT_FAILURE);
        }
    }

    for (; i >= 0; i--)
        time_pps_destroy(handle[i]);
    
    return;

}


void ofApp::update()
{
}
