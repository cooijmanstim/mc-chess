#include <boost/thread.hpp>

// like boost::optional, but dereferencing blocks until there is data
template <typename T>
class Sometimes<T> {
  boost::optional<T> optional_data;
  boost::mutex mutex;
  boost::condition_variable data_becomes_available_condition;

public:
  // returns by value because otherwise the data may be unset by the time the
  // caller gets to it
  inline
  T operator * () {
    boost::lock_guard lock(mutex);
    while (!optional_data)
      data_becomes_available_condition.wait();
    return *optional_data;
  }

  inline
  template <typename U>
  Sometimes<T>& operator = (U const& u) {
    boost::lock_guard lock(mutex);
    optional_data = u;
    data_becomes_available_condition.notify_all();
    return *this;
  }
};
