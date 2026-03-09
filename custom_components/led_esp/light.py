from __future__ import annotations

from typing import Any

from homeassistant.components.light import ATTR_RGB_COLOR, ColorMode, LightEntity
from homeassistant.config_entries import ConfigEntry
from homeassistant.core import HomeAssistant
from homeassistant.helpers.entity_platform import AddEntitiesCallback
from homeassistant.helpers.restore_state import RestoreEntity

from .const import CONF_ESP_NAME, DOMAIN


async def async_setup_entry(
    hass: HomeAssistant, entry: ConfigEntry, async_add_entities: AddEntitiesCallback
) -> None:
    name = entry.data[CONF_ESP_NAME]
    async_add_entities([LedEspLight(name)])


class LedEspLight(LightEntity, RestoreEntity):
    _attr_color_mode = ColorMode.RGB
    _attr_supported_color_modes = {ColorMode.RGB}
    _attr_should_poll = False

    def __init__(self, name: str) -> None:
        self._attr_unique_id = f"led_esp_{name}_light"
        self._attr_name = name          # entity_id becomes light.<name>
        self._is_on = False
        self._rgb: tuple[int, int, int] = (255, 255, 255)

    async def async_added_to_hass(self) -> None:
        last = await self.async_get_last_state()
        if last:
            self._is_on = last.state == "on"
            rgb = last.attributes.get("rgb_color")
            if rgb:
                self._rgb = tuple(rgb)

    @property
    def is_on(self) -> bool:
        return self._is_on

    @property
    def rgb_color(self) -> tuple[int, int, int]:
        return self._rgb

    async def async_turn_on(self, **kwargs: Any) -> None:
        self._is_on = True
        if ATTR_RGB_COLOR in kwargs:
            self._rgb = kwargs[ATTR_RGB_COLOR]
        self.async_write_ha_state()

    async def async_turn_off(self, **kwargs: Any) -> None:
        self._is_on = False
        self.async_write_ha_state()
