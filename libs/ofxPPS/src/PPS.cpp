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


#include "ofx/PPS.h"


namespace ofx {


int PPS::findSource(char* path, pps_handle_t* handle, int* avail_mode)
{
    pps_params_t params;
    int ret;

    printf("trying PPS source \"%s\"\n", path);

    /* Try to find the source by using the supplied "path" name */
    ret = open(path, O_RDWR);
    if (ret < 0) {
        fprintf(stderr, "unable to open device \"%s\" (%m)\n", path);
        return ret;
    }

    /* Open the PPS source (and check the file descriptor) */
    ret = time_pps_create(ret, handle);
    if (ret < 0) {
        fprintf(stderr, "cannot create a PPS source from device "
                "\"%s\" (%m)\n", path);
        return -1;
    }
    printf("found PPS source \"%s\"\n", path);

    /* Find out what features are supported */
    ret = time_pps_getcap(*handle, avail_mode);
    if (ret < 0) {
        fprintf(stderr, "cannot get capabilities (%m)\n");
        return -1;
    }
    if ((*avail_mode & PPS_CAPTUREASSERT) == 0) {
        fprintf(stderr, "cannot CAPTUREASSERT\n");
        return -1;
    }
    if ((*avail_mode & PPS_OFFSETASSERT) == 0) {
        fprintf(stderr, "cannot OFFSETASSERT\n");
        return -1;
    }

    /* Capture assert timestamps, and compensate for a 675 nsec
     * propagation delay */
    ret = time_pps_getparams(*handle, &params);
    if (ret < 0) {
        fprintf(stderr, "cannot get parameters (%m)\n");
        return -1;
    }
    params.assert_offset.tv_sec = 0;
    params.assert_offset.tv_nsec = 675;
    params.mode |= PPS_CAPTUREASSERT | PPS_OFFSETASSERT;
    ret = time_pps_setparams(*handle, &params);
    if (ret < 0) {
        fprintf(stderr, "cannot set parameters (%m)\n");
        return -1;
    }

    return 0;
}

int PPS::fetch_source(int i, pps_handle_t* handle, int* avail_mode)
{
    struct timespec timeout;
    pps_info_t infobuf;
    int ret;

    /* create a zero-valued timeout */
    timeout.tv_sec = 3;
    timeout.tv_nsec = 0;

retry:
    if (*avail_mode & PPS_CANWAIT) /* waits for the next event */
        ret = time_pps_fetch(*handle, PPS_TSFMT_TSPEC, &infobuf,
                             &timeout);
    else {
        sleep(1);
        ret = time_pps_fetch(*handle, PPS_TSFMT_TSPEC, &infobuf,
                             &timeout);
    }
    if (ret < 0) {
        if (ret == -EINTR) {
            fprintf(stderr, "time_pps_fetch() got a signal!\n");
            goto retry;
        }

        fprintf(stderr, "time_pps_fetch() error %d (%m)\n", ret);
        return -1;
    }

    printf("source %d - "
           "assert %ld.%09ld, sequence: %ld - "
           "clear  %ld.%09ld, sequence: %ld\n",
           i,
           infobuf.assert_timestamp.tv_sec,
           infobuf.assert_timestamp.tv_nsec,
           infobuf.assert_sequence,
           infobuf.clear_timestamp.tv_sec,
           infobuf.clear_timestamp.tv_nsec, infobuf.clear_sequence);
    fflush(stdout);
    
    return 0;
}


}  // namespace ofx
