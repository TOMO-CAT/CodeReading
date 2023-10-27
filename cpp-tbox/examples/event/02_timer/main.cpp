/*
 *     .============.
 *    //  M A K E  / \
 *   //  C++ DEV  /   \
 *  //  E A S Y  /  \/ \
 * ++ ----------.  \/\  .
 *  \\     \     \ /\  /
 *   \\     \     \   /
 *    \\     \     \ /
 *     -============'
 *
 * Copyright (c) 2018 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */
#include <iostream>

#include <tbox/event/loop.h>
#include <tbox/event/timer_event.h>

using namespace std;
using namespace tbox;
using namespace tbox::event;

void TimerCallback()
{
    cout << "." << flush;
}

void PrintUsage(const char *process_name)
{
    cout << "Usage:" << process_name << " epoll" << endl;
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        PrintUsage(argv[0]);
        return 0;
    }

    Loop* sp_loop = Loop::New(argv[1]);
    if (sp_loop == nullptr) {
        cout << "fail, exit" << endl;
        return 0;
    }

    TimerEvent* sp_timer = sp_loop->newTimerEvent();
    sp_timer->initialize(std::chrono::milliseconds(200), Event::Mode::kPersist);
    sp_timer->setCallback(TimerCallback);
    sp_timer->enable();

    sp_loop->runLoop(Loop::Mode::kForever);

    delete sp_timer;
    delete sp_loop;
    return 0;
}
