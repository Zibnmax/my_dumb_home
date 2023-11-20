from aiohttp import web, ClientSession
import jinja2
import aiohttp_jinja2

ESP_WATER_TANK_IP = 'http://192.168.1.13'
routes = web.RouteTableDef()

@aiohttp_jinja2.template("index.html")
@routes.get('/')
async def main_page(request):
    return {'title': 'Hi there'}
    # return web.Response(text="Hi there!")

@routes.post("/water-tank/send-data")
async def water_tank_read(request):
    data = await request.query()
    context = {
    "name": data.get("name", ""),
    "ip": data.get("ip", ""),
    "wifi_connections_count": data.get("wifi_connections_count", 0),
    "float_low": data.get("float_low", ""),
    "float_high": data.get("float_high", ""),
    "low_temp": data.get("low_temp", 0),
    "high_temp": data.get("high_temp", 0),
    "temp": data.get("temp", ""),
    "is_temp_sensor_ok": data.get("is_temp_sensor_ok", ""),
    "is_pump_available": data.get("is_pump_available", ""),
    "is_heater_available": data.get("is_heater_available", ""),
    "pump_state": data.get("pump_state", ""),
    "heater_state": data.get("heater_state", ""),
    "is_force_fill_tank_ready": data.get("is_force_fill_tank_ready", ""),
    "is_force_heat_water_ready": data.get("is_force_heat_water_ready", ""),
    "is_shower_ready": data.get("is_shower_ready", ""),
    }

@aiohttp_jinja2.template("water-tank.html")
@routes.get('/water-tank')
async def get_water_tank_readings(request):
    async with ClientSession() as session:
        async with session.get(ESP_WATER_TANK_IP) as resp:
            print(resp.status)
            data = await resp.json()
            print(data)
            context = {
                "name": data.get("name", ""),
                "ip": data.get("ip", ""),
                "wifi_connections_count": data.get("wifi_connections_count", 0),
                "float_low": data.get("float_low", ""),
                "float_high": data.get("float_high", ""),
                "low_temp": data.get("low_temp", 0),
                "high_temp": data.get("high_temp", 0),
                "temp": data.get("temp", ""),
                "is_temp_sensor_ok": data.get("is_temp_sensor_ok", ""),
                "is_pump_available": data.get("is_pump_available", ""),
                "is_heater_available": data.get("is_heater_available", ""),
                "is_mixer_available": data.get("is_mixer_available", ""),
                "pump_state": data.get("pump_state", ""),
                "heater_state": data.get("heater_state", ""),
                "mixer_state": data.get("mixer_state", ""),
                "is_force_fill_tank_ready": data.get("is_force_fill_tank_ready", ""),
                "is_force_heat_water_ready": data.get("is_force_heat_water_ready", ""),
                "is_shower_ready": data.get("is_shower_ready", ""),
            }
            return aiohttp_jinja2.render_template("water-tank.html", request, context)


def start_server():
    app = web.Application()
    app.add_routes(routes)
    app.router.add_static('/static/', path='static', name='static')
    aiohttp_jinja2.setup(app=app, loader=jinja2.FileSystemLoader("templates"))
    web.run_app(app, port=80)

if __name__ == "__main__":
    start_server()
