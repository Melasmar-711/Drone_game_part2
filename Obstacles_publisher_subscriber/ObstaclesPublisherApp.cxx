#include "ObstaclesPublisherApp.hpp"

#include <condition_variable>
#include <csignal>
#include <stdexcept>
#include <thread>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#include "ObstaclesPubSubTypes.hpp"
#include <fastdds/rtps/transport/TCPv4TransportDescriptor.hpp>
using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::rtps;

ObstaclesPublisherApp::ObstaclesPublisherApp(
        const int& domain_id)
    : factory_(nullptr)
    , participant_(nullptr)
    , publisher_(nullptr)
    , topic_(nullptr)
    , writer_(nullptr)
    , type_(new ObstaclesPubSubType())
    , matched_(0)
    , samples_sent_(0)
    , stop_(false)
{
    DomainParticipantQos pqos = PARTICIPANT_QOS_DEFAULT;
   // pqos.transport().use_builtin_transports = false;

    //std::shared_ptr<TCPv4TransportDescriptor> tcp_transport = std::make_shared<TCPv4TransportDescriptor>();
    //tcp_transport->sendBufferSize = 131072;
    //tcp_transport->receiveBufferSize = 131072;
    //tcp_transport->add_listener_port(5200);

   // pqos.transport().user_transports.push_back(tcp_transport);

   // Locator_t initial_peer;
    //initial_peer.kind = LOCATOR_KIND_TCPv4;
   // IPLocator::setIPv4(initial_peer, "127.0.0.1");
   // initial_peer.port = 5201;
   // pqos.wire_protocol().builtin.initialPeersList.push_back(initial_peer);

    //pqos.name("Obstacles_pub_participant");
    
    factory_ = DomainParticipantFactory::get_shared_instance();
    participant_ = factory_->create_participant(domain_id, pqos, nullptr, StatusMask::none());
    if (participant_ == nullptr)
    {
        throw std::runtime_error("Obstacles Participant initialization failed");
    }

    type_.register_type(participant_);

    PublisherQos pub_qos = PUBLISHER_QOS_DEFAULT;
    participant_->get_default_publisher_qos(pub_qos);
    publisher_ = participant_->create_publisher(pub_qos, nullptr, StatusMask::none());
    if (publisher_ == nullptr)
    {
        throw std::runtime_error("Obstacles Publisher initialization failed");
    }

    TopicQos topic_qos = TOPIC_QOS_DEFAULT;
    participant_->get_default_topic_qos(topic_qos);
    topic_ = participant_->create_topic("ObstaclesTopic", type_.get_type_name(), topic_qos);
    if (topic_ == nullptr)
    {
        throw std::runtime_error("Obstacles Topic initialization failed");
    }

    DataWriterQos writer_qos = DATAWRITER_QOS_DEFAULT;
    publisher_->get_default_datawriter_qos(writer_qos);
    writer_qos.reliability().kind = ReliabilityQosPolicyKind::RELIABLE_RELIABILITY_QOS;
    writer_qos.durability().kind = DurabilityQosPolicyKind::TRANSIENT_LOCAL_DURABILITY_QOS;
    writer_qos.history().kind = HistoryQosPolicyKind::KEEP_ALL_HISTORY_QOS;
    writer_ = publisher_->create_datawriter(topic_, writer_qos, this, StatusMask::all());
    if (writer_ == nullptr)
    {
        throw std::runtime_error("Obstacles DataWriter initialization failed");
    }
}

ObstaclesPublisherApp::~ObstaclesPublisherApp()
{
    if (nullptr != participant_)
    {
        participant_->delete_contained_entities();
        factory_->delete_participant(participant_);
    }
}

void ObstaclesPublisherApp::on_publication_matched(
        DataWriter* /*writer*/,
        const PublicationMatchedStatus& info)
{
    if (info.current_count_change == 1)
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            matched_ = info.current_count;
        }
        std::cout << "Obstacles Publisher matched." << std::endl;
        cv_.notify_one();
    }
    else if (info.current_count_change == -1)
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            matched_ = info.current_count;
        }
        std::cout << "Obstacles Publisher unmatched." << std::endl;
    }
}

void ObstaclesPublisherApp::run()
{
    const char *fifo_path = "/tmp/obstacle_generator_fifo_pub_0";
    struct stat st;
    if (stat(fifo_path, &st) != 0) {
        if (mkfifo(fifo_path, 0666) < 0) {
            perror("Failed to create FIFO");
            exit(1);
        }
    }

    int fd = open(fifo_path, O_RDONLY);
    if (fd < 0) {
        perror("Failed to open FIFO");
        exit(1);
    }
    std::cout << "Opened the FIFO: " << fifo_path << std::endl;
    
    while (!is_stopped())
    {
        Obstacle_gen obstacles_from_publisher = {0};
        ssize_t bytes_read = read(fd, &obstacles_from_publisher, sizeof(obstacles_from_publisher));

        if (bytes_read != 0)
        {
            if (publish(&obstacles_from_publisher))
            {
                std::cout << "Sample '" << std::to_string(++samples_sent_) << "' SENT" << std::endl;
            }
        }

        std::unique_lock<std::mutex> period_lock(mutex_);
        cv_.wait_for(period_lock, std::chrono::milliseconds(10000), [this]()
                {
                    return is_stopped();
                });
    }
}

bool ObstaclesPublisherApp::publish(Obstacle_gen *obstacles_from_publisher)
{
    bool ret = false;
    std::unique_lock<std::mutex> matched_lock(mutex_);
    cv_.wait(matched_lock, [&]() { return ((matched_ > 0) || is_stopped()); });

    std::vector<int32_t> x_values, y_values;
    for (int i = 0; i < obstacles_from_publisher->num_obstacles; i++) {
        x_values.push_back(obstacles_from_publisher->obstacles[i][0]);
        y_values.push_back(obstacles_from_publisher->obstacles[i][1]);
    }

    if (!is_stopped())
    {
        Obstacles sample_;
        sample_.obstacles_number(obstacles_from_publisher->num_obstacles);
        sample_.obstacles_x(x_values);
        sample_.obstacles_y(y_values);
        ret = (RETCODE_OK == writer_->write(&sample_));
    }
    return ret;
}

bool ObstaclesPublisherApp::is_stopped()
{
    return stop_.load();
}

void ObstaclesPublisherApp::stop()
{
    stop_.store(true);
    cv_.notify_one();
}
