import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID, CONF_PLATFORM

DEPENDENCIES = ["esp32"]

# Define the storage namespace
storage_ns = cg.esphome_ns.namespace("storage")
Storage = storage_ns.class_("Storage", cg.Component)
SdMmcStorage = storage_ns.class_("SdMmcStorage", Storage)

MULTI_CONF = True

# Simple platform validation
def validate_platform(value):
    if value == "sd_mmc":
        return value
    raise cv.Invalid(f"Platform '{value}' is not supported")

CONFIG_SCHEMA = cv.All(
    cv.ensure_list,
    [
        cv.Schema(
            {
                cv.Required(CONF_PLATFORM): validate_platform,
                cv.GenerateID(): cv.declare_id(SdMmcStorage),
                cv.Required("path_prefix"): cv.string,
                cv.Required("sd_mmc_id"): cv.use_id(cg.Component),
            }
        ).extend(cv.COMPONENT_SCHEMA)
    ],
)

async def to_code(config):
    for conf in config:
        var = cg.new_Pvariable(conf[CONF_ID])
        await cg.register_component(var, conf)
        
        # Set path prefix
        cg.add(var.set_path_prefix(conf["path_prefix"]))
        
        # Get SD MMC component reference
        sd_mmc_component = await cg.get_variable(conf["sd_mmc_id"])
        cg.add(var.set_sd_mmc_component(sd_mmc_component))
