/********************************************************************************
* Copyright (c) 2022 Contributors to the Eclipse Foundation
*
* See the NOTICE file(s) distributed with this work for additional
* information regarding copyright ownership.
*
* This program and the accompanying materials are made available under the
* terms of the Apache License 2.0 which is available at
* http://www.apache.org/licenses/LICENSE-2.0
*
* SPDX-License-Identifier: Apache-2.0
********************************************************************************/
/**
 * @file      data_broker_feeder.cc
 * @brief     File contains implementation of the generic class DataBrokerFeeder.
 *
 */
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdlib>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include <grpcpp/grpcpp.h>

#include "data_broker_feeder.h"
#include "sdv/databroker/v1/collector.grpc.pb.h"

#include "simple_log.h"

// declare logger in root namespace
LOGGER_STATIC_INIT("# DataBrokerFeeder::");

namespace sdv {
namespace broker_feeder {

using DatapointId = google::protobuf::int32;

class DataBrokerFeederImpl final:
    public DataBrokerFeeder
{
private:
    const GrpcMetadata grpc_metadata_;
    const DatapointConfiguration dp_config_;
    DatapointValues stored_values_;
    google::protobuf::Map<std::string, DatapointId> id_map_;

    std::atomic<bool> feeder_active_;
    std::string broker_addr_;
    std::mutex stored_values_mutex_;
    std::condition_variable feeder_thread_sync_;

    std::shared_ptr<CollectorClient> client_;
    std::unique_ptr<grpc::ClientContext> subscriber_context_;

   public:
    DataBrokerFeederImpl(std::shared_ptr<CollectorClient> client, DatapointConfiguration&& dp_config)
        : client_(client)
        , dp_config_(std::move(dp_config))
        , feeder_active_(true)
    {
        LOGGER_SET_LEVEL_ENV("DBF_DEBUG", LEVEL_INF);
    }

    ~DataBrokerFeederImpl() { Shutdown(); }

    void Run() override {
        /* This thread is responsible for establishing a connection to the data broker.
         * Once connection is present, it starts registering the data points (metadata)
         * with the broker and feeds the initial values (plus possible already stored values
         * to the broker.
         * Afterwards it is forwarding values stored by the feeding medthods and trys
         * re-establishing a lost connection to the broker.
         */
        while (feeder_active_) {
            LOG_INFO << "Connecting to data broker [" << client_->GetBrokerAddr() << "] ..." << std::endl;
            auto deadline = std::chrono::system_clock::now() + std::chrono::seconds(5);
            client_->WaitForConnected(deadline);
            if (client_->Connected()) {
                LOG_INFO << "connected to data broker." << std::endl;
            }
            if (feeder_active_ && client_->Connected()) {
                if (!registerDatapoints()) {
                    // don't attempt to feed values (too often) if registration status was an error
                    std::this_thread::sleep_for(std::chrono::seconds(5));
                    continue;
                }
            }

            bool also_feed_initial_values = true;
            while (feeder_active_ && client_->Connected()) {
                feedStoredValues(also_feed_initial_values);
                also_feed_initial_values = false;

                if (feeder_active_ && client_->Connected()) {
                    std::unique_lock<std::mutex> lock(stored_values_mutex_);
                    if (stored_values_.empty()) {
                        feeder_thread_sync_.wait(lock);
                    }
                }
            }
        }
    }

    void Shutdown() override {
        if (feeder_active_) {
            LOG_INFO << "Waiting for feeder to stop ..." << std::endl;
            {
                std::unique_lock<std::mutex> lock(stored_values_mutex_);
                stored_values_.clear();
                feeder_active_ = false;
            }
            feeder_thread_sync_.notify_all();
            LOG_INFO << "Feeder stopped." << std::endl;
        }

        if (subscriber_context_) {
            subscriber_context_->TryCancel();
        }
    }

    /** Feed a set ("batch") of datapoint values to the data broker.
     *  If the data broker is currently not connected or another "recoverable"error occurs, the passed
     *  values are stored by the feeder and tried being send, when the connection to the broker could be
     *  established (again).
     */
    void FeedValues(const DatapointValues& values) override
    {
        if (feeder_active_) {
            LOG_TRACE << "Enqueue " << values.size() << " values." << std::endl;
            std::unique_lock<std::mutex> lock(stored_values_mutex_);
            storeValues(values);
            feeder_thread_sync_.notify_all();
        }
    }

    /** Feed a single datapoint value to the data broker.
     *  (@see FeedValues)
     */
    void FeedValue(const std::string& name, const sdv::databroker::v1::Datapoint& value) override
    {
        if (feeder_active_) {
            LOG_TRACE << "Enqueue value: { " << value.ShortDebugString() << " } " << std::endl;
            std::unique_lock<std::mutex> lock(stored_values_mutex_);
            storeValue(name, value);
            feeder_thread_sync_.notify_all();
        }
    }

private:
    /** Add the passed values to the stored values (possibly overwriting already stored values) */
    void storeValues(const DatapointValues& values) {
        for (const auto& value : values) {
            storeValue(value.first, value.second);
        }
    }

    /** Add the passed value to the stored values (possibly overwriting an already stored value) */
    void storeValue(const std::string& name, const sdv::databroker::v1::Datapoint& value) {
        stored_values_[name] = value;
    }

    /** Register the data points (metadata) passed to the c-tor with the data broker.
     */
    bool registerDatapoints() {
        LOG_DEBUG << "Registering " << dp_config_.size() << " datapoints." << std::endl;
        sdv::databroker::v1::RegisterDatapointsRequest request;
        for (const auto& metadata : dp_config_) {
            ::sdv::databroker::v1::RegistrationMetadata reg_data;
            reg_data.set_name(metadata.name);
            reg_data.set_data_type(metadata.data_type);
            reg_data.set_change_type(metadata.change_type);
            reg_data.set_description(metadata.description);
            request.mutable_list()->Add(std::move(reg_data));
        }

        auto context = client_->createClientContext();
        sdv::databroker::v1::RegisterDatapointsReply reply;
        grpc::Status status = client_->RegisterDatapoints(context.get(), request, &reply);
        if (status.ok()) {
            LOG_INFO << "Datapoints registered." << std::endl;
            id_map_ = std::move(*reply.mutable_results());
            for (const auto& name_to_id : id_map_) {
                LOG_DEBUG << "    '" << name_to_id.first << "' -> " << std::dec << name_to_id.second << std::endl;
            }
            return true;
        } else {
            client_->handleGrpcError(status, "DataBrokerFeederImpl::registerDatapoints");
            return false;
        }
    }

    /** Feed stored and - on demand - initial values to the data broker.
     *  If for a datapoint an initial as well as a stored value is present, the stored on gets precedence.
     */
    void feedStoredValues(bool feed_initial_values = false) {
        DatapointValues values_to_feed;
        {
            std::unique_lock<std::mutex> lock(stored_values_mutex_);
            values_to_feed.swap(stored_values_);
        }
        if (feed_initial_values) {
            for (const auto& metadata : dp_config_) {
                values_to_feed.insert(std::make_pair(metadata.name, metadata.initial_value));
            }
        }
        bool successfully_sent = feedToBroker(values_to_feed);
        if (!successfully_sent) {
            restoreValues(std::move(values_to_feed));
        }
    }

    /** Feed the passed values to the data broker. */
    bool feedToBroker(const DatapointValues& values_to_feed) {
        LOG_DEBUG << "Feeding " << std::dec << values_to_feed.size() << " values." << std::endl;
        sdv::databroker::v1::UpdateDatapointsRequest request;
        for (const auto& value : values_to_feed) {
            auto iter = id_map_.find(value.first);
            if (iter != id_map_.end()) {
                auto id = iter->second;
                (*request.mutable_datapoints())[id] = value.second;
                LOG_DEBUG <<"    '"<< value.first <<"' ("<< id <<") of type "
                    << std::dec << (int)value.second.value_case()
                    << ", value: { " << value.second.ShortDebugString() << " }"
                    << std::endl;
            } else {
                LOG_ERROR <<"    Unknown name '" << value.first << "'!" << std::endl;
            }
        }

        auto context = client_->createClientContext();
        sdv::databroker::v1::UpdateDatapointsReply reply;
        grpc::Status status = client_->UpdateDatapoints(context.get(), request, &reply);
        if (status.ok()) {
            return true;
        }
        client_->handleGrpcError(status, "DataBrokerFeederImpl::feedToBroker");
        return false;
    }

    /** Re-store values on a feeding error; already contained values are rated newer and are not overwritten */
    void restoreValues(DatapointValues&& values) {
        std::unique_lock<std::mutex> lock(stored_values_mutex_);
        stored_values_.insert(values.begin(), values.end());
    }

};

std::shared_ptr<DataBrokerFeeder> DataBrokerFeeder::createInstance(std::shared_ptr<CollectorClient> client,
                                                                    DatapointConfiguration&& dpConfig) {
    return std::make_shared<DataBrokerFeederImpl>(client, std::move(dpConfig));
}

}  // namespace broker_feeder
}  // namespace sdv
