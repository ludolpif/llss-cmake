#pragma once
/*
 * This file is part of LLSS.
 *
 * LLSS is free software: you can redistribute it and/or modify it under the terms of the
 * Affero GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * LLSS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with LLSS.
 * If not, see <https://www.gnu.org/licenses/>. See LICENSE file at root of this git repo.
 *
 * Copyright 2025 ludolpif <ludolpif@gmail.com>
 */
#include "app-components-core.h"

void AppSystemsCoreImport(ecs_world_t *world);

void InjectIOAsyncEvents(ecs_iter_t *it);
void ImGuiPrepareForNewFrame(ecs_iter_t *it);
void ImGuiSetupDockSpace(ecs_iter_t *it);
void ImGuiRenderAndSubmit(ecs_iter_t *it);
