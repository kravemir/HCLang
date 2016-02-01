/*
 * HCLang - Highly Concurrent Language
 * Copyright (c) 2015 Miroslav Kravec
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */
#include "async_io.h"

#include <aio.h>
#include <signal.h>

#define BUF_SIZE 4          /* Size of buffers for read operations */
#define IO_SIGNAL SIGRTMIN + 8   /* Signal used to notify I/O completion */

struct aioRequest {
    SlotReference callback;
    struct aiocb *aiocbp;
};

static volatile int numOpenReads = 0; // atomic int
static void                 aioSigHandler(int sig, siginfo_t *si, void *ucontext);

int asyncIoInitialize() {
    struct sigaction sa;
    sa.sa_flags = SA_RESTART;
    sigemptyset(&sa.sa_mask);

    sa.sa_flags = SA_RESTART | SA_SIGINFO;
    sa.sa_sigaction = aioSigHandler;
    if (sigaction(IO_SIGNAL, &sa, NULL) == -1)
        return 1;
    return 0;
}

static
void aioSigHandler(int sig, siginfo_t *si, void *ucontext)
{
    struct aioRequest *request = (aioRequest *) si->si_value.sival_ptr;
    int bytes_read = aio_return(request->aiocbp);

    // printf("I/O completion signal received %d: %.*s\n", bytes_read, bytes_read, request->aiocbp->aio_buf);

    AioCallbackData *callbackData = (AioCallbackData *) malloc(sizeof(AioCallbackData));
    callbackData->count = bytes_read;
    callbackData->data = (char *) request->aiocbp->aio_buf;

    system_putMsg(request->callback.system, request->callback.msg_id, callbackData);

    numOpenReads -= 1;
}

bool asyncIoHasPendingReads() {
    return numOpenReads > 0;
}

void asyncIoRead(int filedescriptor, SlotReference callback ) {
    struct aioRequest *request = (aioRequest *) malloc(sizeof(struct aioRequest));
    struct aiocb *aiocbInstance = (aiocb *) malloc(sizeof(struct aiocb));

    request->callback = callback;

    request->aiocbp = aiocbInstance;
    request->aiocbp->aio_fildes = filedescriptor;
    request->aiocbp->aio_buf = malloc(BUF_SIZE);
    request->aiocbp->aio_nbytes = BUF_SIZE;
    request->aiocbp->aio_reqprio = 0;
    request->aiocbp->aio_offset = 0;
    request->aiocbp->aio_sigevent.sigev_notify = SIGEV_SIGNAL;
    request->aiocbp->aio_sigevent.sigev_signo = IO_SIGNAL;
    request->aiocbp->aio_sigevent.sigev_value.sival_ptr = request;

    numOpenReads += 1;

    if (aio_read(request->aiocbp) == -1)
        ; // TODO error handle
}