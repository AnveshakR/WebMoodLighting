from __future__ import annotations

from homeassistant.components.select import SelectEntity
from homeassistant.config_entries import ConfigEntry
from homeassistant.core import HomeAssistant
from homeassistant.helpers.entity_platform import AddEntitiesCallback
from homeassistant.helpers.restore_state import RestoreEntity

from .const import CONF_ESP_NAME, DISPLAY_MODES, DOMAIN


async def async_setup_entry(
    hass: HomeAssistant, entry: ConfigEntry, async_add_entities: AddEntitiesCallback
) -> None:
    name = entry.data[CONF_ESP_NAME]
    async_add_entities([LedEspDisplaySelect(name)])


class LedEspDisplaySelect(SelectEntity, RestoreEntity):
    _attr_options = DISPLAY_MODES
    _attr_should_poll = False

    def __init__(self, name: str) -> None:
        self._attr_unique_id = f"led_esp_{name}_display_state"
        self._attr_name = f"{name}_display_state"   # entity_id: select.<name>_display_state
        self._current_option = "solid"

    async def async_added_to_hass(self) -> None:
        last = await self.async_get_last_state()
        if last and last.state in DISPLAY_MODES:
            self._current_option = last.state

    @property
    def current_option(self) -> str:
        return self._current_option

    async def async_select_option(self, option: str) -> None:
        self._current_option = option
        self.async_write_ha_state()
