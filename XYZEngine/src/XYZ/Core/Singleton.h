#pragma once
#include "Core.h"


namespace XYZ {
    template <typename T>
    class XYZ_API Singleton
    {  
    public:
        Singleton(const Singleton&) = delete;
        Singleton& operator=(const Singleton&) = delete;
        Singleton(Singleton&&) = delete;
        Singleton& operator=(Singleton&&) = delete;
    
        static T& Get()
        {
            static T instance(token{});
            return instance;
        }

    protected:
        struct token {};
        Singleton() {}; // Disallow instantiation outside of the class.
    };
}