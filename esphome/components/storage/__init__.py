import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID

sd_mmc_ns = cg.esphome_ns.namespace("sd_mmc_card")
SdMmc = sd_mmc_ns.class_("SdMmc", cg.Component)
SDMMCStorage = cg.esphome_ns.namespace("storage").class_("SD_MMC_Storage", cg.Component)

CONF_SD_MMC_ID = "sd_mmc_id"

def _get_storage_schema(cls):
    # Import différé pour éviter circular import
    from esphome.components import storage
    return storage.storage_schema(cls)

CONFIG_SCHEMA = _get_storage_schema(SDMMCStorage).extend(
    {
        cv.GenerateID(): cv.declare_id(SDMMCStorage),
        cv.Required(CONF_SD_MMC_ID): cv.use_id(SdMmc),
    }
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    sd_mmc_component = await cg.get_variable(config[CONF_SD_MMC_ID])
    cg.add(var.set_sd_mmc(sd_mmc_component))

    from esphome.components import storage
    await storage.storage_to_code(config)


