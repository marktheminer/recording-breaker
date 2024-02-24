/*
Plugin Name
Copyright (C) 2024 Mark Jones <mark.jones1112+nospam@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program. If not, see <https://www.gnu.org/licenses/>
*/

#include <obs.hpp>
#include <obs-module.h>
#include <plugin-support.h>
#include <obs-frontend-api.h>
#include <stdlib.h>
#include <util/platform.h>
#include <QAction>

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE(PLUGIN_NAME, "en-US")

bool enabled = false;

bool restartRecording = false;

QAction *action = nullptr;

static void OBSEvent(enum obs_frontend_event event, void *private_data)
{
	if (!enabled)
		return;

	switch (event) {
	case OBS_FRONTEND_EVENT_RECORDING_STOPPED: {
		if (restartRecording) {
			restartRecording = false;
			obs_log(LOG_INFO, "[%s]: Sleep 100ms", PLUGIN_NAME);
			// very unreliable without the sleep
			os_sleep_ms(100);
			obs_frontend_recording_start();
		}
		break;
	}
	case OBS_FRONTEND_EVENT_SCENE_CHANGED: {
		if (obs_frontend_recording_active()) {
			restartRecording = true;
			obs_frontend_recording_stop();
		}
		break;
	}
	default: {
		break;
	}
	}
}

static void HandleSettings(obs_data_t *save_data, bool saving, void *)
{
	if (saving) {
		const OBSDataAutoRelease obj = obs_data_create();
		obs_data_set_bool(obj, "enabled", enabled);
		blog(LOG_INFO, "[%s]: About to save data", PLUGIN_NAME);
		obs_data_set_obj(save_data, PLUGIN_NAME, obj);
		blog(LOG_INFO, "[%s]: Done saving data", PLUGIN_NAME);
	} else {
		OBSDataAutoRelease obj =
			obs_data_get_obj(save_data, PLUGIN_NAME);

		if (!obj)
			obj = obs_data_create();
		blog(LOG_INFO, "[%s]: loading variables", PLUGIN_NAME);
		enabled = obs_data_get_bool(obj, "enabled");
		if (action != nullptr)
			action->setChecked(enabled);
	}
}

extern "C" bool obs_module_load(void)
{
	obs_log(LOG_INFO, "[%s]: plugin loaded successfully (version %s)",
		PLUGIN_NAME, PLUGIN_VERSION);
	obs_frontend_add_event_callback(OBSEvent, NULL);
	action = (QAction *)obs_frontend_add_tools_menu_qaction(
		obs_module_text("RecordingBreakerAction"));
	action->setCheckable(true);
	action->setChecked(enabled);
	const auto cb = []() {
		enabled = !enabled;
		action->setChecked(enabled);
		obs_log(LOG_INFO, "Scene change recording breaker enabled = %i",
			enabled);
	};
	action->connect(action, &QAction::triggered, cb);
	obs_frontend_add_save_callback(HandleSettings, nullptr);

	return true;
}

extern "C" void obs_module_unload(void)
{
	obs_log(LOG_INFO, "[%s]: plugin unloaded", PLUGIN_NAME);
}
