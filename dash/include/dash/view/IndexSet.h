#ifndef DASH__VIEW__INDEX_SET_H__INCLUDED
#define DASH__VIEW__INDEX_SET_H__INCLUDED

#include <dash/view/ViewTraits.h>
#include <dash/view/Origin.h>
#include <dash/view/Local.h>

#include <dash/internal/Logging.h>


namespace dash {

template <class ViewType>
const typename ViewType::index_set_type & index(const ViewType & v) {
  return v.index_set();
}


namespace detail {

template <class IndexSetType>
class IndexSetIterator {
  typedef IndexSetIterator<IndexSetType>        self_t;
  typedef typename IndexSetType::index_type index_type;
public:
  constexpr IndexSetIterator(
    const IndexSetType & index_set,
    index_type           position)
  : _index_set(index_set), _pos(position)
  { }

  constexpr index_type operator*() const {
    return _index_set[_pos];
  }

  constexpr self_t operator++(int) const {
    return self_t(_index_set, _pos+1);
  }

  constexpr self_t operator--(int) const {
    return self_t(_index_set, _pos-1);
  }

  constexpr self_t operator+(int i) const {
    return self_t(_index_set, _pos+i);
  }

  constexpr self_t operator-(int i) const {
    return self_t(_index_set, _pos-i);
  }

  constexpr index_type pos() const {
    return _pos;
  }

private:
  const IndexSetType & _index_set;
  index_type           _pos;
};

} // namespace detail

// -----------------------------------------------------------------------


/* NOTE: Local and global mappings of index sets should be implemented
 *       without IndexSet member functions like this:
 *
 *       dash::local(index_set) {
 *         return dash::index(
 *                  // map the index set's view to local type, not the
 *                  // index set itself:
 *                  dash::local(index_set.view())
 *                );
 *
 */


template <
  class IndexSetType,
  class ViewType >
class IndexSetBase;

template <
  class IndexSetType,
  class ViewType >
constexpr auto local(
  const IndexSetBase<IndexSetType, ViewType> & index_set)
-> decltype(index_set.local()) {
  return index_set.local();
}

template <
  class IndexSetType,
  class ViewType >
class IndexSetBase
{
public:
  typedef typename ViewType::index_type
    index_type;
  typedef typename dash::view_traits<ViewType>::origin_type
    origin_type;
  typedef typename dash::view_traits<ViewType>::domain_type
    view_domain_type;
  typedef typename ViewType::local_type
    view_local_type;
  typedef typename dash::view_traits<view_domain_type>::index_set_type
    index_set_domain_type;
  typedef typename origin_type::pattern_type
    pattern_type;
  typedef typename dash::view_traits<view_local_type>::index_set_type
    local_type;
public:
  typedef detail::IndexSetIterator<IndexSetType> iterator;

  constexpr explicit IndexSetBase(const ViewType & view)
  : _view(view), _pattern(dash::origin(view).pattern())
  { }

  constexpr iterator begin() const {
    return iterator(*static_cast<const IndexSetType *>(this), 0);
  }

  constexpr iterator end() const {
    return iterator(*static_cast<const IndexSetType *>(this),
                    static_cast<const IndexSetType *>(this)->size());
  }

  constexpr const ViewType & view() const {
    return _view;
  }

  constexpr const index_set_domain_type & domain() const {
    return dash::index(dash::domain(_view));
  }

  constexpr const pattern_type & pattern() const {
    return _pattern;
  }

private:
  const ViewType     & _view;
  const pattern_type & _pattern;
};

// -----------------------------------------------------------------------

template <class ViewType>
class IndexSetIdentity;

template <
  class ViewType >
constexpr const IndexSetIdentity<ViewType> & local(
  const IndexSetIdentity<ViewType> & index_set) {
  return index_set;
}

template <class ViewType>
class IndexSetIdentity
: public IndexSetBase<
           IndexSetIdentity<ViewType>,
           ViewType >
{
  typedef IndexSetIdentity<ViewType>                            self_t;
  typedef IndexSetBase<self_t, ViewType>                        base_t;
public:
  typedef typename ViewType::index_type                     index_type;

  constexpr explicit IndexSetIdentity(
    const ViewType & view)
  : base_t(view)
  { }

  constexpr index_type operator[](index_type image_index) const {
    return image_index;
  }

  constexpr index_type size() const {
    return dash::domain(*this).size();
  }
};

// -----------------------------------------------------------------------

template <
  class ViewType >
class IndexSetSub;

template <
  class ViewType >
constexpr auto local(
  const IndexSetSub<ViewType> & index_set)
-> decltype(index_set.local()) {
  return index_set.local();
}

template <class ViewType>
class IndexSetSub
: public IndexSetBase<
           IndexSetSub<ViewType>,
           ViewType >
{
  typedef IndexSetSub<ViewType>                                 self_t;
  typedef IndexSetBase<self_t, ViewType>                        base_t;
public:
  typedef typename ViewType::index_type                     index_type;
  typedef typename base_t::view_domain_type           view_domain_type;
  typedef typename base_t::local_type                       local_type;

  constexpr IndexSetSub(
    const ViewType   & view,
    index_type         begin,
    index_type         end)
  : base_t(view),
    _domain_begin_idx(begin),
    _domain_end_idx(end)
  { }

  constexpr index_type operator[](index_type image_index) const {
    return _domain_begin_idx + image_index;
  }

  constexpr index_type size() const {
    return _domain_end_idx - _domain_begin_idx;
  }

  constexpr const local_type & local() const {
    return dash::index(dash::local(this->view()));
  }

private:
  index_type _domain_begin_idx;
  index_type _domain_end_idx;
};

// -----------------------------------------------------------------------

template <class ViewType>
class IndexSetLocal;

template <
  class ViewType >
constexpr const IndexSetLocal<ViewType> & local(
  const IndexSetLocal<ViewType> & index_set) {
  return index_set;
}

template <class ViewType>
class IndexSetLocal
: public IndexSetBase<
           IndexSetLocal<ViewType>,
           ViewType >
{
  typedef IndexSetLocal<ViewType>                               self_t;
  typedef IndexSetBase<self_t, ViewType>                        base_t;
public:
  typedef typename ViewType::index_type                     index_type;
  typedef self_t                                            local_type;

  constexpr explicit IndexSetLocal(const ViewType & view)
  : base_t(view)
  { }

public:
  constexpr index_type operator[](index_type local_index) const {
    return this->pattern().global(
             // domain start index to local index:
             this->pattern().at(this->domain()[0]) +
             // apply specified local offset:
             local_index);
  }

  constexpr index_type size() const {
    return static_cast<index_type>(this->pattern().local_size());
  }

  constexpr const local_type & local() const {
    return *this;
  }
};

} // namespace dash

#endif // DASH__VIEW__INDEX_SET_H__INCLUDED