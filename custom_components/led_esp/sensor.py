from __future__ import annotations

from homeassistant.components.sensor import SensorEntity
from homeassistant.config_entries import ConfigEntry
from homeassistant.core import HomeAssistant
from homeassistant.helpers.entity_platform import AddEntitiesCallback
from homeassistant.helpers.restore_state import RestoreSensor

from .const import CONF_ESP_NAME, DOMAIN


async def async_setup_entry(
    hass: HomeAssistant, entry: ConfigEntry, async_add_entities: AddEntitiesCallback
) -> None:
    name = entry.data[CONF_ESP_NAME]
    sensor = LedEspIPSensor(name)
    hass.data[DOMAIN][entry.entry_id]["ip_sensor"] = sensor
    async_add_entities([sensor])


class LedEspIPSensor(SensorEntity, RestoreSensor):
    _attr_should_poll = False

    def __init__(self, name: str) -> None:
        self._attr_unique_id = f"led_esp_{name}_ip"
        self._attr_name = f"{name}_ip"      # entity_id: sensor.<name>_ip
        self._ip = ""

    async def async_added_to_hass(self) -> None:
        last = await self.async_get_last_sensor_data()
        if last:
            self._ip = last.native_value or ""

    @property
    def native_value(self) -> str:
        return self._ip

    async def async_set_ip(self, ip: str) -> None:
        self._ip = ip
        self.async_write_ha_state()
