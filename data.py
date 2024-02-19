from dataclasses import dataclass
from typing import Optional


@dataclass
class WaterTankReadings:
    
    name: Optional[str] = None
    ip: Optional[str] = None
    wifi_connections_count: Optional[int] = None
    float_low: Optional[bool] = None
    float_high: Optional[bool] = None
    is_temp_sensor_ok: Optional[bool] = None
    is_pump_available: Optional[bool] = None
    is_heater_available: Optional[bool] = None
    is_mixer_available: Optional[bool] = None
    pump_state: Optional[bool] = None
    heater_state: Optional[bool] = None
    mixer_state: Optional[bool] = None
    low_temp: Optional[int] = None
    high_temp: Optional[int] = None
    temp: Optional[int] = None
    # time_left_to_fill: Optional[int] = None
    is_force_fill_tank_ready: Optional[bool] = None
    is_force_heat_water_ready: Optional[bool] = None
    is_shower_ready: Optional[bool] = None
