import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID, CONF_PLATFORM

DEPENDENCIES = ["esp32"]

# Define the storage namespace
storage_ns = cg.esphome_ns.namespace("storage")
Storage = storage_ns.class_("Storage", cg.Component)
SdMmcStorage = storage_ns.class_("SdMmcStorage", Storage)

MULTI_CONF = True

# Platform-specific configurations
PLATFORM_SCHEMA = cv.platformio_ns.one_of("sd_mmc")

CONFIG_SCHEMA = cv.All(
    cv.ensure_list,
    [
        cv.Schema(
            {
                cv.Required(CONF_PLATFORM): PLATFORM_SCHEMA,
                cv.GenerateID(): cv.declare_id(Storage),
                cv.Required("path_prefix"): cv.string,
                cv.Required("sd_mmc_id"): cv.use_id(cg.Component),
            }
        ).extend(cv.COMPONENT_SCHEMA)
    ],
)

async def to_code(config):
    for conf in config:
        platform = conf[CONF_PLATFORM]
        
        if platform == "sd_mmc":
            var = cg.new_Pvariable(conf[CONF_ID], SdMmcStorage)
            await cg.register_component(var, conf)
            
            # Set path prefix
            cg.add(var.set_path_prefix(conf["path_prefix"]))
            
            # Get SD MMC component reference
            sd_mmc_component = await cg.get_variable(conf["sd_mmc_id"])
            cg.add(var.set_sd_mmc_component(sd_mmc_component))
