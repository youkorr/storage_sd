import esphome.codegen as cg
from esphome.components import storage
import esphome.config_validation as cv
from esphome.const import CONF_ID

# ✅ Ajout du namespace manquant
sd_mmc_ns = cg.esphome_ns.namespace("sd_mmc_card")  # à adapter si ton namespace a un autre nom

DEPENDENCIES = ["sd_mmc_card", "storage"]
sd_mmc_storage = sd_mmc_ns.class_("SD_MMC_Storage", cg.Component)

CONF_SD_MMC_ID = "sd_mmc_id"  # à ajouter si pas déjà défini quelque part

CONFIG_SCHEMA = storage.storage_schema(sd_mmc_storage).extend(
    {
        cv.GenerateID(): cv.declare_id(sd_mmc_storage),
        cv.Required(CONF_SD_MMC_ID): cv.use_id(sd_mmc_ns.class_("SdMmc")),  # ou juste `cg.Component` si pas typé
    }
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    sd_mmc_component = await cg.get_variable(config[CONF_SD_MMC_ID])
    cg.add(var.set_sd_mmc(sd_mmc_component))
    await storage.storage_to_code(config)

