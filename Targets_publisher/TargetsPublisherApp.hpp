// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/*!
 * @file TargetsPublisherApp.hpp
 * This header file contains the declaration of the publisher functions.
 *
 * This file was generated by the tool fastddsgen.
 */

#ifndef FAST_DDS_GENERATED__TARGETSPUBLISHERAPP_HPP
#define FAST_DDS_GENERATED__TARGETSPUBLISHERAPP_HPP

#include <condition_variable>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

#include "TargetsApplication.hpp"
#include "Generator_functions.h"

class TargetsPublisherApp : public TargetsApplication,
        public eprosima::fastdds::dds::DataWriterListener
{
public:

    TargetsPublisherApp(
            const int& domain_id);

    ~TargetsPublisherApp();

    //! Publisher matched method
    void on_publication_matched(
            eprosima::fastdds::dds::DataWriter* writer,
            const eprosima::fastdds::dds::PublicationMatchedStatus& info) override;

    //! Run publisher
    void run() override;

    //! Trigger the end of execution
    void stop() override;

private:

    //! Return the current state of execution
    bool is_stopped();

    //! Publish a sample
    bool publish(Targets_gen targets);

    std::shared_ptr<eprosima::fastdds::dds::DomainParticipantFactory> factory_;
    eprosima::fastdds::dds::DomainParticipant* participant_;
    eprosima::fastdds::dds::Publisher* publisher_;
    eprosima::fastdds::dds::Topic* topic_;
    eprosima::fastdds::dds::DataWriter* writer_;
    eprosima::fastdds::dds::TypeSupport type_;
    std::condition_variable cv_;
    int32_t matched_;
    std::mutex mutex_;
    const uint32_t period_ms_ = 100; // in ms
    uint16_t samples_sent_;
    std::atomic<bool> stop_;
};

#endif // FAST_DDS_GENERATED__TARGETSPUBLISHERAPP_HPP