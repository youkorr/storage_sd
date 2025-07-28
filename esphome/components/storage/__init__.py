import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID

DEPENDENCIES = ["esp32"]

# Define the storage namespace
storage_ns = cg.esphome_ns.namespace("storage")
Storage = storage_ns.class_("Storage", cg.Component)
SdMmcStorage = storage_ns.class_("SdMmcStorage", Storage)

MULTI_CONF = True

# Define the platform schemas
PLATFORM_SCHEMAS = {
    "sd_mmc": cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(SdMmcStorage),
            cv.Required("path_prefix"): cv.string,
            cv.Required("sd_mmc_id"): cv.use_id(cg.Component),
        }
    ).extend(cv.COMPONENT_SCHEMA),
}

def validate_platform(config):
    if "platform" not in config:
        raise cv.Invalid("Platform must be specified")
    
    platform = config["platform"]
    if platform not in PLATFORM_SCHEMAS:
        raise cv.Invalid(f"Unknown platform '{platform}'")
    
    return PLATFORM_SCHEMAS[platform](config)

CONFIG_SCHEMA = cv.All(
    cv.ensure_list,
    [validate_platform]
)

async def to_code(config):
    for conf in config:
        platform = conf["platform"]
        
        if platform == "sd_mmc":
            var = cg.new_Pvariable(conf[CONF_ID])
            await cg.register_component(var, conf)
            
            # Set path prefix
            cg.add(var.set_path_prefix(conf["path_prefix"]))
            
            # Get SD MMC component reference
            sd_mmc_component = await cg.get_variable(conf["sd_mmc_id"])
            cg.add(var.set_sd_mmc_component(sd_mmc_component))
