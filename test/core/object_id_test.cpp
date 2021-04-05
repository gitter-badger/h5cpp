//
// (c) Copyright 2017 DESY,ESS
//               2021 Eugen Wintersberger <eugen.wintersberger@gmail.com>
//
// This file is part of h5pp.
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
// Author: Martin Shetty <martin.shetty@esss.se>
//         Eugen Wintersberger <eugen.wintersberger@gmail.com>
// Created on: Sep 13, 2017
//

/*

 Summary of test results

  hid does not uniquely identify an h5 node

  object_address is stable across all scenarios

  file_number:
     not equal if same file closed and reopened
     identifies the file that owns object, not owner of link (in case of ext
 link)

  file_name:
     not equal if file opened via symbolic link (h5 allows opening same file
 twice) not equal if object opened via external link vs. original node


  Conclusions:
    file_number adequately reflects identity, unless file has been closed and
 reopened. file_name does not adequately reflect identity. Resolving symbolic
 link would solve part of the problem, but would also need to somehow
 dereference external links to identify the name of the object-owning file.

*/
#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include <h5cpp/core/filesystem.hpp>
#include <h5cpp/core/hdf5_capi.hpp>
#include <h5cpp/core/object_handle.hpp>
#include <h5cpp/core/object_id.hpp>
#include <h5cpp/core/types.hpp>
#include <iostream>
#include <tuple>

static const fs::path kFilePath1 = fs::absolute("file1.h5");
static const fs::path kFilePath2 = fs::absolute("file2.h5");
static const fs::path kFilePath3 = fs::absolute("file3.h5");

using namespace hdf5;

hid_t to_hid(const ObjectHandle& handle) {
  return static_cast<hid_t>(handle);
}
ObjectHandle h5f_create(const fs::path& path) {
  const char *path_ptr = path.string().data();
  hid_t id = H5Fcreate(path_ptr, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
  return ObjectHandle(id);
}

ObjectHandle h5g_create(const ObjectHandle& parent, const std::string& name) {
  return ObjectHandle(H5Gcreate(to_hid(parent), name.data(), H5P_DEFAULT,
                                H5P_DEFAULT, H5P_DEFAULT));
}

auto open = [](const fs::path& p) {
  return H5Fopen(p.string().data(), H5F_ACC_RDONLY, H5P_DEFAULT);
};

auto openrw = [](const fs::path& p) {
  return H5Fopen(p.string().data(), H5F_ACC_RDWR, H5P_DEFAULT);
};

void hard_link(const ObjectHandle& parent,
               const std::string& orig_path,
               const std::string& link_path) {
  REQUIRE(H5Lcreate_hard(to_hid(parent), orig_path.data(), to_hid(parent),
                         link_path.data(), H5P_DEFAULT, H5P_DEFAULT) >= 0);
}

void soft_link(const ObjectHandle& parent,
               const std::string& orig_path,
               const std::string& link_path) {
  REQUIRE(H5Lcreate_soft(orig_path.data(), to_hid(parent), link_path.data(),
                         H5P_DEFAULT, H5P_DEFAULT) >= 0);
}
struct File {
  fs::path file_path;
  ObjectHandle file;
  ObjectHandle group1;
  ObjectHandle group2;
  ObjectHandle dataset;

  File(const fs::path& path)
      : file_path{path},
        file{h5f_create(file_path)},
        group1{h5g_create(file, "/group1")},
        group2{h5g_create(file, "/group2")},
        dataset{} {
    Dimensions dims{3, 3};
    ObjectHandle space(
        H5Screate_simple(static_cast<int>(dims.size()), dims.data(), nullptr));
    dataset = ObjectHandle(H5Dcreate(
        to_hid(group1), "dset1", H5T_NATIVE_DOUBLE,
        to_hid(space), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT));
  }
};

SCENARIO("testing Id construction") {
  GIVEN("a default constructed id") {
    ObjectId id;
    THEN("the file number must be 0") { REQUIRE(id.file_number() == 0l); }
    THEN("the objects' address must be 0") {
      REQUIRE(id.object_address() == 0l);
    }
    THEN("the filename must be an empty string") {
      REQUIRE(id.file_name().empty());
    }
    AND_GIVEN("a default constructed handle") {
      ObjectHandle handle;
      WHEN("constructing a new ID from this handle") {
        ObjectId id2(handle);
        THEN("this id must be equal to the default constructed id") {
          REQUIRE(id == id2);
        }
      }
      THEN("retrieving the file name from this handle must fail") {
        REQUIRE_THROWS_AS(ObjectId::get_file_name(handle), std::runtime_error);
      }
      THEN("retrieving the info instance from this handle must fail") {
        REQUIRE_THROWS_AS(ObjectId::get_info(handle), std::runtime_error);
      }
    }
  }

  GIVEN("a single HDF5 file with some content") {
    File file1(kFilePath1);
    WHEN("constructing an id from the handle to group1") {
      ObjectId id(file1.group1);
      THEN("we can retrieve the filename from this id instance") {
        REQUIRE(id.file_name() == kFilePath1);
      }
    }
  }
  fs::remove(kFilePath1);
}

/*
SCENARIO("testing info retrievel from a file") {
  GIVEN("an HDF5 file with some content") {
    File file1(kFilePath1);
    THEN("we can obtain the filename") {
      REQUIRE(ObjectId::get_file_name(file1.group1) == kFilePath1);
    }
    THEN("we can obtain object info via that id") {
      REQUIRE_NOTHROW(ObjectId::get_info(file1.group1));
    }
  }
  fs::remove(kFilePath1);
}
SCENARIO("working with links") {
  GIVEN("a file object with a group and a dataset") {
    File file1(kFilePath1);
    ObjectId id_group1(file1.group1);
    ObjectId id_dataset(file1.dataset);

    GIVEN("a hard link to group1 with name group3") {
      hard_link(file1.file, "/group1", "/group3");
      AND_GIVEN("an id to group3") {
        ObjectHandle g(H5Gopen(to_hid(file1.file), "/group3", H5P_DEFAULT));
        THEN("the handles must not be equal") {
          REQUIRE(to_hid(g) != to_hid(file1.group1));
        }
        WHEN("constructing an id from this handle") {
          ObjectId id(g);
          THEN("all id attributes must be equal") {
            REQUIRE(id_group1.file_name() == id.file_name());
            REQUIRE(id_group1 == id);
          }
        }
      }
    }
    GIVEN("a soft link to group1 with name group4") {
      soft_link(file1.file, "/group1", "/group4");
      AND_GIVEN("a handle to group4") {
        ObjectHandle g(H5Gopen(to_hid(file1.file), "/group4", H5P_DEFAULT));
        THEN("the handles to group4 and group1 must be different") {
          REQUIRE(to_hid(g) != to_hid(file1.group1));
        }
        WHEN("constructing an id from this handle") {
          ObjectId id(g);
          THEN("all the ID parameters must match the original") {
            REQUIRE(id_group1.file_name() == id.file_name());
            REQUIRE(id_group1 == id);
          }
        }
      }
    }
    GIVEN("a hard link to dset1 of name dset2") {
      hard_link(file1.file, "/group1/dset1", "/group2/dset2");
      AND_GIVEN("a handle to /group2/dset2") {
        ObjectHandle d(H5Dopen(to_hid(file1.group2), "dset2", H5P_DEFAULT));
        THEN("handles must not be equal") {
          REQUIRE(to_hid(d) != to_hid(file1.dataset));
        }
        WHEN("constructing and id from this handle") {
          ObjectId id(d);
          THEN("all id parameters must be equal") {
            REQUIRE(id_dataset.file_name() == id.file_name());
            REQUIRE(id_dataset == id);
          }
        }
      }
    }
    GIVEN("a soft link to dset1 of name /group2/dset3") {
      soft_link(file1.file, "/group1/dset1", "/group2/dset3");
      AND_GIVEN("a handle to /group2/dset3") {
        ObjectHandle d(H5Dopen(to_hid(file1.group2), "dset3", H5P_DEFAULT));
        THEN("handles must not be equal") {
          REQUIRE(to_hid(d) != to_hid(file1.dataset));
        }
        WHEN("constructing an id from this handle") {
          ObjectId id(d);
          THEN("all id parameters must be equal") {
            REQUIRE(id_dataset.file_name() == id.file_name());
            REQUIRE(id_dataset == id);
          }
        }
      }
    }
  }
  fs::remove(kFilePath1);
}


SCENARIO("checking copies and files of identical structure") {
  {
    // create two files with identical structure
    FileGuard{file_path_1()};
    FileGuard{file_path_2()};
  }

#ifdef WITH_BOOST
  fs::copy_file(file_path_1(), file_path_1(),
                fs::copy_option::overwrite_if_exists);
#else
  fs::copy_file(kFilePath_1, kFilePath_2, fs::copy_options::overwrite_existing);
#endif

  using files_t = std::tuple<fs::path, fs::path>;
  auto files = GENERATE(
      table<fs::path, fs::path>({files_t{file_path_1(), file_path_2()},
                                 files_t{file_path_1(), file_path_2()}}));
  GIVEN("a handle to group 1 in the first file") {
    ObjectHandle file1{open(std::get<0>(files))};
    ObjectHandle g_file1(H5Gopen(to_hid(file1), "/group1", H5P_DEFAULT));
    AND_GIVEN("a handle to group 1 in the second file") {
      ObjectHandle file2{open(std::get<1>(files))};
      ObjectHandle g_file2(H5Gopen(to_hid(file2), "/group1", H5P_DEFAULT));
      THEN("we expect the two handles to be different") {
        REQUIRE(to_hid(g_file1) != to_hid(g_file2));
      }
      WHEN("constructing ids from this group handles") {
        ObjectId id_file1(g_file1);
        ObjectId id_file2(g_file2);
        THEN("the file names must be different") {
          REQUIRE(id_file1.file_name() != id_file2.file_name());
        }
        THEN("the file number must be different") {
          REQUIRE(id_file1.file_number() != id_file2.file_number());
        }
        THEN("the object address in the file must be equal") {
          REQUIRE(id_file1.object_address() == id_file2.object_address());
        }
      }
    }
  }
  fs::remove(file_path_1());
  fs::remove(file_path_2());
  fs::remove(file_path_3());
}

#ifndef _MSC_VER
// Symbolic link (in OS) is made FILE2 -> FILE1
//   only file_number and object_address are equal
//   file_name is not equal
SCENARIO("testing symbolic links") {
  { FileGuard{file_path_1()}; }

  // Symlink FILE2 -> FILE1
  fs::create_symlink(file_path_1(), file_path_2());

  REQUIRE(fs::canonical(file_path_1()) == fs::canonical(file_path_2()));
  GIVEN("a handler to the first group in the original file") {
    ObjectHandle orig(open(file_path_1()));
    ObjectHandle link(open(file_path_2()));
    ObjectHandle g_orig(H5Gopen(to_hid(orig), "/group1", H5P_DEFAULT));
    AND_GIVEN("a handle to the group in the symbolic link") {
      ObjectHandle g_link(H5Gopen(to_hid(link), "/group1", H5P_DEFAULT));
      THEN("the two handle must be different") {
        REQUIRE(to_hid(g_orig) != to_hid(g_link));
      }
      WHEN("constructing Ids for the the two handles") {
        ObjectId id_orig(g_orig);
        ObjectId id_link(g_link);
        THEN("the file names must be different") {
          REQUIRE(id_orig.file_name() != id_link.file_name());
        }
        THEN("the ids must be equal") { REQUIRE(id_orig == id_link); }
      }
    }
  }
  fs::remove(file_path_1());
  fs::remove(file_path_2());
}
#endif

void external_link(const fs::path& target_file,
                   const std::string& target_path,
                   const ObjectHandle& link_file,
                   const std::string& link_path) {
  REQUIRE(H5Lcreate_external(target_file.string().data(), target_path.c_str(),
                             to_hid(link_file), link_path.c_str(), H5P_DEFAULT,
                             H5P_DEFAULT) >= 0);
}

SCENARIO("testing with external links") {
  {
    FileGuard{file_path_1()};
    FileGuard{file_path_2()};
  }

  // Extlink file2/group3 -> file1/group1
  GIVEN("an external link in file2 to group1 in file1") {
    ObjectHandle file2(openrw(file_path_2()));
    external_link(file_path_1(), "/group1", file2, "/group3");
    AND_GIVEN("a handle to this external link group in file") {
      ObjectHandle linked(H5Gopen(to_hid(file2), "/group3", H5P_DEFAULT));
      AND_GIVEN("a handle to the original group in the first file") {
        ObjectHandle file1(open(file_path_1()));
        ObjectHandle original(H5Gopen(to_hid(file1), "/group1", H5P_DEFAULT));
        THEN("the two handles must be different") {
          REQUIRE(linked != original);
        }
        WHEN("creating the ids from this to handles") {
          ObjectId id_original(original);
          ObjectId id_linked(linked);
          THEN("all attributes of the ids must be equal") {
            REQUIRE(id_original.file_number() == id_linked.file_number());
            REQUIRE(id_original.file_name() == id_linked.file_name());
            REQUIRE(id_original.object_address() == id_linked.object_address());
          }
        }
      }
    }
  }

  fs::remove(file_path_1());
  fs::remove(file_path_2());
}

#ifndef _MSC_VER
// Symbolic link (in OS) is made FILE3 -> FILE1
// External link is made file2/group3 -> File3/group1
//   only file_number and object_address are equal
//   file_name is not equal
SCENARIO("testing wiht external synmbolic link") {
  {
    FileGuard{file_path_1()};
    FileGuard{file_path_2()};
  }

  // Symlink FILE3 -> FILE1
  fs::create_symlink(file_path_1(), file_path_3());

  // Extlink file2/group3 -> file3/group1
  ObjectHandle file2(openrw(file_path_2()));
  external_link(file_path_3(), "/group1", file2, "/group3");
  ObjectHandle group23(H5Gopen(to_hid(file2), "/group3", H5P_DEFAULT));

  // Original node
  ObjectHandle file1(open(file_path_1()));
  ObjectHandle group11(H5Gopen(to_hid(file1), "/group1", H5P_DEFAULT));

  // Node in symlinked file
  ObjectHandle file3(open(file_path_3()));
  ObjectHandle group31(H5Gopen(to_hid(file3), "/group1", H5P_DEFAULT));

  ObjectId info11(group11);
  ObjectId info31(group31);
  ObjectId info23(group23);

  REQUIRE(to_hid(group11) != to_hid(group31));
  REQUIRE(to_hid(group11) != to_hid(group23));
  REQUIRE(to_hid(group23) != to_hid(group31));

  REQUIRE(info11.file_name() != info31.file_name());
  REQUIRE(info31.file_name() == info23.file_name());
  REQUIRE(info11.file_name() != info23.file_name());

  REQUIRE(info11.file_number() == info31.file_number());
  REQUIRE(info11.file_number() == info23.file_number());
  REQUIRE(info31.file_number() == info23.file_number());

  REQUIRE(info11.object_address() == info31.object_address());
  REQUIRE(info11.object_address() == info23.object_address());
  REQUIRE(info31.object_address() == info23.object_address());

  fs::remove(file_path_1());
  fs::remove(file_path_2());
  fs::remove(file_path_3());
}
#endif

SCENARIO("opening an instance serveral time") {
  // This works because the ObjectId does not store any HDF5 object.
  // It only stores some metadata about an object.
  GIVEN("an id to the first file object") {
    ObjectId id1;
    {
      FileGuard file(file_path_1());
      id1 = ObjectId(file.file);
    }
    AND_GIVEN("an id to the file object opend the second time") {
      ObjectId id2;
      {
        FileGuard file(file_path_1());
        id2 = ObjectId(file.file);
      }
      THEN("the filenames must be equal") {
        REQUIRE(id1.file_name() == id2.file_name());
      }
      THEN("the file numbers must be different") {
        REQUIRE(id1.file_number() != id2.file_number());
      }
      THEN("the object addresses must be equal") {
        REQUIRE(id1.object_address() == id2.object_address());
      }
    }
  }
  fs::remove(file_path_1());
}

SCENARIO("comparing ids") {
  ObjectId file1_id, file2_id, group1_id, group2_id;

  {
    FileGuard file1(file_path_1());
    FileGuard file2(file_path_2());

    file1_id = ObjectId(file1.file);
    file2_id = ObjectId(file2.file);
    group1_id = ObjectId(file2.group1);
    group2_id = ObjectId(file2.group2);
  }

  REQUIRE(file1_id == file1_id);
  REQUIRE(group1_id == group1_id);
  REQUIRE(file1_id != file2_id);
  REQUIRE(group1_id != group2_id);
  REQUIRE(file1_id != group1_id);
  REQUIRE(file1_id != group2_id);
  REQUIRE(file2_id != group1_id);
  REQUIRE(file2_id != group2_id);
  REQUIRE(file1_id < file2_id);
  REQUIRE_FALSE(file1_id < file1_id);
  REQUIRE_FALSE(file2_id < file1_id);
  REQUIRE_FALSE(group2_id < group1_id);
  REQUIRE(group1_id < group2_id);
  REQUIRE(file1_id < group1_id);
  REQUIRE(file2_id < group1_id);

  // remove the two HDF5 file as the yare no longer required
  fs::remove(file_path_1());
  fs::remove(file_path_2());
}*/