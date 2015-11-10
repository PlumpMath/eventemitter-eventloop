#pragma once

#include <utility>

namespace Core
{

/**
 * @brief A way to strictly type common typedefs.
 * @code
 *  typedef TypeTag<unsigned int, class _event_id_type>    EventId;
 *  typedef TypeTag<unsigned int, class _listener_id_type> ListenerId;
 *  ListenerId lid = 1;
 *  EventId    eid = 2;
 *  lid = eid ; // compile-time error!
 * @endcode
 *
 * @tparam     _Base  { description }
 * @tparam     _Tag   { description }
 */


template <class _Base, class _Tag> // requires LessThanComparable<_Base>
class TypeTag
{
public:
    explicit TypeTag(_Base i = _Base()) : p(i) {}
    bool operator==(const TypeTag& other) { return p == other.p; }

	friend bool operator<(const TypeTag<_Base, _Tag>& lhs, const TypeTag<_Base, _Tag>& rhs)
	{
		return lhs.p < rhs.p;
	}

	bool operator<(const TypeTag<_Base, _Tag>& rhs)
	{
		return p < rhs.p;
	}

	TypeTag<_Base, _Tag> operator++(int inc)
	{
		return TypeTag(p++);
	}

	TypeTag<_Base, _Tag>& operator++()
	{
		++p;
		return *this;
	}

private:
    _Base p;
};

} // namespace Core
