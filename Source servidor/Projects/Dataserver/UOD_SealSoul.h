#pragma once

#include "pugixml.hpp"
#include "stBase.h"

bool ReadSealInfo(int id, SealFileInfo& info);
bool WriteSealInfo(int id, const SealFileInfo& info);