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
 * @file TargetsSubscriberApp.hpp
 * This header file contains the declaration of the subscriber functions.
 *
 * This file was generated by the tool fastddsgen.
 */

#ifndef FAST_DDS_GENERATED__TARGETSSUBSCRIBERAPP_HPP
#define FAST_DDS_GENERATED__TARGETSSUBSCRIBERAPP_HPP

#include <condition_variable>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

#include "Targets.hpp"
#include "TargetsApplication.hpp"
#include "Generator_functions.h"


class TargetsSubscriberApp : public TargetsApplication,
        public eprosima::fastdds::dds::DataReaderListener
{
public:

    TargetsSubscriberApp(
            const int& domain_id);

    virtual ~TargetsSubscriberApp();

    //! Subscription callback
    void on_data_available(
            eprosima::fastdds::dds::DataReader* reader) override;

    //! Subscriber matched method
    void on_subscription_matched(
            eprosima::fastdds::dds::DataReader* reader,
            const eprosima::fastdds::dds::SubscriptionMatchedStatus& info) override;

    //! Run subscriber
    void run() override;

    //! Trigger the end of execution
    void stop() override;

private:

    //! Return the current state of execution
    bool is_stopped();

    std::shared_ptr<eprosima::fastdds::dds::DomainParticipantFactory> factory_;
    eprosima::fastdds::dds::DomainParticipant* participant_;
    eprosima::fastdds::dds::Subscriber* subscriber_;
    eprosima::fastdds::dds::Topic* topic_;
    eprosima::fastdds::dds::DataReader* reader_;
    eprosima::fastdds::dds::TypeSupport type_;
    uint16_t samples_received_;
    std::atomic<bool> stop_;
    mutable std::mutex terminate_cv_mtx_;
    std::condition_variable terminate_cv_;
};

#endif // FAST_DDS_GENERATED__TARGETSSUBSCRIBERAPP_HPP