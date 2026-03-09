import re

import voluptuous as vol
from homeassistant import config_entries

from .const import CONF_ESP_NAME, DOMAIN


class LedEspConfigFlow(config_entries.ConfigFlow, domain=DOMAIN):
    VERSION = 1

    async def async_step_user(self, user_input=None):
        errors = {}

        if user_input is not None:
            name = user_input[CONF_ESP_NAME].strip().lower()
            if not re.match(r"^[a-z0-9_]+$", name):
                errors[CONF_ESP_NAME] = "invalid_name"
            else:
                await self.async_set_unique_id(name)
                self._abort_if_unique_id_configured()
                return self.async_create_entry(
                    title=name.replace("_", " ").title(),
                    data={CONF_ESP_NAME: name},
                )

        return self.async_show_form(
            step_id="user",
            data_schema=vol.Schema({vol.Required(CONF_ESP_NAME): str}),
            errors=errors,
        )
