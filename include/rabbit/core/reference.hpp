#pragma once 

#include <atomic>
#include <cstddef>
#include <type_traits>

namespace rb {
    /** 
     * @brief Base class for reference counting.
     */
    class reference {
    public:
        /**
         * @brief Construct a new reference.
         */
        reference() = default;

        /**
         * @brief Disable copy constructor.
         */
        reference(const reference&) = delete;

        /**
         * @brief Default virtual destructor.
         */
        virtual ~reference() = default;

        /**
         * @brief Disable copy assignment.
         */
        reference& operator=(const reference&) = delete;

        /**
         * @brief Increment reference counter.
         */
        void retain();

        /**
         * @brief Decrement reference counter.
         */
        void release();

    private:
        /**
         * @brief Reference counter.
         */
        std::atomic<int> m_count = 0;
    };

    /**
     * @brief Reference wrapper utility.
     */
    template<typename Ref>
    class ref {
    public:
        /**
         * @brief Construct an empty reference wrapper.
         */
        ref() = default;

        /**
         * @brief Construct a reference wrapper.
         * 
         * @param ptr Pointer to a reference.
         */
        ref(Ref* ptr) : m_ptr(ptr) { retain(); }

        /**
         * @brief Construct an empty reference wrapper.
         */
        ref(std::nullptr_t) : m_ptr(nullptr) {}

        /**
         * @brief Copy constructor.
         * 
         * @param ref Reference wrapper to copy from.
         */
        ref(const ref<Ref>& ref) : m_ptr(ref.m_ptr) { retain(); }

        /**
         * @brief Copy constructor.
         *
         * @param ref Reference wrapper to copy from.
         */
        template<typename Derived, std::enable_if_t<std::is_base_of_v<Ref, Derived>, int> = 0>
        ref(const ref<Derived>& ref) : m_ptr(ref.m_ptr) { retain(); }

        /**
         * @brief Move constructor.
         *
         * @param ref Reference wrapper to move from.
         */
        ref(ref<Ref>&& ref) noexcept : m_ptr(ref.m_ptr) { ref.m_ptr = nullptr; }

        /**
         * @brief Construct reference wrapper with moved value.
         *
         * @param value Value to initialize.
         */
        ref(Ref&& value) : m_ptr(new Ref(std::forward<Ref>(value))) { retain(); }

        /**
         * @brief Destruct a reference wrapper.
         */
        ~ref() { release(); }

        /**
         * @brief Copy assignment.
         *
         * @param ref Reference wrapper to copy from.
         */
        ref<Ref>& operator=(const ref<Ref>& ref) {
            release();
            m_ptr = ref.m_ptr;
            retain();
            return *this;;
        }

        /**
         * @brief Copy assignment.
         *
         * @param ref Reference wrapper to copy from.
         */
        template<typename Derived, std::enable_if_t<std::is_base_of_v<Ref, Derived>, int> = 0>
        ref<Ref>& operator=(const ref<Derived>& ref) {
            release();
            m_ptr = ref.m_ptr;
            retain();
            return *this;
        }

        /**
         * @brief Move assignment.
         *
         * @param ref Reference wrapper to move from.
         */
        ref<Ref>& operator=(ref<Ref>&& ref) noexcept {
            release();
            m_ptr = ref.m_ptr;
            ref.m_ptr = nullptr;
            return *this;
        }

        /**
         * @brief Null pointer assignment.
         */
        ref<Ref>& operator=(std::nullptr_t) {
            release();
            m_ptr = nullptr;
            return *this;
        }

        /**
         * @brief Dereference the stored pointer.
         * 
         * @return The result of dereferencing the stored pointer, i.e. *get().
         */
        Ref& operator*() const { return *m_ptr; }

        /**
         * @brief Get the stored pointer.
         *
         * @return The stored pointer, i.e. get().
         */
        Ref* operator->() const { return m_ptr; }

        /**
         * @brief Get the stored pointer.
         *
         * @return Stored pointer.
         */
        Ref* get() const { return m_ptr; }

    private:
        /**
         * @brief Increment reference counter if ptr is exists.
         */
        void retain() {
            if (m_ptr) {
                m_ptr->retain();
            }
        }

        /**
         * @brief Decrement reference counter  if ptr is exists.
         */
        void release() {
            if (m_ptr) {
                m_ptr->release();
            }
        }

        /**
         * @brief Pointer to a reference.
         */
        Ref* m_ptr = nullptr;
    };
}
