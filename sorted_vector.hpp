// from http://lafstern.org/matt/col1.pdf
// NOTE: set semantics; inserting a duplicate has no effect
template <class T, class Compare = std::less<T> >
struct sorted_vector {
  std::vector<T> V;
  Compare cmp;

  typedef typename std::vector<T>::iterator iterator;
  typedef typename std::vector<T>::const_iterator const_iterator;
  inline iterator begin() { return V.begin(); }
  inline iterator end() { return V.end(); }
  inline const_iterator begin() const { return V.begin(); }
  inline const_iterator end() const { return V.end(); }

  inline size_t size() { return V.size(); }

  inline sorted_vector(Compare const& c = Compare())
  : V(), cmp(c)
  {}

  template <class InputIterator>
  inline sorted_vector(
    InputIterator first, InputIterator last,
    const Compare& c = Compare())
  : V(first, last), cmp(c)
  {
    std::sort(begin(), end(), cmp);
  }

  inline iterator insert(T const& t) {
    iterator i = std::lower_bound(begin(), end(), t, cmp);
    if (i == end() || cmp(t, *i))
      V.insert(i, t);
    return i;
  }

  inline const_iterator find(T const& t) const {
    const_iterator i = std::lower_bound(begin(), end(), t, cmp);
    return i == end() || cmp(t, *i) ? end() : i;
  }

  inline bool contains(T const& t) const {
    return find(t) != end();
  }
};
