# Copyright 2015 Google Inc. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := entity
LOCAL_ARM_MODE := arm
LOCAL_STATIC_LIBRARIES :=
LOCAL_SHARED_LIBRARIES :=

ENTITY_RELATIVE_DIR := ..
ENTITY_DIR := $(LOCAL_PATH)/$(ENTITY_RELATIVE_DIR)

LOCAL_EXPORT_C_INCLUDES := $(ENTITY_DIR)/include

LOCAL_C_INCLUDES := $(LOCAL_EXPORT_C_INCLUDES) $(ENTITY_DIR)/src

LOCAL_SRC_FILES := $(ENTITY_RELATIVE_DIR)/src/entity_manager.cpp

include $(BUILD_STATIC_LIBRARY)