//
// (c) Copyright 2019 Eugen Wintersberger <eugen.wintersberger@gmail.com>
//
// This file is part of h5cpp.
//
// This library is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published
// by the Free Software Foundation; either version 2.1 of the License, or
// (at your option) any later version.
//
// This library is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty ofMERCHANTABILITY
// or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
// License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this library; if not, write to the
// Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor
// Boston, MA  02110-1301 USA
// ===========================================================================
//
// Author: Eugen Wintersberger <eugen.wintersberger@gmail.com>
// Created on: Sep 1, 2019
//
#pragma once
#include <h5cpp/dataspace/dataspace.hpp>
#include <h5cpp/datatype/datatype.hpp>

namespace hdf5 {
namespace type {


template<typename T>
class Trait
{
  public:

    static T create(const datatype::Datatype & = datatype::Datatype(),
                    const dataspace::Dataspace & = dataspace::Dataspace())
    {
      return T();
    }

    static void* pointer(T &value)
    {
      return reinterpret_cast<void*>(&value);
    }

    static const void* pointer(const T &value)
    {
      return reinterpret_cast<const void*>(&value);
    }
};




}
}

