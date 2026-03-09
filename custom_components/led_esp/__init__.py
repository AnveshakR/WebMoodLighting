from homeassistant.components.webhook import async_register, async_unregister
from homeassistant.config_entries import ConfigEntry
from homeassistant.core import HomeAssistant

from .const import CONF_ESP_NAME, DOMAIN

PLATFORMS = ["light", "select", "sensor"]


async def async_setup_entry(hass: HomeAssistant, entry: ConfigEntry) -> bool:
    name = entry.data[CONF_ESP_NAME]
    hass.data.setdefault(DOMAIN, {})
    hass.data[DOMAIN][entry.entry_id] = {"name": name}

    await hass.config_entries.async_forward_entry_setups(entry, PLATFORMS)

    webhook_id = f"led_esp_{name}_ip"

    async def handle_ip_webhook(hass, webhook_id, request):
        try:
            data = await request.json()
            ip = data.get("ip", "")
            sensor = hass.data[DOMAIN][entry.entry_id].get("ip_sensor")
            if sensor and ip:
                await sensor.async_set_ip(ip)
        except Exception:
            pass
        from aiohttp.web import Response
        return Response(text="ok")

    async_register(hass, DOMAIN, f"LED ESP {name} IP", webhook_id, handle_ip_webhook)
    hass.data[DOMAIN][entry.entry_id]["webhook_id"] = webhook_id

    return True


async def async_unload_entry(hass: HomeAssistant, entry: ConfigEntry) -> bool:
    webhook_id = hass.data[DOMAIN][entry.entry_id].get("webhook_id")
    if webhook_id:
        async_unregister(hass, webhook_id)

    unload_ok = await hass.config_entries.async_unload_platforms(entry, PLATFORMS)
    if unload_ok:
        hass.data[DOMAIN].pop(entry.entry_id)
    return unload_ok
