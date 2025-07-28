import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID
from esphome.cpp_generator import MockObjClass

# Namespace et classes
storage_ns = cg.esphome_ns.namespace("storage")
Storage = storage_ns.class_("Storage", cg.Component)
StorageClient = storage_ns.class_("StorageClient", cg.Component)

# Configuration
CONF_PREFIX = "path_prefix"

# Schéma de configuration pour le composant storage
CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(Storage),
    cv.Required(CONF_PREFIX): cv.string,
}).extend(cv.COMPONENT_SCHEMA)

def storage_schema(class_: MockObjClass = Storage) -> cv.Schema:
    """Schéma pour les clients de stockage"""
    return cv.Schema({
        cv.GenerateID(): cv.declare_id(class_),
        cv.Required(CONF_PREFIX): cv.string,
    }).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    """Fonction principale de génération du code"""
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    
    # Configuration du préfixe de chemin
    cg.add(var.set_path_prefix(config[CONF_PREFIX]))

async def storage_to_code(config):
    """Fonction pour configurer un client de stockage"""
    parent = await cg.get_variable(config[CONF_ID])
    return parent
