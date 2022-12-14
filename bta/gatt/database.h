/******************************************************************************
 *
 *  Copyright 2018 The Android Open Source Project
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at:
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 ******************************************************************************/

#pragma once

#include <set>
#include <string>
#include <utility>
#include <vector>

#include <bluetooth/uuid.h>
#include "stack/include/bt_types.h" /* for Octet16 */

namespace gatt {
constexpr uint16_t HANDLE_MIN = 0x0001;
constexpr uint16_t HANDLE_MAX = 0xffff;

/* Representation of GATT attribute for storage */
struct StoredAttribute {
  uint16_t handle;
  bluetooth::Uuid type;

  union {
    /* primary or secondary service definition */
    struct {
      bluetooth::Uuid uuid;
      uint16_t end_handle;
    } service;

    /* included service definition */
    struct {
      uint16_t handle;
      uint16_t end_handle;
      bluetooth::Uuid uuid;
    } included_service;

    /* characteristic definition */
    struct {
      uint8_t properties;
      uint16_t value_handle;
      bluetooth::Uuid uuid;
    } characteristic;

    /* for descriptor we store value only for
     * «Characteristic Extended Properties» */
    uint16_t characteristic_extended_properties;
  } value;
};

struct IncludedService;
struct Characteristic;
struct Descriptor;

struct Service {
  uint16_t handle;
  bluetooth::Uuid uuid;
  bool is_primary;
  uint16_t end_handle;
  std::vector<IncludedService> included_services;
  std::vector<Characteristic> characteristics;
};

struct IncludedService {
  uint16_t handle; /* definition handle */
  bluetooth::Uuid uuid;
  uint16_t start_handle; /* start handle of included service */
  uint16_t end_handle;   /* end handle of included service */
};

struct Characteristic {
  uint16_t declaration_handle;
  bluetooth::Uuid uuid;
  uint16_t value_handle;
  uint8_t properties;
  std::vector<Descriptor> descriptors;
};

struct Descriptor {
  uint16_t handle;
  bluetooth::Uuid uuid;
  /* set and used for «Characteristic Extended Properties» only */
  uint16_t characteristic_extended_properties;
};

class DatabaseBuilder;

class Database {
 public:
  /* Return true if there are no services in this database. */
  bool IsEmpty() const { return services.empty(); }

  /* Clear the GATT database. This method forces relocation to ensure no extra
   * space is used unnecesarly */
  void Clear() { std::vector<Service>().swap(services); }

  /* Return list of services available in this database */
  const std::vector<Service>& Services() const { return services; }

  std::string ToString() const;

  std::vector<gatt::StoredAttribute> Serialize() const;

  static Database Deserialize(const std::vector<gatt::StoredAttribute>& nv_attr,
                              bool* success);

  /* Return 128 bit unique identifier of this GATT database */
  Octet16 Hash() const;

  friend class DatabaseBuilder;

 private:
  std::vector<Service> services;
};

/* Find a service that should contain handle. Helper method for internal use
 * inside gatt namespace.*/
Service* FindService(std::vector<Service>& services, uint16_t handle);

}  // namespace gatt
