import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID

sd_mmc_ns = cg.esphome_ns.namespace("sd_mmc_card")
sd_mmc_storage = sd_mmc_ns.class_("SD_MMC_Storage", cg.Component)

CONF_SD_MMC_ID = "sd_mmc_id"

def _get_storage_schema(cls):
    from esphome.components.storage import storage_schema  # ✅ import ici
    return storage_schema(cls)

CONFIG_SCHEMA = _get_storage_schema(sd_mmc_storage).extend(
    {
        cv.GenerateID(): cv.declare_id(sd_mmc_storage),
        cv.Required(CONF_SD_MMC_ID): cv.use_id(sd_mmc_ns.class_("SdMmc")),
    }
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    sd_mmc_component = await cg.get_variable(config[CONF_SD_MMC_ID])
    cg.add(var.set_sd_mmc(sd_mmc_component))

    from esphome.components import storage  # ✅ import ici aussi
    await storage.storage_to_code(config)

