/**
Copyright (c) 2015 Matthew Schnee

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
**/
#pragma once

#include <utility>

namespace EventEmitter
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
	TypeTag(_Base i = _Base()) : p(i) {}

	bool operator==(const TypeTag& other) { return p == other.p; }

	friend bool operator<(const TypeTag& lhs, const TypeTag& rhs)
	{
		return lhs.p < rhs.p;
	}

	bool operator<(const TypeTag& rhs)
	{
		return p < rhs.p;
	}

	TypeTag operator++(int inc)
	{
		return TypeTag(p++);
	}

	TypeTag& operator++()
	{
		++p;
		return *this;
	}

private:
	_Base p;
};


} // namespace Core
