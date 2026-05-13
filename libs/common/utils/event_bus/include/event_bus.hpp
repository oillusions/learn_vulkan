#pragma once
#include <iostream>
#include <typeindex>
#include <functional>
#include <memory>
#include <map>

class EventBus {
    public:
        EventBus() = default;
        ~EventBus() = default;

        EventBus(const EventBus&) = delete;
        EventBus& operator = (const EventBus&) = delete;

        EventBus(EventBus&&) noexcept;
        EventBus& operator = (EventBus&&) noexcept;

        /**
         * @brief 发布事件
         * @details 他似乎不需要详细注释[划掉]
         * @tparam EventType 事件类型
         * @param identifier 事件标识符
         * @param content 事件内容
         */
        template<typename EventType>
        void publish(const std::string& identifier, EventType& content) const noexcept {
            if (identifier.empty()) return;
            if (_subscriptionMap.empty()) return;

            if (const auto typeMap_it = _subscriptionMap.find(typeid(EventType)); typeMap_it != _subscriptionMap.end()) {
                if (typeMap_it->second.empty()) return;
                if (const auto identifierMap_it = typeMap_it->second.find(identifier); identifierMap_it != typeMap_it->second.end()) {
                    if (identifierMap_it->second.empty()) return;
                    for (auto& [call, id] : identifierMap_it->second) {
                        if (auto* wrapper = static_cast<CallWrapper<EventType&>*>(call.get())) {
                            wrapper->call(content);
                        }
                    }
                }
            }
        }

        /**
         * @brief 订阅事件
         * @details 他似乎不需要详细注释[划掉]
         * @tparam EventType 事件类型
         * @param identifier 事件标识符
         * @param call 事件回调
         * @return 回调ID
         */
        template<typename EventType>
        size_t subscribe(const std::string& identifier, std::function<void(EventType&)> call) noexcept {
            _subscriptionMap[typeid(EventType)][identifier].emplace_back(CallInfo{std::make_unique<CallWrapper<EventType&>>(call), _idCounter});
            _idCounter++;
            return _idCounter - 1;
        }

        /**
         * @brief 退订事件
         * @details 他似乎不需要详细注释[划掉]
         * @tparam EventType 事件类型
         * @param identifier 事件标识符
         * @param id 事件id
         */
        template<typename EventType>
        void unsubscribe(const std::string& identifier, size_t id) noexcept {
            auto type_it = _subscriptionMap.find(typeid(EventType));
            if (type_it == _subscriptionMap.end()) return;

            auto identifier_it = type_it->second.find(identifier);
            if (identifier_it == type_it->second.end()) return;

            for (auto i = identifier_it->second.begin(); i != identifier_it->second.end(); i++) {
                if (i->id == id) {
                    identifier_it->second.erase(i);
                    return;
                }
            }
        }

    private:
        /**
         * @brief 回调类型擦除基类
         * @details 他似乎不需要详细注释[划掉]
         */
        class CallBase {
            public:
                virtual ~CallBase() = default;
                virtual void call() {
                    std::cout << "空" << std::endl;
                }
            };

        /**
         * @brief 类型擦除回调包装器
         * @tparam CallType 回调类型
         */
        template<typename CallType>
        class CallWrapper: public CallBase{
            public:
                CallWrapper(std::function<void(CallType&)> call): _call(call) {}
                ~CallWrapper() override = default;

                void call(CallType& content) {
                    _call(content);
                }

                void operator () (CallType& content) {
                    call(content);
                }
            private:
                std::function<void(CallType&)> _call;
        };

        /**
         * @brief 回调信息
         */
        struct CallInfo {
            std::unique_ptr<CallBase> call;
            size_t id;
        };


        std::map<std::type_index, std::map<std::string, std::vector<CallInfo>>> _subscriptionMap;
        size_t _idCounter{};

};

// #pragma once
// #include <functional>
// #include <map>
// #include <memory>
// #include <set>
// #include <string>
// #include <typeindex>
// #include <utility>
// #include <vector>
//
// class EventBus {
//     public:
//         template<typename FuncType>
//         class Subscription {
//             public:
//                 friend EventBus;
//                 using Type = FuncType;
//                 EventBus& event_bus;
//                 const std::string identifier;
//                 const size_t id;
//
//             protected:
//                 bool unsubscribed{false};
//
//                 explicit Subscription(EventBus& event_bus, const std::string &identifier, const size_t id) noexcept :
//                     event_bus(event_bus),
//                     identifier(identifier),
//                     id(id)
//                 {}
//
//             public:
//                 Subscription(Subscription&& other) noexcept :
//                     event_bus(other.event_bus),
//                     identifier(other.identifier),
//                     id(other.id),
//                     unsubscribed(other.unsubscribed) {
//                     other.unsubscribed = true;
//                 }
//                 Subscription& operator = (Subscription&&) noexcept = delete;
//
//                 Subscription(const Subscription&) = delete;
//                 Subscription& operator = (const Subscription&) = delete;
//
//                 void unsubscribe() noexcept {
//                     if (!unsubscribed) {
//                         event_bus.unsubscribe(*this);
//                         unsubscribed = true;
//                     }
//                 }
//
//                 ~Subscription() {
//                     unsubscribe();
//                 }
//         };
//
//         EventBus() noexcept = default;
//
//
//         EventBus(const EventBus&) = delete;
//         EventBus& operator = (const EventBus&) = delete;
//
//
//
//         ~EventBus() = default;
//
//     protected:
//         class BaseWrapper {
//         public:
//             const size_t id;
//             explicit BaseWrapper(const size_t id) noexcept : id(id) {};
//             virtual ~BaseWrapper() = default;
//             virtual void* func_ptr() {return nullptr;};
//         };
//
//         template<typename Func>
//         class FuncWrapper : public BaseWrapper {
//         private:
//             const Func& func;
//
//         public:
//             explicit FuncWrapper(const Func&& func, const size_t id) noexcept :
//                 BaseWrapper(id),
//                 func(std::forward<Func>(func))
//             {};
//
//             void* func_ptr() override {
//                 return &func;
//             }
//         };
//
//         std::map<std::type_index, std::map<std::string, std::set<std::unique_ptr<BaseWrapper>>>> subscription_func_map;
//         size_t counter{0};
//
//     public:
//
//         template<typename Func>
//         requires std::is_function_v<Func>
//         Subscription<Func> subscribe(const std::string& identifier, Func&& func) {
//             auto typeid_it = subscription_func_map.find(typeid(Func));
//             if (typeid_it == subscription_func_map.end()) {
//                 typeid_it = subscription_func_map.emplace(typeid(Func), std::map<std::string, std::set<std::unique_ptr<BaseWrapper>>>());
//             }
//
//             auto list_it = typeid_it->second.find(identifier);
//             if (list_it == typeid_it->second.end()) {
//                 list_it = typeid_it->second.emplace(identifier, std::set<std::unique_ptr<BaseWrapper>>()).first;
//             }
//
//             list_it->second.emplace(std::make_unique<FuncWrapper<Func>>(std::forward<Func>(func), counter++));
//
//             return Subscription<Func>(*this, identifier, counter);
//         }
//         template<typename Func, typename... Args>
//         requires std::is_function_v<Func>
//         void publish(const std::string& identifier, Args&&... args) noexcept {
//             auto typeid_it = subscription_func_map.find(typeid(Func));
//             if (typeid_it == subscription_func_map.end()) return;
//
//             auto func_list_it = typeid_it->second.find(identifier);
//             if (func_list_it == typeid_it->second.end()) return;
//
//             for (const auto func : func_list_it->second) {
//                 std::invoke(*static_cast<FuncWrapper<Func*>>(func->func_ptr()), std::forward<Args>(args)...);
//             }
//         }
//
//         template<typename Func>
//         requires std::is_function_v<Func>
//         void unsubscribe(Subscription<Func> & subscription) {
//             auto typeid_it = subscription_func_map.find(typeid(Func));
//             if (typeid_it == subscription_func_map.end()) return;
//
//             auto func_list_it = typeid_it->second.find(subscription.identifier);
//             if (func_list_it == typeid_it->second.end()) return;
//
//             for (auto unsubscribe_it = func_list_it->second.begin(); unsubscribe_it != func_list_it->second.end(); ++unsubscribe_it) {
//                 if ((*unsubscribe_it).id == subscription.id) {
//                     func_list_it->second.erase(unsubscribe_it);
//                     return;
//                 }
//             }
//         }
//
// };

