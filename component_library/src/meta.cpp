// Copyright 2015 Google Inc. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <string.h>
#include "component_library/meta.h"
#include "component_library/rendermesh.h"
#include "component_library/common_services.h"
#include "mathfu/utilities.h"

FPL_ENTITY_DEFINE_COMPONENT(fpl::component_library::MetaComponent,
                            fpl::component_library::MetaData)

namespace fpl {
namespace component_library {

void MetaComponent::AddFromRawData(entity::EntityRef& entity,
                                   const void* raw_data) {
  MetaData* meta_data = AddEntity(entity);
  if (raw_data != nullptr) {
    auto meta_def = static_cast<const MetaDef*>(raw_data);
    if (meta_def->entity_id() != nullptr) {
      if (meta_data->entity_id != "") {
        RemoveEntityFromDictionary(meta_data->entity_id);
      }
      meta_data->entity_id = meta_def->entity_id()->c_str();
      AddEntityToDictionary(meta_data->entity_id, entity);
    }
    if (meta_def->prototype() != nullptr) {
      meta_data->prototype = meta_def->prototype()->c_str();
    }
    if (meta_def->comment() != nullptr) {
      meta_data->comment = meta_def->comment()->c_str();
    }
  }
}
void MetaComponent::AddFromPrototypeData(entity::EntityRef& entity,
                                         const MetaDef* meta_def) {
  MetaData* meta_data = AddEntity(entity);
  if (meta_def->comment() != nullptr) {
    meta_data->comment = meta_def->comment()->c_str();
  }
}

void MetaComponent::AddWithSourceFile(entity::EntityRef& entity,
                                      const std::string& source_file) {
  MetaData* data = AddEntity(entity);
  data->source_file = source_file;
  auto ext = data->source_file.rfind('.');
  if (ext != std::string::npos) {
    data->source_file = data->source_file.substr(0, ext);
  }
}

entity::ComponentInterface::RawDataUniquePtr MetaComponent::ExportRawData(
    entity::EntityRef& entity) const {
  const MetaData* data = GetComponentData(entity);
  if (data == nullptr) return nullptr;

  flatbuffers::FlatBufferBuilder fbb;
  // Const-cast should be okay here because we're just giving
  // something an entity ID before exporting it (if it doesn't already
  // have one).
  auto entity_id =
      fbb.CreateString(const_cast<MetaComponent*>(this)->GetEntityID(entity));
  auto prototype =
      (data->prototype != "") ? fbb.CreateString(data->prototype) : 0;
  auto comment = (data->comment != "") ? fbb.CreateString(data->comment) : 0;

  MetaDefBuilder builder(fbb);
  builder.add_entity_id(entity_id);
  if (data->prototype != "") {
    builder.add_prototype(prototype);
  }
  if (data->comment != "") {
    builder.add_comment(comment);
  }
  fbb.Finish(builder.Finish());
  return fbb.ReleaseBufferPointer();
}

void MetaComponent::InitEntity(entity::EntityRef& entity) {
  MetaData* data = GetComponentData(entity);
  if (data != nullptr && data->entity_id != "")
    AddEntityToDictionary(data->entity_id, entity);
}

void MetaComponent::CleanupEntity(entity::EntityRef& entity) {
  MetaData* data = GetComponentData(entity);
  if (data != nullptr && data->entity_id != "")
    RemoveEntityFromDictionary(data->entity_id);
}

const std::string& MetaComponent::GetEntityID(entity::EntityRef& entity) {
  MetaData* data = GetComponentData(entity);
  if (data == nullptr) {
    return empty_string;
  }
  if (data->entity_id == "") {
    // If this entity doesn't already have an ID, generate a new random one.
    GenerateRandomEntityID(&data->entity_id);
    AddEntityToDictionary(data->entity_id, entity);
  }
  return data->entity_id;
}

void MetaComponent::AddEntityToDictionary(const std::string& key,
                                          entity::EntityRef& entity) {
  entity_dictionary_[key] = entity;
}
void MetaComponent::RemoveEntityFromDictionary(const std::string& key) {
  auto i = entity_dictionary_.find(key);
  if (i != entity_dictionary_.end()) {
    entity_dictionary_.erase(i);
  }
}
entity::EntityRef MetaComponent::GetEntityFromDictionary(
    const std::string& key) {
  auto i = entity_dictionary_.find(key);
  if (i == entity_dictionary_.end()) {
    return entity::EntityRef();
  }
  if (!i->second.IsValid()) {
    // The entity with this key is no longer valid, we'd better remove it.
    RemoveEntityFromDictionary(key);
    return entity::EntityRef();
  }
  return i->second;
}

static const size_t kMaximumGeneratedEntityIDStringLength = 33;

void MetaComponent::GenerateRandomEntityID(std::string* output) {
  int num_digits = 16;
  const char digits[] = "0123456789abcdef";
  int digit_choices = strlen(digits);
  const char separator = '-';
  const int separator_every = 5;

  char name[kMaximumGeneratedEntityIDStringLength + 1];
  size_t i = 0;
  name[i++] = '$';
  for (; i < sizeof(name) - 1; i++) {
    if (i % separator_every != 0) {
      int random_digit = mathfu::RandomInRange(0, digit_choices - 1);
      name[i] = digits[random_digit];
      num_digits--;
    } else {
      name[i] = separator;
    }
    if (num_digits == 0) break;
  }
  name[i + 1] = '\0';
  // Do we actually need to check if the entity ID we generated is
  // already in the entity dictionary? It's pretty low odds...
  *output = std::string(name);
}

}  // namespace component_library
}  // namespace fpl