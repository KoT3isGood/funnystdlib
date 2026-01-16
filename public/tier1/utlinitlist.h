//================= Copyright kotofyt, All rights reserved ==================//
// C++ only supports std::initializer_list, so we need to get around compiler.
//
// fuck C++ once
// fuck C++ twice
// fuck C++ thrice
//===========================================================================//

#ifndef TIER1_UTL_INITIALIZER_LIST_H
#define TIER1_UTL_INITIALIZER_LIST_H

#include "stddef.h"

// from GCC 
namespace std
{
	template<class _E>
	class initializer_list
	{
	public:
		typedef _E 		value_type;
		typedef const _E& 	reference;
		typedef const _E& 	const_reference;
		typedef size_t 		size_type;
		typedef const _E* 	iterator;
		typedef const _E* 	const_iterator;

	private:
		iterator			_M_array;
		size_type			_M_len;

		constexpr initializer_list(const_iterator __a, size_type __l)
			: _M_array(__a), _M_len(__l) { }

	public:
		constexpr initializer_list() noexcept
			: _M_array(0), _M_len(0) { }

		constexpr size_type
			size() const noexcept { return _M_len; }

		constexpr const_iterator
			begin() const noexcept { return _M_array; }

		constexpr const_iterator
			end() const noexcept { return begin() + size(); }
	};

	template<class _Tp>
		constexpr const _Tp*
		begin(initializer_list<_Tp> __ils) noexcept
		{ return __ils.begin(); }

	template<class _Tp>
		constexpr const _Tp*
		end(initializer_list<_Tp> __ils) noexcept
		{ return __ils.end(); }
}

template<typename T>
using CUtlInitializerList = std::initializer_list<T>;


#endif
