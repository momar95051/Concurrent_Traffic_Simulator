#include <iostream>
#include <random>
#include<mutex>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */
using namespace std::chrono;

template <typename T>
T MessageQueue<T>::recieve()
{

    // perform vector modification under the lock
    std::unique_lock<std::mutex> uLock(_mutex);
    _cond.wait(uLock, [this] { return !_messages.empty(); }); // pass unique lock to condition variable

    // remove last vector element from queue
    T message = std::move(_messages.back());
    _messages.pop_back();

    return message; // will not be copied due to return value optimization (RVO) in C++
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex>
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    // simulate some work
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // perform vector modification under the lock
    std::lock_guard<std::mutex> uLock(_mutex);

    // add vector to queue
    std::cout << "   Message #" << msg << " will be added to the queue" << std::endl;
    _messages.push_back(std::move(msg));
    _cond.notify_one(); // notify client after pushing new Vehicle into vector
}

/* Implementation of class "TrafficLight" */

TrafficLight::TrafficLight()
{
    _type = ObjectType::objectTrafficLight;
    _currentPhase = TrafficLightPhase::red;
}

TrafficLight::~TrafficLight() {}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop
    // runs and repeatedly calls the receive function on the message queue.
    // Once it receives TrafficLightPhase::green, the method returns.
    while (true)
    {
        auto light = this->_currentPhase = this->queue.recieve();
        if (light == TrafficLightPhase::green)
        {
            break;
        }
        /* code */
    }
}

void TrafficLight::setCurrentPhase(TrafficLightPhase phase)
{
    std::lock_guard<std::mutex> locker(_mutex);
    _currentPhase = phase;
}


TrafficLightPhase TrafficLight::getCurrentPhase()
{
    std::lock_guard<std::mutex> locker(_mutex);
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class.
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles
    // and toggles the current phase of the traffic light between red and green and sends an update method
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds.
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles.

    auto random = rand() % 6 + 4;
    auto start = std::chrono::steady_clock::now();
    auto end = std::chrono::steady_clock::now();

    int state = 1;

    while (true)
    {

        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        if (std::chrono::duration_cast<std::chrono::seconds>(end - start).count() < random)
        {
            //std::this_thread::sleep_for(std::chrono::seconds(1));
            std::cout << "elapsed " << (end -start).count() << " seconds" << std::endl;
            end = std::chrono::steady_clock::now();
            continue;
        }
        else
        {
            TrafficLightPhase phase = (TrafficLightPhase)(( state) % 2);
            setCurrentPhase(phase);
            queue.send(std::move(getCurrentPhase()));
            state++;
            random = rand() % 6 + 4;
            std::cout << "Time for next trafic light change is :" << random << " seconds" << std::endl;
            start = std::chrono::steady_clock::now();
            end = std::chrono::steady_clock::now();
        }
    }
}
