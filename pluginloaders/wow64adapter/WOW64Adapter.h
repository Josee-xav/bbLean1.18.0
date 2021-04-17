#ifndef __WOW64_ADAPTER_H__
#define __WOW64_ADAPTER_H__
#include "BBApiPluginLoader.h"
#include <functional>

struct HeavyFunction {
    std::function<void(void*, void**)> fn;
    size_t returnSize;

    HeavyFunction() {}

    HeavyFunction(std::function<void(void*, void**)> fn, size_t returnSize) : fn(fn), returnSize(returnSize) {
    } 
};

template<unsigned int N, typename Return>
struct Wrapper {
    // Unpacks parameter types and adds a casting argument selector to the next level's params
    template<typename Param, typename... Params, typename... FnParams, typename...Args>
    static std::function<void(void*, void**)> Wrap(Return(*fn)(FnParams...), Args... argSelectors) {
        return Wrapper<N-1, Return>::Wrap<Params..., FnParams..., Args...>(fn, argSelectors..., 
            [](void** args) { return *(Param*)args[sizeof...(Args)]; });
    }
};

template<>
struct Wrapper<0, void> {
    // Specialization for procedures, builds a lambda which calls the wrapped procedure 
    template<typename... Params, typename... FnParams, typename...Args>
    static std::function<void(void*, void**)> Wrap(void(*proc)(FnParams...), Args... argSelectors) {
        return [=](void* r, void**args) { proc((argSelectors(args))...); };
    }
};

template<typename Return>
struct Wrapper<0, Return> {
    template<typename... Params, typename... FnParams, typename...Args>
    static std::function<void(void*, void**)> Wrap(Return(*fn)(FnParams...), Args... argSelectors) {
        return [=](void* r, void**args) { *(Return*)r = fn((argSelectors(args))...); };
    }
};

template<typename R, typename... FnArgs>
static std::function<void(void*, void**)> Wrap(R(*fn)(FnArgs...)) {
    return Wrapper<sizeof...(FnArgs), R>::Wrap<FnArgs..., FnArgs...>(fn);
}

#endif