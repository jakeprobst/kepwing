#include "fifothread.h"

#include <stdio.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

const int BUFLEN = 1024*4;

FifoThread::FifoThread(xdgHandle xdg)
{
    running = true;
    spath = QString(xdgConfigHome(&xdg)) + "/kepwing/control";

    const char* path = spath.toUtf8().constData();
    printf("path: %s\n", path);
    unlink(path);
    mkfifo(path, 0766);
    fifofd = open(path, O_RDONLY|O_NONBLOCK);
}

FifoThread::~FifoThread()
{
    close(fifofd);
    unlink(spath.toUtf8().constData());
}

void FifoThread::stop()
{
    running = false;
}

void FifoThread::ExecCommand(QString str)
{
    str = str.remove("\n");
    if (str.startsWith("popup")) {
        str = str.replace("popup ", "");
        emit PopupWord(str);
    }
    else if (str.startsWith("window")) {
        str = str.replace("window ", "");
        emit InWindowSearch(str);
    }
}



// select/poll, why are you so bad with fifo?
void FifoThread::run()
{
    /*fd_set fds;
    timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;*/

    /*pollfd fds[2];
    fds[0].fd = fifofd;
    fds[0].events = POLLIN;*/
    
    char fifobuf[BUFLEN];
    int fb = 0;
    while(running) {
        while (true) {
            int r = read(fifofd, fifobuf+fb, 1);
            if (r <= 0)
                break;
            
            if (fifobuf[fb] == '\n') {
                fifobuf[fb] = '\0';
                //printf("fb: %s\n", fifobuf);
                //QString fb(fifobuf);
                //printf("fb2: %s\n", fb.toUtf8().constData());
                ExecCommand(QString::fromUtf8(fifobuf));
                memset(fifobuf, 0, BUFLEN);
                fb = -1;
            }
            fb++;

        }

        usleep(150);
        /*int r = poll(fds, 1, 5000);
        if (r > 0) {
            printf("changed!\n");
        }*/
        /*FD_ZERO(&fds);
        FD_SET(fifofd, &fds);
        int r = select(fifofd+1, &fds, NULL, NULL, &tv);
        if (FD_ISSET(fifofd, &fds)) {
            printf("changed?\n");
            char buf[BUFLEN];
            read(fifofd, buf, BUFLEN);
            ExecCommand(buf);

            close(fifofd);
            const char* path = spath.toUtf8().constData();
            fifofd = open(path, O_RDONLY|O_NONBLOCK);

        }
        printf("loop\n");*/
    }
}









