/* Copyright (C) 2018 Michal Kosciesza <michal@mkiol.net>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QDebug>
#include <QThread>

#include "taskexecutor.h"

TaskExecutor* TaskExecutor::inst = nullptr;

TaskExecutor::Task::Task(std::function<void()> job) : m_job(job)
{
}

void TaskExecutor::Task::run()
{
    m_job();
}

TaskExecutor* TaskExecutor::instance(QObject* parent)
{
    if (TaskExecutor::inst == nullptr) {
        TaskExecutor::inst = new TaskExecutor(parent);
    }

    return TaskExecutor::inst;
}

TaskExecutor::TaskExecutor(QObject* parent) :
    m_pool(parent)
{
    m_pool.setMaxThreadCount(1);
}

void TaskExecutor::startTask(const std::function<void()> &job)
{
    auto task = new Task(job);
    task->setAutoDelete(true);
    m_pool.start(task);
}

void TaskExecutor::waitForDone()
{
    m_pool.waitForDone();
}

bool TaskExecutor::taskActive()
{
    return m_pool.activeThreadCount() > 0;
}

void TaskExecutor::tsleep(int ms)
{
    if (ms > 0)
        QThread::currentThread()->msleep(ms);
}

