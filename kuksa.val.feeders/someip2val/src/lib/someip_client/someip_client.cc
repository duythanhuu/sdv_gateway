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

#ifndef VSOMEIP_ENABLE_SIGNAL_HANDLING
// #  warning VSOMEIP_ENABLE_SIGNAL_HANDLING is not defined!
#endif

#include <csignal>
#include <chrono>
#include <condition_variable>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <thread>

#include <vsomeip/vsomeip.hpp>

#include "sample_ids.h"
#include "someip_client.h"

namespace sdv {
namespace someip {

/*** LOG helpers */
#define LEVEL_TRC   3
#define LEVEL_DBG   2
#define LEVEL_INF   1
#define LEVEL_ERR   0

#define MODULE_PREFIX   "# SomeIPClient<"

#define LOG_TRACE   if (config_.debug >= LEVEL_TRC) std::cout << MODULE_PREFIX << name_ << ">::" << __func__ << ": [trace] "
#define LOG_DEBUG   if (config_.debug >= LEVEL_DBG) std::cout << MODULE_PREFIX << name_ << ">::" << __func__ << ": [debug] "
#define LOG_INFO    if (config_.debug >= LEVEL_INF) std::cout << MODULE_PREFIX << name_ << ">::" << __func__ << ": [info] "
#define LOG_ERROR   if (config_.debug >= LEVEL_ERR) std::cerr << MODULE_PREFIX << name_ << ">::" << __func__ << ": [error] "

SomeIPClient::SomeIPClient(SomeIPConfig _config, message_callback_t _callback) :
    config_(_config),
    callback_(_callback),
    stop_requested_(false),
    initialized_(false),
    use_req_(false),
    req_service_available(false)
{
    use_tcp_ = config_.use_tcp;
    name_ = config_.app_name.empty() ? "UNKNOWN" : config_.app_name;
    // log_prefix_ = "*** [SomeIPClient/" + name_ + "] "

    // FIXME: replace <service_> with config_.<service> everywhere...
    service_ = config_.service;
    instance_ = config_.instance;
    service_major_ = config_.service_major;
    service_minor_ = config_.service_minor;
    event_group_ = config_.event_group;
    event_ = config_.event;

    use_req_ = config_.req.use_req;
    // req_config_ = config_.req;
    req_service_ = config_.req.service;
    req_instance_ = config_.req.instance;
    req_service_major_ = config_.req.service_major;
    req_service_minor_ = config_.req.service_minor;
    req_method_ = config_.req.method;

    app_ = vsomeip::runtime::get()->create_application(name_);
    if (callback_ == nullptr) {
        LOG_ERROR << "Warning, Some/IP callback is not set!" << std::endl;
    }
}

SomeIPClient::~SomeIPClient() {
    LOG_TRACE << "called. " << std::endl;
    Shutdown();
    LOG_TRACE << "done." << std::endl;
}

bool SomeIPClient::init() {
    // WARNING: init() may call std::exit() in some cases, it probably would deadlock on app->stop()
    if (!app_->init()) {
        LOG_ERROR << "Couldn't initialize application: " << app_->get_name() << std::endl;
        return false;
    }
    // important! handles stop() from app->init()
    initialized_ = true;
    name_ = app_->get_name(); // app name is valid here

    LOG_INFO << "Client settings "
            << "{ service:0x" << std::hex << std::setfill('0') << std::setw(4) << service_
            << ", instance:0x" << std::hex << std::setfill('0') << std::setw(4) << instance_
            << ", ver " << std::dec << (int)service_major_ << "." << (int)service_minor_
            << ", group_:0x" << std::hex << std::setfill('0') << std::setw(4) << event_group_
            << ", event:0x" << std::hex << std::setfill('0') << std::setw(4) << event_
            << "} [protocol=" << (use_tcp_ ? "TCP" : "UDP") << "]"
            << std::endl;

    app_->register_state_handler(
            std::bind(&SomeIPClient::on_state, this,
                    std::placeholders::_1));

    app_->register_message_handler(
            vsomeip::ANY_SERVICE, vsomeip::ANY_INSTANCE, vsomeip::ANY_METHOD,
            std::bind(&SomeIPClient::on_message, this,
                    std::placeholders::_1));

    app_->register_availability_handler(
            vsomeip_v3::ANY_SERVICE, vsomeip_v3::ANY_INSTANCE,
            std::bind(&SomeIPClient::on_availability,
                        this,
                        std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
        );

    init_event_service(service_, instance_, event_group_, event_, service_major_, service_minor_);
    return true;
}


void SomeIPClient::init_event_service(
        vsomeip::service_t service,
        vsomeip::instance_t instance,
        vsomeip::eventgroup_t event_group,
        vsomeip::event_t event,
        vsomeip::major_version_t service_major,
        vsomeip::minor_version_t service_minor) {

    std::set<vsomeip::eventgroup_t> its_groups;
    its_groups.insert(event_group);

    vsomeip::event_type_e event_type = vsomeip::event_type_e::ET_FIELD;
    vsomeip::reliability_type_e reliability_type =
            (use_tcp_ ? vsomeip::reliability_type_e::RT_RELIABLE :
                    vsomeip::reliability_type_e::RT_UNRELIABLE);
    LOG_INFO << "Request event ["
            << std::hex << std::setfill('0') << std::setw(4) << service << "."
            << std::hex << std::setfill('0') << std::setw(4) << instance
            << "], event:0x" << std::hex << std::setfill('0') << std::setw(4) << event
            << ", event_type:" << std::dec << (int)event_type
            << ", reliability:" << std::dec << (int)reliability_type
            << std::endl;
    app_->request_event(
            service, instance, event,
            its_groups,
            event_type,
            reliability_type);

    LOG_INFO << "Subscribing ["
        << std::hex << std::setfill('0') << std::setw(4) << service << "."
        << std::hex << std::setfill('0') << std::setw(4) << instance
        << "] ver." << std::dec << (int)service_major
        << ", event_group:0x"
        << std::hex << std::setfill('0') << std::setw(4) << event_group
        << ", event:0x"
        << std::hex << std::setfill('0') << std::setw(4) << event
        << std::endl;

    app_->subscribe(service, instance, event_group, service_major, event);
}

/**
 * @brief SOMEIP main thread, blocking call, should be called in a thread
 *
 * @return true if initialized
 */
bool SomeIPClient::Run() {
    if (init()) {
        start();
        return true;
    }
    return false;
}

void SomeIPClient::Shutdown() {
    std::unique_lock<std::mutex> its_lock(stop_mutex_);
    LOG_TRACE << "Shutdown() / stop_requested_=" << stop_requested_
            << ", initialized_=" << initialized_ << std::endl;
    if (!stop_requested_) {
        stop_requested_ = true;
        LOG_DEBUG << "Shutting down..." << std::endl;
        stop();
    }
}

void SomeIPClient::start() {
    LOG_INFO << "Starting..." << std::endl;
    app_->start();
    LOG_TRACE << "done." << std::endl;
}

/**
 * @brief Shuts down someip client (may be called from signal handler)
 * May cause problems if someip is compiled with -DVSOMEIP_ENABLE_SIGNAL_HANDLING
 */
void SomeIPClient::stop() {
    LOG_INFO << "Stopping..." << std::endl;

    req_service_available = false;
    req_condition_.notify_all();

    app_->clear_all_handler();
    LOG_DEBUG << "Releasing Event service..." << std::endl;
    app_->unsubscribe(service_, instance_, event_group_, event_);
    app_->release_event(service_, instance_, event_);
    app_->release_service(service_, instance_);

    if (use_req_) {
        LOG_DEBUG << "Releasing Reqest/Response service..." << std::endl;
        app_->release_service(req_service_, req_instance_);
    }
    if (!initialized_) {
        LOG_INFO << "Not stopping partially initialized app!" << std::endl;
    } else { // experimental code, stop may hung, without it rely on app destructor
        if (config_.debug > 2) {
            LOG_INFO << "app->stop()" << std::endl;
        }
        app_->stop();
        if (config_.debug > 0) {
            LOG_INFO << "stopped." << std::endl;
        }
    }
}

const SomeIPConfig SomeIPClient::GetConfig() const {
    return (const SomeIPConfig)config_;
}

void SomeIPClient::on_state(vsomeip::state_type_e _state) {
    if (config_.debug > 0) {
        LOG_INFO << "State "
            << (_state == vsomeip::state_type_e::ST_REGISTERED ? "REGISTERED" : "DEREGISTERED")
            << std::endl;
    }
    if (_state == vsomeip::state_type_e::ST_REGISTERED) {
        LOG_INFO << "Requesting Event Service ["
            << std::setw(4) << std::setfill('0') << std::hex << service_
            << "."
            << std::setw(4) << std::setfill('0') << std::hex << instance_
            << " v"
            << std::dec << (int)service_major_
            << "."
            << (int)service_minor_ << "]" << std::endl;
        app_->request_service(service_, instance_, service_major_, service_minor_);

        if (use_req_) {
            LOG_INFO << "Requesting Request/Response Service ["
                << std::setw(4) << std::setfill('0') << std::hex << req_service_
                << "."
                << std::setw(4) << std::setfill('0') << std::hex << req_instance_
                << " v"
                << std::dec << (int)req_service_major_
                << "."
                << (int)req_service_minor_ << "]" << std::endl;
            app_->request_service(req_service_, req_instance_,
                    req_service_major_, req_service_minor_);
        }
    }
}

void SomeIPClient::on_availability(vsomeip::service_t _service, vsomeip::instance_t _instance, bool _is_available) {
    LOG_INFO << "Service ["
            << std::setw(4) << std::setfill('0') << std::hex << _service << "."
            << std::setw(4) << std::setfill('0') << std::hex << _instance
            << "] is "
            << (_is_available ? "available." : "NOT available.")
            << std::endl;

    // if (_service == service_ && _instance == instance_) {
    // } else
    if (_service == req_service_ && _instance == req_instance_) {
        LOG_INFO << "Notify Request Service available" << std::endl;
        req_service_available = _is_available;
        req_condition_.notify_one();
    }
}

void SomeIPClient::on_message(const std::shared_ptr<vsomeip::message> &_response) {
    std::shared_ptr<vsomeip::payload> its_payload = _response->get_payload();
    if (config_.debug > 2) {
        std::stringstream its_message;
        its_message << "Received a "
                << message_type_to_string(_response->get_message_type())
                << " for Event ["
                << std::setw(4) << std::setfill('0') << std::hex
                << _response->get_service() << "."
                << std::setw(4) << std::setfill('0') << std::hex
                << _response->get_instance() << "."
                << std::setw(4) << std::setfill('0') << std::hex
                << _response->get_method() << "] v"
                << _response->get_interface_version()
                << " to Client/Session ["
                << std::setw(4) << std::setfill('0') << std::hex
                << _response->get_client() << "/"
                << std::setw(4) << std::setfill('0') << std::hex
                << _response->get_session()
                << "] = ";

        its_message << "(" << std::dec << its_payload->get_length() << ") ";
        for (uint32_t i = 0; i < its_payload->get_length(); ++i) {
            its_message << std::hex << std::setw(2) << std::setfill('0')
                << (int) its_payload->get_data()[i] << " ";
        }
        its_message << std::endl;
        LOG_INFO << its_message.str();
    }
    // callback should handle if it knows service:instance:event and avoid parsing wrong events
    if (callback_ != nullptr) {
        int rc = callback_(_response->get_service(),
                _response->get_instance(),
                _response->get_method(),
                its_payload->get_data(),
                its_payload->get_length());
        if (rc) {
            LOG_ERROR << "WARNING! callback failed decoding "
                    << its_payload->get_length() << " bytes" << std::endl;
        }
    }
}

int SomeIPClient::SendRequest(vsomeip::service_t req_service, vsomeip::instance_t req_instance,
        vsomeip::method_t req_method, std::vector<vsomeip::byte_t> payload)
{
    std::unique_lock<std::mutex> cond_lock(req_mutex_);
    if (!use_req_) {
        LOG_ERROR << "Requst Service not enabled!" << std::endl;
        return -1;
    }
    if (!req_service_available) {
        LOG_DEBUG << "Waiting for Request Service..." << std::endl;
        auto deadline = std::chrono::system_clock::now() + std::chrono::seconds(5);
        req_condition_.wait_until(cond_lock, deadline);
        if (!req_service_available) {
            LOG_ERROR << "Requst Service not available! Attempt sending anyway..." << std::endl;
            // return -2;
        }
    }
    // Create a new request
    std::shared_ptr<vsomeip::message> rq = vsomeip::runtime::get()->create_request();
    // Set the VSS service as target of the request
    rq->set_service(req_service);
    rq->set_instance(req_instance);
    rq->set_method(req_method);
    // rq->set_interface_version(req_major); // not needed, set to vsomeip_v3::sd::interface_version(0x1)

    std::shared_ptr<vsomeip::payload> pl = vsomeip::runtime::get()->create_payload();
    pl->set_data(payload);
    rq->set_payload(pl);
    // Send the request to the service. Response will be delivered to the
    // registered message handler
    std::stringstream ss;
    ss << "### Sending Request to ["
            << std::setw(4) << std::setfill('0') << std::hex << req_service << "."
            << std::setw(4) << std::setfill('0') << std::hex << req_instance << "."
            << std::setw(4) << std::setfill('0') << std::hex << req_method << "]";

    if (config_.debug > 0) {
        ss << " with payload: " << hexdump(payload.data(), payload.size());
    }
    ss << std::endl;
    LOG_INFO << ss.str();
    app_->send(rq);
    LOG_TRACE << "### Request sent." << std::endl;
    // TODO: wait for reply?
    return 0;
}



std::shared_ptr<SomeIPClient> SomeIPClient::createInstance(SomeIPConfig _config, message_callback_t callback)
{
    return std::make_shared<SomeIPClient>(_config, callback);
}

SomeIPConfig SomeIPClient::createEnvConfig() {
    // SomeIPConfig config = SomeIPClient::createDefaultConfig();
    SomeIPConfig config;
    config.debug = getEnvironmentInt("SOMEIP_CLI_DEBUG", 1);

    config.use_tcp = (getEnvironmentInt("SOMEIP_CLI_TCP", 0) == 1);
    config.app_config = getEnvironmentStr("VSOMEIP_CONFIGURATION", "");
    config.app_name =  getEnvironmentStr("VSOMEIP_APPLICATION_NAME", "UNKNOWN");

    config.service = getEnvironmentInt("SOMEIP_CLI_SERVICE", SAMPLE_SERVICE_ID);
    config.instance = getEnvironmentInt("SOMEIP_CLI_INSTANCE", SAMPLE_INSTANCE_ID);

    config.event_group = getEnvironmentInt("SOMEIP_CLI_EVENTGROUP", SAMPLE_EVENTGROUP_ID);
    config.event = getEnvironmentInt("SOMEIP_CLI_EVENT", SAMPLE_EVENT_ID);

    config.service_major = getEnvironmentInt("SOMEIP_CLI_MAJOR", SAMPLE_SERVICE_MAJOR);
    config.service_minor = getEnvironmentInt("SOMEIP_CLI_MINOR", SAMPLE_SERVICE_MINOR);

    // req/response support (disabled by default)
    config.req.use_req = (getEnvironmentInt("SOMEIP_CLI_REQ", 0) == 1);
    config.req.service = getEnvironmentInt("SOMEIP_CLI_REQ_SERVICE", SAMPLE_INVALID_VALUE);
    config.req.instance = getEnvironmentInt("SOMEIP_CLI_REQ_INSTANCE", SAMPLE_INVALID_VALUE);
    config.req.service_major = getEnvironmentInt("SOMEIP_CLI_REQ_MAJOR", vsomeip::DEFAULT_MAJOR);
    config.req.service_minor = getEnvironmentInt("SOMEIP_CLI_REQ_MINOR", vsomeip::DEFAULT_MINOR);
    config.req.method = getEnvironmentInt("SOMEIP_CLI_REQ_METHOD", SAMPLE_INVALID_VALUE);
    // TODO: validate req config?

    return config;
}


std::string message_type_to_string(vsomeip::message_type_e msg_type) {
    switch (msg_type) {
        case vsomeip::message_type_e::MT_ERROR:
            return "Error";
         case vsomeip::message_type_e::MT_ERROR_ACK:
            return "Error/ack";
         case vsomeip::message_type_e::MT_NOTIFICATION:
            return "Notification";
         case vsomeip::message_type_e::MT_NOTIFICATION_ACK:
            return "Notification/ack";
         case vsomeip::message_type_e::MT_REQUEST:
            return "Request";
         case vsomeip::message_type_e::MT_REQUEST_ACK:
            return "Request/ack";
         case vsomeip::message_type_e::MT_REQUEST_NO_RETURN:
            return "Request/no_ret";
         case vsomeip::message_type_e::MT_REQUEST_NO_RETURN_ACK:
            return "Request/no_ret/ack";
         case vsomeip::message_type_e::MT_RESPONSE:
            return "Response";
         case vsomeip::message_type_e::MT_RESPONSE_ACK:
            return "Response/ack";
        default:
            std::stringstream its_message;
            its_message << "Unknown <0x" << std::hex << (int)msg_type << ">";
            return its_message.str();
    }
}

int getEnvironmentInt(const std::string &envVar, int defaultValue, bool dump) {
    const char* envValue = ::getenv(envVar.c_str());
    if (envValue) {
        try {
            int result = std::stoi(std::string(envValue), nullptr, 0);
            if (dump) {
                std::cout << __func__ << " [env] " << envVar << " := " << envValue << std::endl;
            }
            return result;
        } catch(std::exception const& ex) {
            std::cerr << __func__ << " Invalid integer for " << envVar
                    << " : " << envValue
                    << ", (" << ex.what() << ")" << std::endl;
        }
    }
    return defaultValue;
}

std::string getEnvironmentStr(const std::string &envVar, const std::string &defaultValue, bool dump) {
    const char* value = ::getenv(envVar.c_str());
    if (value) {
        if (dump) {
            std::cout << __func__ << " [env] " << envVar << " := " << value << std::endl;
        }
        return std::string(value);
    }
    return defaultValue;
}

std::string hexdump(uint8_t *buf, size_t size) {
    std::stringstream ss;
    // ss << "(" << std::dec << size << ") ";
    for (uint32_t i = 0; i < size; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int) buf[i];
        if (i < size - 1) {
            ss << " ";
        }
    }
    return ss.str();
}

}  // namespace someip
}  // namespace sdv
