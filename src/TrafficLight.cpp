#include <iostream>
#include <random>
#include <future>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */
using std::mutex;
using std::unique_lock;
using std::condition_variable;
using std::lock_guard;
using std::thread;


template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
    unique_lock<mutex> lock (_mtx);
    _cond_var.wait(lock, [this]() {
        return !_queue.empty();
    });
    T t = std::move(_queue.back());
    _queue.pop_back();
    //std::cout << "queue size=" << _queue.size() << std::endl;
    return t;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    lock_guard<mutex> lock(_mtx);
    if(!_queue.empty()) { _queue.pop_back(); }
    _queue.push_back(std::move(msg));
    _cond_var.notify_one();
    //std::cout << "queue size=" << _queue.size() << std::endl;
}

/* Implementation of class "TrafficLight" */

 
TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    TrafficLightPhase phase;
    while(true) {
        phase = _mq.receive();
        //std::cout << "TrafficLight Light is " << getTrafficLightPhaseAsString(phase) << std::endl;
        if(phase == TrafficLightPhase::green) {
            return;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

std::string TrafficLight::getTrafficLightPhaseAsString(TrafficLightPhase phase)
{
    return phase == TrafficLightPhase::green ? "green" : "red";
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. 
    // To do this, use the thread queue in the base class.     
    threads.emplace_back(thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases() {
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles.
    auto prev_loop_start { std::chrono::high_resolution_clock::now() };
    const auto ms4 { std::chrono::milliseconds(4000) };
    const auto ms6 { std::chrono::milliseconds(6000) };
    while(true) {        
        auto loop_duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - prev_loop_start);
        if(loop_duration > ms4 && loop_duration < ms6) {
            
            // wait an additional 0 to 2 seconds.
            std::random_device rd;
            std::mt19937 eng(rd());
            std::uniform_int_distribution<> distr(0, 2);
            int wait = distr(eng);
            std::this_thread::sleep_for(std::chrono::seconds(wait));

            prev_loop_start = std::chrono::high_resolution_clock::now();
            _currentPhase = _currentPhase == TrafficLightPhase::green ? TrafficLightPhase::red : TrafficLightPhase::green;
            _mq.send(std::move(_currentPhase));
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    } 
}
