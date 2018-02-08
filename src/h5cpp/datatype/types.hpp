//
// (c) Copyright 2017 DESY,ESS
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
// Authors:
//   Eugen Wintersberger <eugen.wintersberger@desy.de>
//   Martin Shetty <martin.shetty@esss.se>
// Created on: Aug 23, 2017
//
#pragma once

#include <iostream>
#include <type_traits>
#include <h5cpp/core/hdf5_capi.hpp>
#include <h5cpp/core/windows.hpp>

namespace hdf5 {
namespace datatype {

const decltype(H5T_VARIABLE) kVariable = H5T_VARIABLE;

//!
//! \enum data type class
//!
//! Enumeration type describing the class a particular data type belongs to.
//!
enum class Class : std::underlying_type<H5T_class_t>::type
{
  NONE = H5T_NO_CLASS,         //!< indicates a non-type
  INTEGER = H5T_INTEGER,       //!< indicates an integer type
  FLOAT = H5T_FLOAT,           //!< indicates a float type
  TIME = H5T_TIME,             //!< indicates a time type
  STRING = H5T_STRING,         //!< indicates a string type
  BITFIELD = H5T_BITFIELD,     //!< indicates a bitfield type
  OPAQUE = H5T_OPAQUE,         //!< indicates an opaque type
  COMPOUND = H5T_COMPOUND,     //!< indicates a compound type
  REFERENCE = H5T_REFERENCE,   //!< indicates a reference type
  ENUM = H5T_ENUM,             //!< indicates an enumeration type
  VARLENGTH = H5T_VLEN,        //!< indicates a variable length type
  ARRAY = H5T_ARRAY            //!< indicates an array type
};

//!
//! @brief stream output operator for the Class enumeration
//! @relates Class
//!
//! @param stream reference to the output stream
//! @param c reference to the class enumeration
//! @return modified output stream
//!
DLL_EXPORT std::ostream &operator<<(std::ostream &stream, const Class &c);

//!
//! \brief enumeration type for byte order
//!
enum class Order : std::underlying_type<H5T_order_t>::type
{
  LE = H5T_ORDER_LE, //!< littlen endian type
  BE = H5T_ORDER_BE  //!< big endian type
};

//!
//! @brief output stream operator for the Order enumeration
//!
//! @param stream reference to the output stream
//! @param o reference to an Order enumeration instance
//! @return modified output stream
//!
DLL_EXPORT std::ostream &operator<<(std::ostream &stream, const Order &o);

//!
//! \brief enumeration for sign
//!
enum class Sign : std::underlying_type<H5T_sign_t>::type
{
  TWOS_COMPLEMENT = H5T_SGN_2, //!< indicates a signed type
  UNSIGNED = H5T_SGN_NONE      //!< indicates an unsigned type
};

//!
//! @brief output stream for Sign enumeration
//!
//! @param stream reference to the output stream
//! @param s reference to the Sign enumeration
//! @return modified output stream
//!
DLL_EXPORT std::ostream &operator<<(std::ostream &stream, const Sign &s);

//!
//! \brief enumeration describes mantissa norm
//!
enum class Norm : std::underlying_type<H5T_norm_t>::type
{
  IMPLIED = H5T_NORM_IMPLIED,
  MSBSET = H5T_NORM_MSBSET,
  NONE = H5T_NORM_NONE
};

//!
//! @brief stream output operator for the Norm enumeration
//!
//! @param stream reference to the output stream
//! @param n refernece to a Norm enumeration instance
//! @return modified ouput stream
//!
DLL_EXPORT std::ostream &operator<<(std::ostream &stream, const Norm &n);

//!
//! \brief enumeration for floating point padding
//!
enum class Pad : std::underlying_type<H5T_pad_t>::type
{
  ZERO = H5T_PAD_ZERO,
  ONE = H5T_PAD_ONE,
  BACKGROUND = H5T_PAD_BACKGROUND
};

//!
//! @brief stream output operator for Pad enumerations
//!
//! @param stream reference to the output stream
//! @param p reference to a Pad enumeratino instance
//! @return modified output stream
//!
DLL_EXPORT std::ostream &operator<<(std::ostream &stream, const Pad &p);

//!
//! \brief enumeration determining string padding
//!
enum class StringPad : std::underlying_type<H5T_str_t>::type
{
  NULLTERM = H5T_STR_NULLTERM, //!< indicates a null terminated string type
  NULLPAD = H5T_STR_NULLPAD,   //!< indicates a null padded string type
  SPACEPAD = H5T_STR_SPACEPAD  //!< indicates a space padded string type
};

//!
//! @brief stream output operator for StringPad enumerations
//!
//! @param stream reference to an output stream
//! @param pad reference to a StringPad enumeration instance
//! @return modified output stream
//!
DLL_EXPORT std::ostream &operator<<(std::ostream &stream, const StringPad &pad);

enum class Direction : std::underlying_type<H5T_direction_t>::type
{
  ASCEND = H5T_DIR_ASCEND,
  DESCEND = H5T_DIR_DESCEND
};

//!
//! @brief stream output operator for Direction enumerations
//!
//! @param stream reference to an output stream
//! @param d reference to a Direction enumeration instance
//! @return modified output stream
//!
DLL_EXPORT std::ostream &operator<<(std::ostream &stream, const Direction &d);

//!
//! @brief character set encoding
//!
//! Enumeration type determining the character encoding used by string types
//! and links.
//!
enum class CharacterEncoding : std::underlying_type<H5T_cset_t>::type {
  ASCII = H5T_CSET_ASCII, //!< ASCII
  UTF8 = H5T_CSET_UTF8    //!< UTF8
};

//!
//! @brief stream output operator for CharacterEncoding enumerations
//!
//! @param stream reference to an output stream
//! @param enc reference to a CharacterEncoding enumeration instance
//! @return modified output stream
//!
DLL_EXPORT std::ostream &operator<<(std::ostream &stream, const CharacterEncoding &enc);

} // namespace datatype
} // namespace hdf5
