// Copyright (C) 2011 - 2012 Andrzej Krzemienski.
// Copytight (C) 2014 Jaroslaw Szpilewski (addition of Error + move into jsz namespace)
//
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// The idea and interface is based on Boost.Optional library
// authored by Fernando Luis Cacciola Carballal

# ifndef ___OPTIONAL_HPP___
# define ___OPTIONAL_HPP___

# include <utility>
# include <type_traits>
# include <initializer_list>
# include <cassert>
# include <functional>
# include <string>
# include <stdexcept>
#include <execinfo.h>
#include <vector>

# define REQUIRES(...) typename std::enable_if<__VA_ARGS__::value, bool>::type = false

# if defined __clang__
#  if (__clang_major__ > 2) || (__clang_major__ == 2) && (__clang_minor__ >= 9)
#   define OPTIONAL_HAS_THIS_RVALUE_REFS 1
#  else
#   define OPTIONAL_HAS_THIS_RVALUE_REFS 0
#  endif
# else
#  define OPTIONAL_HAS_THIS_RVALUE_REFS 0
# endif


namespace jsz {
    
    //use Ref<T> and MutRef<T> to box const references/references into optional!
    template <class T>
    struct Ref {
    public:
        Ref(T &r) : ref(r) {
            
        }
        
        Ref(const T &r) : ref(r) {
            
        }
        
        operator T() const {
            return ref;
        }
        
        const T &unbox() const {
            return ref;
        }
    private:
        const T &ref;
    };
    
    
    template <class T>
    struct MutRef {
    public:
        MutRef(T &r) : ref(r) {
            
        }
        
        operator T() {
            return ref;
        }
        
        T &unbox() {
            return ref;
        }
    private:
        T &ref;
    };
    
    
    struct Error {
        Error() noexcept : code(0), context(""), description("") {};
        
        Error(const int &code_, const std::string &context_, const std::string &description_) noexcept : code(code_), context(context_), description(description_) {
            printf("[!] jsz::Error: %i - %s - %s\n", code, context.c_str(), description.c_str());
            
            void* _callstack[128];
            int i, frames = backtrace(_callstack, 128);
            char** strs = backtrace_symbols(_callstack, frames);
            for (i = 0; i < frames; ++i) {
                callstack.push_back(std::string(strs[i]));
            }
            free(strs);
        };
        
        int code;
        std::string context;
        std::string description;
        std::vector<std::string> callstack;
    };
    
    
# if (defined __GNUC__) && (__GNUC__ >= 4) && (__GNUC_MINOR__ >= 7)
    // leave it; our metafunctions are already defined.
# elif defined __clang__
    // leave it; our metafunctions are already defined.
# else
    
    
    // workaround for missing traits in GCC and CLANG
    template <class T>
    struct is_nothrow_move_constructible
    {
        constexpr static bool value = std::is_nothrow_constructible<T, T&&>::value;
    };
    
    
    template <class T, class U>
    struct is_assignable
    {
        template <class X, class Y>
        static constexpr bool has_assign(...) { return false; }
        
        template <class X, class Y, size_t S = sizeof(std::declval<X>() = std::declval<Y>()) >
        static constexpr bool has_assign(bool) { return true; }
        
        constexpr static bool value = has_assign<T, U>(true);
    };
    
    
    template <class T>
    struct is_nothrow_move_assignable
    {
        template <class X, bool has_any_move_massign>
        struct has_nothrow_move_assign {
            constexpr static bool value = false;
        };
        
        template <class X>
        struct has_nothrow_move_assign<X, true> {
            constexpr static bool value = noexcept( std::declval<X&>() = std::declval<X&&>() );
        };
        
        constexpr static bool value = has_nothrow_move_assign<T, is_assignable<T&, T&&>::value>::value;
    };
    // end workaround
    
    
# endif
    
    
    //namespace experimental{
    
    
    // 20.5.4, optional for object types
    template <class T> class optional;
    
    // 20.5.5, optional for lvalue reference types
    template <class T> class optional<T&>;
    
    
    // workaround: std utility functions aren't constexpr yet
    template <class T> inline constexpr T&& constexpr_forward(typename std::remove_reference<T>::type& t) noexcept
    {
        return static_cast<T&&>(t);
    }
    
    template <class T> inline constexpr T&& constexpr_forward(typename std::remove_reference<T>::type&& t) noexcept
    {
        static_assert(!std::is_lvalue_reference<T>::value, "!!");
        return static_cast<T&&>(t);
    }
    
    template <class T> inline constexpr typename std::remove_reference<T>::type&& constexpr_move(T&& t) noexcept
    {
        return static_cast<typename std::remove_reference<T>::type&&>(t);
    }
    
    template<class _Ty> inline constexpr _Ty * constexpr_addressof(_Ty& _Val)
    {
        return ((_Ty *) &(char&)_Val);
    }
    
    
#if defined NDEBUG
# define ASSERTED_EXPRESSION(CHECK, EXPR) (EXPR)
#else
# define ASSERTED_EXPRESSION(CHECK, EXPR) ((CHECK) ? (EXPR) : (fail(#CHECK, __FILE__, __LINE__), (EXPR)))
    inline void fail(const char* expr, const char* file, unsigned line)
    {
# if defined __clang__ || defined __GNU_LIBRARY__
        __assert(expr, file, line);
# elif defined __GNUC__
        _assert(expr, file, line);
# else
#   error UNSUPPORTED COMPILER
# endif
    }
#endif
    
    
    template <typename T>
    struct has_overloaded_addressof
    {
        template <class X>
        static constexpr bool has_overload(...) { return false; }
        
        template <class X, size_t S = sizeof(std::declval< X&>().operator&()) >
        static constexpr bool has_overload(bool) { return true; }
        
        constexpr static bool value = has_overload<T>(true);
    };
    
    
    
    template <typename T, REQUIRES(!has_overloaded_addressof<T>)>
    constexpr T* static_addressof(T& ref)
    {
        return &ref;
    }
    
    template <typename T, REQUIRES(has_overloaded_addressof<T>)>
    T* static_addressof(T& ref)
    {
        return std::addressof(ref);
    }
    
    
    
    template <class U>
    struct is_not_optional
    {
        constexpr static bool value = true;
    };
    
    template <class T>
    struct is_not_optional<optional<T>>
    {
        constexpr static bool value = false;
    };
    
    
    constexpr struct trivial_init_t{} trivial_init{};
    
    
    // 20.5.6, In-place construction
    constexpr struct emplace_t{} emplace{};
    
    
    // 20.5.7, Disengaged state indicator
    struct nullopt_t
    {
        struct init{};
        constexpr nullopt_t(init){};
    };
    constexpr nullopt_t nullopt{nullopt_t::init{}};
    
    
    // 20.5.8, class bad_optional_access
    class bad_optional_access : public std::logic_error {
    public:
        explicit bad_optional_access(const std::string& what_arg) : std::logic_error{what_arg} {}
        explicit bad_optional_access(const char* what_arg) : std::logic_error{what_arg} {}
    };
    
    
    template <class T>
    union storage_t
    {
        unsigned char dummy_;
        T value_;
        
        constexpr storage_t( trivial_init_t ) noexcept : dummy_() {};
        
        template <class... Args>
        constexpr storage_t( Args&&... args ) : value_(constexpr_forward<Args>(args)...) {}
        
        ~storage_t(){}
    };
    
    
    template <class T>
    union constexpr_storage_t
    {
        unsigned char dummy_;
        T value_;
        
        constexpr constexpr_storage_t( trivial_init_t ) noexcept : dummy_() {};
        
        template <class... Args>
        constexpr constexpr_storage_t( Args&&... args ) : value_(constexpr_forward<Args>(args)...) {}
        
        ~constexpr_storage_t() = default;
    };
    
    
    constexpr struct only_set_initialized_t{} only_set_initialized{};
    
    
    template <class T>
    struct optional_base
    {
        bool init_;
        storage_t<T> storage_;
        
        constexpr optional_base() noexcept : init_(false), storage_(trivial_init) {};
        
        constexpr explicit optional_base(only_set_initialized_t, bool init) noexcept : init_(init), storage_(trivial_init) {};
        
        explicit constexpr optional_base(const T& v) : init_(true), storage_(v) {}
        
        explicit constexpr optional_base(T&& v) : init_(true), storage_(constexpr_move(v)) {}
        
        template <class... Args> explicit optional_base(emplace_t, Args&&... args)
        : init_(true), storage_(constexpr_forward<Args>(args)...) {}
        
        template <class U, class... Args, REQUIRES(std::is_constructible<T, std::initializer_list<U>>)>
        explicit optional_base(emplace_t, std::initializer_list<U> il, Args&&... args)
        : init_(true), storage_(il, std::forward<Args>(args)...) {}
        
        ~optional_base() { if (init_) storage_.value_.T::~T(); }
    };
    
    
    template <class T>
    struct constexpr_optional_base
    {
        bool init_;
        constexpr_storage_t<T> storage_;
        
        constexpr constexpr_optional_base() noexcept : init_(false), storage_(trivial_init) {};
        
        constexpr explicit constexpr_optional_base(only_set_initialized_t, bool init) noexcept : init_(init), storage_(trivial_init) {};
        
        explicit constexpr constexpr_optional_base(const T& v) : init_(true), storage_(v) {}
        
        explicit constexpr constexpr_optional_base(T&& v) : init_(true), storage_(constexpr_move(v)) {}
        
        template <class... Args> explicit constexpr constexpr_optional_base(emplace_t, Args&&... args)
        : init_(true), storage_(constexpr_forward<Args>(args)...) {}
        
        template <class U, class... Args, REQUIRES(std::is_constructible<T, std::initializer_list<U>>)>
        explicit constexpr_optional_base(emplace_t, std::initializer_list<U> il, Args&&... args)
        : init_(true), storage_(il, std::forward<Args>(args)...) {}
        
        ~constexpr_optional_base() = default;
    };
    
    template <class T>
    using OptionalBase = typename std::conditional<
    std::is_trivially_destructible<T>::value,
    constexpr_optional_base<T>,
    optional_base<T>
    >::type;
    
    
    
    template <class T>
    class optional : private OptionalBase<T>
    {
        static_assert( !std::is_same<typename std::decay<T>::type, nullopt_t>::value, "bad T" );
        static_assert( !std::is_same<typename std::decay<T>::type, emplace_t>::value, "bad T" );
        
        
        constexpr bool initialized() const noexcept { return OptionalBase<T>::init_; }
        T* dataptr() {  return std::addressof(OptionalBase<T>::storage_.value_); }
        constexpr const T* dataptr() const { return static_addressof(OptionalBase<T>::storage_.value_); }
        
# if OPTIONAL_HAS_THIS_RVALUE_REFS == 1
        constexpr const T& contained_val() const& { return OptionalBase<T>::storage_.value_; }
        T& contained_val() & { return OptionalBase<T>::storage_.value_; }
        T&& contained_val() && { return std::move(OptionalBase<T>::storage_.value_); }
# else
        constexpr const T& contained_val() const { return OptionalBase<T>::storage_.value_; }
        T& contained_val() { return OptionalBase<T>::storage_.value_; }
# endif
        
        void clear() noexcept {
            if (initialized()) dataptr()->T::~T();
            OptionalBase<T>::init_ = false;
        }
        
        template <class... Args>
        void initialize(Args&&... args) noexcept(noexcept(T(std::forward<Args>(args)...)))
        {
            assert(!OptionalBase<T>::init_);
            new (dataptr()) T(std::forward<Args>(args)...);
            OptionalBase<T>::init_ = true;
        }
        
        template <class U, class... Args>
        void initialize(std::initializer_list<U> il, Args&&... args) noexcept(noexcept(T(il, std::forward<Args>(args)...)))
        {
            assert(!OptionalBase<T>::init_);
            new (dataptr()) T(il, std::forward<Args>(args)...);
            OptionalBase<T>::init_ = true;
        }
        
    public:
        typedef T value_type;
        
        // 20.5.5.1, constructors
        constexpr optional() noexcept : OptionalBase<T>(), has_error_(false)  {};
        constexpr optional(nullopt_t) noexcept : OptionalBase<T>(), has_error_(false) {};
        
        optional(const Error &err) noexcept : OptionalBase<T>(), error_(err), has_error_(true) {
        };
        
        optional(const Error &&err) noexcept : OptionalBase<T>(), error_(err), has_error_(true) {
        };
        
        
        optional(const optional& rhs)
        : OptionalBase<T>(only_set_initialized, rhs.initialized())
        {
            if (rhs.initialized()) new (dataptr()) T(*rhs);
            has_error_ = rhs.has_error_;
            error_ = rhs.error_;
        }
        
        optional(optional&& rhs) noexcept(std::is_nothrow_move_constructible<T>::value)
        : OptionalBase<T>(only_set_initialized, rhs.initialized())
        {
            if (rhs.initialized()) new (dataptr()) T(std::move(*rhs));
            has_error_ = rhs.has_error_;
            error_ = rhs.error_;
        }
        
        
        constexpr optional(const T& v) : OptionalBase<T>(v) {}
        
        constexpr optional(T&& v) : OptionalBase<T>(constexpr_move(v)) {}
        
        
        template <class... Args>
        constexpr explicit optional(emplace_t, Args&&... args)
        : OptionalBase<T>(emplace_t{}, constexpr_forward<Args>(args)...) {}
        
        template <class U, class... Args, REQUIRES(std::is_constructible<T, std::initializer_list<U>>)>
        explicit optional(emplace_t, std::initializer_list<U> il, Args&&... args)
        : OptionalBase<T>(emplace_t{}, il, constexpr_forward<Args>(args)...) {}
        
        // 20.5.4.2 Destructor
        ~optional() = default;
        
        // 20.5.4.3, assignment
        optional& operator=(nullopt_t) noexcept
        {
            clear();
            return *this;
        }
        
        optional& operator=(const optional& rhs)
        {
            has_error_ = rhs.has_error_;
            error_ = rhs.error_;
            
            if      (initialized() == true  && rhs.initialized() == false) clear();
            else if (initialized() == false && rhs.initialized() == true)  initialize(*rhs);
            else if (initialized() == true  && rhs.initialized() == true)  contained_val() = *rhs;
            return *this;
        }
        
        optional& operator=(optional&& rhs)
        noexcept(std::is_nothrow_move_assignable<T>::value && std::is_nothrow_move_constructible<T>::value)
        {
            has_error_ = rhs.has_error_;
            error_ = rhs.error_;
            if      (initialized() == true  && rhs.initialized() == false) clear();
                else if (initialized() == false && rhs.initialized() == true)  initialize(std::move(*rhs));
                    else if (initialized() == true  && rhs.initialized() == true)  contained_val() = std::move(*rhs);
                        return *this;
        }
        
        void operator=(const Error &err) {
            has_error_ = true;
            error_ = err;
            //            return *this;
        }
        
        template <class U>
        auto operator=(U&& v)
        -> typename std::enable_if
        <
        std::is_same<typename std::remove_reference<U>::type, T>::value,
        optional&
        >::type
        {
            if (initialized()) { contained_val() = std::forward<U>(v); }
            else               { initialize(std::forward<U>(v));  }
            
//TODO: WHY?!
//            has_error_ = v.has_err_;
//            error_ = v.error_;
            return *this;
        }
        
        
        template <class... Args>
        optional<T>& emplace(Args&&... args)
        {
            clear();
            initialize(std::forward<Args>(args)...);
            return *this;
        }
        
        template <class U, class... Args>
        optional<T>& emplace(std::initializer_list<U> il, Args&&... args)
        {
            clear();
            initialize<U, Args...>(il, std::forward<Args>(args)...);
            return *this;
        }
        
        // 20.5.4.4 Swap
        void swap(optional<T>& rhs) noexcept(std::is_nothrow_move_constructible<T>::value && noexcept(swap(std::declval<T&>(), std::declval<T&>())))
        {
            if      (initialized() == true  && rhs.initialized() == false) { rhs.initialize(std::move(**this)); clear(); }
            else if (initialized() == false && rhs.initialized() == true)  { initialize(std::move(*rhs)); rhs.clear(); }
            else if (initialized() == true  && rhs.initialized() == true)  { using std::swap; swap(**this, *rhs); }
            
            has_error_ = rhs.has_err_;
            error_ = rhs.error_;
            
        }
        
        // 20.5.4.5 Observers
        constexpr T const* operator ->() const {
            return ASSERTED_EXPRESSION(initialized(), dataptr());
        }
        
        T* operator ->() {
            assert (initialized());
            return dataptr();
        }
        
        constexpr T const& operator *() const {
            return ASSERTED_EXPRESSION(initialized(), contained_val());
        }
        
        T& operator *() {
            assert (initialized());
            return contained_val();
        }
        
        /*
         If you end up here because of a c++ exception then this means that the optional doesn not contain a value!
         Check the error()!
         */
        
        constexpr T const& value() const {
            return initialized() ? contained_val() : (throw bad_optional_access("bad optional access"), contained_val());
        }
        
        T& value() {
            return initialized() ? contained_val() : (throw bad_optional_access("bad optional access"), contained_val());
        }
        
        constexpr explicit operator bool() const noexcept { return initialized(); }
        
# if OPTIONAL_HAS_THIS_RVALUE_REFS == 1
        
        template <class V>
        constexpr T value_or(V&& v) const&
        {
            return *this ? **this : static_cast<T>(constexpr_forward<V>(v));
        }
        
        template <class V>
        constexpr T value_or(V&& v) const &&
        {
            return *this ? std::move(const_cast<optional<T>&>(*this).contained_val()) : static_cast<T>(constexpr_forward<V>(v));
            }
            
# else
            
            template <class V>
            constexpr T value_or(V&& v) const
            {
                return *this ? **this : static_cast<T>(constexpr_forward<V>(v));
            }
            
# endif
            
        public:
            
            Error error() const {
                return error_;
            }
            
            Error error_or(const int &errcode, const std::string &context, const std::string &errmsg) {
                if (has_error()) {
                    return error();
                }
                
                return Error(errcode, context, errmsg);
            }
            
            void set_error(Error err) {
                has_error_ = true;
                error_ = err;
            }
            
            bool has_error() const {
                return has_error_;
            }
            
        protected:
            Error error_;
            bool has_error_ = false;
            
        };
        
        
        template <class T>
        class optional<T&>
        {
            static_assert( !std::is_same<T, nullopt_t>::value, "bad T" );
            static_assert( !std::is_same<T, emplace_t>::value, "bad T" );
            T* ref;
            
        public:
            
            // 20.5.5.1, construction/destruction
            constexpr optional() noexcept : ref(nullptr), has_error_(false) {}
            
            constexpr optional(nullopt_t) noexcept : ref(nullptr), has_error_(false) {}
            
            constexpr optional(T& v) noexcept : ref(static_addressof(v)), has_error_(v.has_error_), error_(v.error_) {}
            
            optional(T&&) = delete;
            
            constexpr optional(const optional& rhs) noexcept : ref(rhs.ref), has_error_(rhs.has_error_), error_(rhs.error_) {}
            
            explicit constexpr optional(emplace_t, T& v) noexcept : ref(static_addressof(v)), has_error_(v.has_error_), error_(v.error_) {}
            
            explicit optional(emplace_t, T&&) = delete;
            
            ~optional() = default;
            
            // 20.5.5.2, mutation
            optional& operator=(nullopt_t) noexcept {
                ref = nullptr;
                return *this;
            }
            
            // optional& operator=(const optional& rhs) noexcept {
            // ref = rhs.ref;
            // return *this;
            // }
            
            // optional& operator=(optional&& rhs) noexcept {
            // ref = rhs.ref;
            // return *this;
            // }
            
            template <typename U>
            auto operator=(U&& rhs) noexcept
            -> typename std::enable_if
            <
            std::is_same<typename std::decay<U>::type, optional<T&>>::value,
            optional&
            >::type
            {
                ref = rhs.ref;
                return *this;
            }
            
            template <typename U>
            auto operator=(U&& rhs) noexcept
            -> typename std::enable_if
            <
            !std::is_same<typename std::decay<U>::type, optional<T&>>::value,
            optional&
            >::type
            = delete;
            
            optional& emplace(T& v) noexcept {
                ref = static_addressof(v);
                return *this;
            }
            
            optional& emplace(T&&) = delete;
            
            
            void swap(optional<T&>& rhs) noexcept
            {
                std::swap(ref, rhs.ref);
            }
            
            // 20.5.5.3, observers
            constexpr T* operator->() const {
                return ASSERTED_EXPRESSION(ref, ref);
            }
            
            constexpr T& operator*() const {
                return ASSERTED_EXPRESSION(ref, *ref);
            }
            
            constexpr T& value() const {
                return ref ? *ref : (throw bad_optional_access("bad optional access"), *ref);
            }
            
            explicit constexpr operator bool() const noexcept {
                return ref != nullptr;
            }
            
            template <class V>
            constexpr typename std::decay<T>::type value_or(V&& v) const
            {
                return *this ? **this : static_cast<typename std::decay<T>::type>(constexpr_forward<V>(v));
            }
            
        public:
            optional(const Error &err) noexcept : error_(err), has_error_(true) {};
            //    optional(const Error err) noexcept : error_(err), has_error_(true) {};
            //    optional(const Error &&err) noexcept : error_(err), has_error_(true) {};
            
            Error error() const {
                return error_;
            }
            
            void set_error(Error err) {
                has_error_ = true;
                error_ = err;
            }
            
            bool has_error() const {
                return has_error_;
            }
            
        protected:
            Error error_;
            bool has_error_;
            
        };
        
        
        template <class T>
        class optional<T&&>
        {
            static_assert( sizeof(T) == 0, "optional rvalue referencs disallowed" );
        };
        
        
        // 20.5.8, Relational operators
        template <class T> constexpr bool operator==(const optional<T>& x, const optional<T>& y)
        {
            return bool(x) != bool(y) ? false : bool(x) == false ? true : *x == *y;
        }
        
        template <class T> constexpr bool operator!=(const optional<T>& x, const optional<T>& y)
        {
            return !(x == y);
        }
        
        template <class T> constexpr bool operator<(const optional<T>& x, const optional<T>& y)
        {
            return (!y) ? false : (!x) ? true : *x < *y;
        }
        
        template <class T> constexpr bool operator>(const optional<T>& x, const optional<T>& y)
        {
            return (y < x);
        }
        
        template <class T> constexpr bool operator<=(const optional<T>& x, const optional<T>& y)
        {
            return !(y < x);
        }
        
        template <class T> constexpr bool operator>=(const optional<T>& x, const optional<T>& y)
        {
            return !(x < y);
        }
        
        
        // 20.5.9 Comparison with nullopt
        template <class T> constexpr bool operator==(const optional<T>& x, nullopt_t) noexcept
        {
            return (!x);
        }
        
        template <class T> constexpr bool operator==(nullopt_t, const optional<T>& x) noexcept
        {
            return (!x);
        }
        
        template <class T> constexpr bool operator!=(const optional<T>& x, nullopt_t) noexcept
        {
            return bool(x);
        }
        
        template <class T> constexpr bool operator!=(nullopt_t, const optional<T>& x) noexcept
        {
            return bool(x);
        }
        
        template <class T> constexpr bool operator<(const optional<T>&, nullopt_t) noexcept
        {
            return false;
        }
        
        template <class T> constexpr bool operator<(nullopt_t, const optional<T>& x) noexcept
        {
            return bool(x);
        }
        
        template <class T> constexpr bool operator<=(const optional<T>& x, nullopt_t) noexcept
        {
            return (!x);
        }
        
        template <class T> constexpr bool operator<=(nullopt_t, const optional<T>&) noexcept
        {
            return true;
        }
        
        template <class T> constexpr bool operator>(const optional<T>& x, nullopt_t) noexcept
        {
            return bool(x);
        }
        
        template <class T> constexpr bool operator>(nullopt_t, const optional<T>&) noexcept
        {
            return false;
        }
        
        template <class T> constexpr bool operator>=(const optional<T>&, nullopt_t) noexcept
        {
            return true;
        }
        
        template <class T> constexpr bool operator>=(nullopt_t, const optional<T>& x) noexcept
        {
            return (!x);
        }
        
        
        
        // 20.5.10, Comparison with T
        template <class T> constexpr bool operator==(const optional<T>& x, const T& v)
        {
            return bool(x) ? *x == v : false;
        }
        
        template <class T> constexpr bool operator==(const T& v, const optional<T>& x)
        {
            return bool(x) ? v == *x : false;
        }
        
        template <class T> constexpr bool operator!=(const optional<T>& x, const T& v)
        {
            return bool(x) ? *x != v : true;
        }
        
        template <class T> constexpr bool operator!=(const T& v, const optional<T>& x)
        {
            return bool(x) ? v != *x : true;
        }
        
        template <class T> constexpr bool operator<(const optional<T>& x, const T& v)
        {
            return bool(x) ? *x < v : true;
        }
        
        template <class T> constexpr bool operator>(const T& v, const optional<T>& x)
        {
            return bool(x) ? v > *x : true;
        }
        
        template <class T> constexpr bool operator>(const optional<T>& x, const T& v)
        {
            return bool(x) ? *x > v : false;
        }
        
        template <class T> constexpr bool operator<(const T& v, const optional<T>& x)
        {
            return bool(x) ? v < *x : false;
        }
        
        template <class T> constexpr bool operator>=(const optional<T>& x, const T& v)
        {
            return bool(x) ? *x >= v : false;
        }
        
        template <class T> constexpr bool operator<=(const T& v, const optional<T>& x)
        {
            return bool(x) ? v <= *x : false;
        }
        
        template <class T> constexpr bool operator<=(const optional<T>& x, const T& v)
        {
            return bool(x) ? *x <= v : true;
        }
        
        template <class T> constexpr bool operator>=(const T& v, const optional<T>& x)
        {
            return bool(x) ? v >= *x : true;
        }
        
        
        // Comparison of optionsl<T&> with T
        template <class T> constexpr bool operator==(const optional<T&>& x, const T& v)
        {
            return bool(x) ? *x == v : false;
        }
        
        template <class T> constexpr bool operator==(const T& v, const optional<T&>& x)
        {
            return bool(x) ? v == *x : false;
        }
        
        template <class T> constexpr bool operator!=(const optional<T&>& x, const T& v)
        {
            return bool(x) ? *x != v : true;
        }
        
        template <class T> constexpr bool operator!=(const T& v, const optional<T&>& x)
        {
            return bool(x) ? v != *x : true;
        }
        
        template <class T> constexpr bool operator<(const optional<T&>& x, const T& v)
        {
            return bool(x) ? *x < v : true;
        }
        
        template <class T> constexpr bool operator>(const T& v, const optional<T&>& x)
        {
            return bool(x) ? v > *x : true;
        }
        
        template <class T> constexpr bool operator>(const optional<T&>& x, const T& v)
        {
            return bool(x) ? *x > v : false;
        }
        
        template <class T> constexpr bool operator<(const T& v, const optional<T&>& x)
        {
            return bool(x) ? v < *x : false;
        }
        
        template <class T> constexpr bool operator>=(const optional<T&>& x, const T& v)
        {
            return bool(x) ? *x >= v : false;
        }
        
        template <class T> constexpr bool operator<=(const T& v, const optional<T&>& x)
        {
            return bool(x) ? v <= *x : false;
        }
        
        template <class T> constexpr bool operator<=(const optional<T&>& x, const T& v)
        {
            return bool(x) ? *x <= v : true;
        }
        
        template <class T> constexpr bool operator>=(const T& v, const optional<T&>& x)
        {
            return bool(x) ? v >= *x : true;
        }
        
        // Comparison of optionsl<T const&> with T
        template <class T> constexpr bool operator==(const optional<const T&>& x, const T& v)
        {
            return bool(x) ? *x == v : false;
        }
        
        template <class T> constexpr bool operator==(const T& v, const optional<const T&>& x)
        {
            return bool(x) ? v == *x : false;
        }
        
        template <class T> constexpr bool operator!=(const optional<const T&>& x, const T& v)
        {
            return bool(x) ? *x != v : true;
        }
        
        template <class T> constexpr bool operator!=(const T& v, const optional<const T&>& x)
        {
            return bool(x) ? v != *x : true;
        }
        
        template <class T> constexpr bool operator<(const optional<const T&>& x, const T& v)
        {
            return bool(x) ? *x < v : true;
        }
        
        template <class T> constexpr bool operator>(const T& v, const optional<const T&>& x)
        {
            return bool(x) ? v > *x : true;
        }
        
        template <class T> constexpr bool operator>(const optional<const T&>& x, const T& v)
        {
            return bool(x) ? *x > v : false;
        }
        
        template <class T> constexpr bool operator<(const T& v, const optional<const T&>& x)
        {
            return bool(x) ? v < *x : false;
        }
        
        template <class T> constexpr bool operator>=(const optional<const T&>& x, const T& v)
        {
            return bool(x) ? *x >= v : false;
        }
        
        template <class T> constexpr bool operator<=(const T& v, const optional<const T&>& x)
        {
            return bool(x) ? v <= *x : false;
        }
        
        template <class T> constexpr bool operator<=(const optional<const T&>& x, const T& v)
        {
            return bool(x) ? *x <= v : true;
        }
        
        template <class T> constexpr bool operator>=(const T& v, const optional<const T&>& x)
        {
            return bool(x) ? v >= *x : true;
        }
        
        
        // 20.5.12 Specialized algorithms
        template <class T>
        void swap(optional<T>& x, optional<T>& y) noexcept(noexcept(x.swap(y)))
        {
            x.swap(y);
        }
        
        
        template <class T>
        constexpr optional<typename std::decay<T>::type> make_optional(T&& v)
        {
            return optional<typename std::decay<T>::type>(constexpr_forward<T>(v));
        }
        
        template <class X>
        constexpr optional<X&> make_optional(std::reference_wrapper<X> v)
        {
            return optional<X&>(v.get());
        }
        
        
        //} // namespace experimental
    } // namespace std
    
    namespace std
    {
        template <typename T>
        struct hash<jsz::optional<T>>
        {
            typedef typename hash<T>::result_type result_type;
            typedef jsz::optional<T> argument_type;
            
            constexpr result_type operator()(argument_type const& arg) const {
                return arg ? std::hash<T>{}(*arg) : result_type{};
            }
        };
        
        template <typename T>
        struct hash<jsz::optional<T&>>
        {
            typedef typename hash<T>::result_type result_type;
            typedef jsz::optional<T&> argument_type;
            
            constexpr result_type operator()(argument_type const& arg) const {
                return arg ? std::hash<T>{}(*arg) : result_type{};
            }
        };
    }
    
    
    
# endif //___OPTIONAL_HPP___
