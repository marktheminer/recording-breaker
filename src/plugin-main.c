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

#include <obs.h>
#include <obs-module.h>
#include <plugin-support.h>
#include <obs-frontend-api.h>
#include <stdlib.h>
#include <util/platform.h>

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE(PLUGIN_NAME, "en-US")

bool restartRecording = false;

static void OBSEvent(enum obs_frontend_event event, void *private_data)
{
	switch (event) {
	case OBS_FRONTEND_EVENT_RECORDING_STOPPED: {
		if (restartRecording) {
			obs_log(LOG_INFO, "[recording-breaker]: Sleep");
			// very unreliable without the sleep
			os_sleep_ms(100);
			obs_log(LOG_INFO,
				"[recording-breaker]: obs_frontend_recording_start 1");
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
	}
}

bool obs_module_load(void)
{
	obs_log(LOG_INFO, "plugin loaded successfully (version %s)",
		PLUGIN_VERSION);
	obs_frontend_add_event_callback(OBSEvent, NULL);

	return true;
}

void obs_module_unload(void)
{
	obs_log(LOG_INFO, "plugin unloaded");
}
