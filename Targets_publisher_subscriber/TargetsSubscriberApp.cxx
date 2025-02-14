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
 * @file TargetsSubscriberApp.cxx
 * This file contains the implementation of the subscriber functions.
 *
 * This file was generated by the tool fastddsgen.
 */

#include "TargetsSubscriberApp.hpp"

#include <condition_variable>
#include <stdexcept>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <fastdds/dds/core/status/SubscriptionMatchedStatus.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/qos/SubscriberQos.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include "Generator_functions.h"
#include "TargetsPubSubTypes.hpp"

using namespace eprosima::fastdds::dds;

TargetsSubscriberApp::TargetsSubscriberApp(const int& domain_id)
    : factory_(nullptr)
    , participant_(nullptr)
    , subscriber_(nullptr)
    , topic_(nullptr)
    , reader_(nullptr)
    , type_(new TargetsPubSubType())
    , samples_received_(0)
    , stop_(false)
{
    // Create the participant
    DomainParticipantQos pqos = PARTICIPANT_QOS_DEFAULT;
    pqos.name("Targets_sub_participant");
    factory_ = DomainParticipantFactory::get_shared_instance();
    participant_ = factory_->create_participant(domain_id, pqos, nullptr, StatusMask::none());
    if (participant_ == nullptr)
    {
        throw std::runtime_error("Targets Participant initialization failed");
    }

    // Register the type
    type_.register_type(participant_);

    // Create the subscriber
    SubscriberQos sub_qos = SUBSCRIBER_QOS_DEFAULT;
    participant_->get_default_subscriber_qos(sub_qos);
    subscriber_ = participant_->create_subscriber(sub_qos, nullptr, StatusMask::none());
    if (subscriber_ == nullptr)
    {
        throw std::runtime_error("Targets Subscriber initialization failed");
    }

    // Create the topic
    TopicQos topic_qos = TOPIC_QOS_DEFAULT;
    participant_->get_default_topic_qos(topic_qos);
    topic_ = participant_->create_topic("TargetsTopic", type_.get_type_name(), topic_qos);
    if (topic_ == nullptr)
    {
        throw std::runtime_error("Targets Topic initialization failed");
    }

    // Create the reader
    DataReaderQos reader_qos = DATAREADER_QOS_DEFAULT;
    subscriber_->get_default_datareader_qos(reader_qos);
    reader_qos.reliability().kind = ReliabilityQosPolicyKind::RELIABLE_RELIABILITY_QOS;
    reader_qos.durability().kind = DurabilityQosPolicyKind::TRANSIENT_LOCAL_DURABILITY_QOS;
    reader_qos.history().kind = HistoryQosPolicyKind::KEEP_ALL_HISTORY_QOS;
    reader_ = subscriber_->create_datareader(topic_, reader_qos, this, StatusMask::all());
    if (reader_ == nullptr)
    {
        throw std::runtime_error("Targets DataReader initialization failed");
    }
}

TargetsSubscriberApp::~TargetsSubscriberApp()
{
    if (nullptr != participant_)
    {
        participant_->delete_contained_entities();
        factory_->delete_participant(participant_);
    }
}

void TargetsSubscriberApp::on_subscription_matched(DataReader* /*reader*/, const SubscriptionMatchedStatus& info)
{
    if (info.current_count_change == 1)
    {
//print not log
        std::cout << "Targets Subscriber matched." << std::endl;
    }
    else if (info.current_count_change == -1)
    {
//print not log
        std::cout << "Targets Subscriber unmatched." << std::endl;
    }
}

void TargetsSubscriberApp::on_data_available(DataReader* reader)
{
    Targets sample_;
    SampleInfo info;
    const char* fifo_path = "/tmp/target_generator_fifo_sub_0";
    
    struct stat st;
    if (stat(fifo_path, &st) != 0)
    {
        if (mkfifo(fifo_path, 0666) < 0)
        {
            perror("Failed to create FIFO");
            exit(1);
        }
    }
    
    int fd = open(fifo_path, O_WRONLY);
    if (fd < 0)
    {
        perror("Failed to open FIFO");
        exit(1);
    }


    Targets_gen targets_from_publisher = {0};
    
    while ((!is_stopped()) && (RETCODE_OK == reader->take_next_sample(&sample_, &info)))
    {
        targets_from_publisher.num_targets = sample_.targets_number();
        for (int i = 0; i < sample_.targets_number(); i++)
        {
            targets_from_publisher.targets[i][0] = sample_.targets_x()[i];
            targets_from_publisher.targets[i][1] = sample_.targets_y()[i];
        }
        write(fd, &targets_from_publisher, sizeof(Targets_gen));

        if ((info.instance_state == ALIVE_INSTANCE_STATE) && info.valid_data)
        {
            std::cout << "Sample '" << std::to_string(++samples_received_) << "' RECEIVED" << std::endl;
        }
    }
}

void TargetsSubscriberApp::run()
{
    std::unique_lock<std::mutex> lck(terminate_cv_mtx_);
    terminate_cv_.wait(lck, [this]
    {
        return is_stopped();
    });
}

bool TargetsSubscriberApp::is_stopped()
{
    return stop_.load();
}

void TargetsSubscriberApp::stop()
{
    stop_.store(true);
    terminate_cv_.notify_all();
}
