#pragma once

#include "esphome/core/component.h"
#include "esphome/core/entity_base.h"
#include "../sd_mmc_card/sd_mmc_card.h"
#include <vector>
#include <map>

namespace esphome {
namespace storage {

enum class FileType {
  UNKNOWN = 0,
  TEXT = 1,
  AUDIO_WAV = 2,
  AUDIO_MP3 = 3,
  IMAGE_JPEG = 4,
  IMAGE_PNG = 5,
  IMAGE_BMP = 6,
  BINARY = 7
};

struct FileInfo {
  std::string path;
  size_t size;
  bool is_directory;
  size_t read_offset;
  FileType file_type;
  
  // Métadonnées spécifiques aux médias
  struct {
    uint32_t width = 0;
    uint32_t height = 0;
    uint8_t bits_per_pixel = 0;
  } image_info;
  
  struct {
    uint32_t sample_rate = 0;
    uint8_t channels = 0;
    uint8_t bits_per_sample = 0;
    uint32_t duration_ms = 0;
  } audio_info;
  
  FileInfo(std::string const &path, size_t size, bool is_directory);
  FileInfo();
  
  // Détection automatique du type de fichier basé sur l'extension
  void detect_file_type();
  
  // Méthodes utilitaires
  bool is_audio() const { return file_type == FileType::AUDIO_WAV || file_type == FileType::AUDIO_MP3; }
  bool is_image() const { return file_type >= FileType::IMAGE_JPEG && file_type <= FileType::IMAGE_BMP; }
  std::string get_mime_type() const;
};

class Storage : public EntityBase {
 public:
  // Fonctions directes existantes
  virtual uint8_t direct_read_byte(size_t offset) = 0;
  virtual bool direct_write_byte(uint8_t data) = 0;
  virtual bool direct_append_byte(uint8_t data) = 0;
  virtual size_t direct_read_byte_array(size_t offset, uint8_t *data, size_t data_length) = 0;
  virtual bool direct_write_byte_array(uint8_t *data, size_t data_length) = 0;
  virtual bool direct_append_byte_array(uint8_t *data, size_t data_length) = 0;
  
  // Nouvelles fonctions pour les médias
  virtual bool direct_seek(size_t position) = 0;
  virtual size_t direct_get_position() = 0;
  virtual bool direct_create_file(const std::string &path, size_t initial_size = 0) = 0;
  virtual bool direct_delete_file(const std::string &path) = 0;
  virtual bool direct_file_exists(const std::string &path) = 0;
  
  // API publique
  std::vector<FileInfo> list_directory(const std::string &path);
  FileInfo get_file_info(const std::string &path);
  void set_file(FileInfo *file);
  uint8_t read();
  bool write(uint8_t data);
  bool append(uint8_t data);
  size_t read_array(uint8_t *data, size_t data_length);
  bool write_array(uint8_t *data, size_t data_length);
  bool append_array(uint8_t *data, size_t data_length);
  
  // Nouvelles méthodes pour les médias
  bool seek(size_t position);
  size_t get_position();
  bool create_file(const std::string &path, size_t initial_size = 0);
  bool delete_file(const std::string &path);
  bool file_exists(const std::string &path);
  
  // Méthodes spécialisées pour l'audio
  bool write_audio_header_wav(uint32_t sample_rate, uint8_t channels, uint8_t bits_per_sample, uint32_t data_size);
  size_t read_audio_chunk(uint8_t *buffer, size_t chunk_size);
  bool write_audio_chunk(const uint8_t *buffer, size_t chunk_size);
  
  // Méthodes spécialisées pour les images
  bool write_image_header_bmp(uint32_t width, uint32_t height, uint8_t bits_per_pixel);
  size_t read_image_line(uint8_t *buffer, uint32_t line_number, uint32_t bytes_per_line);
  bool write_image_line(const uint8_t *buffer, uint32_t line_number, uint32_t bytes_per_line);

 protected:
  virtual void direct_set_file(const std::string &file) = 0;
  virtual FileInfo direct_get_file_info(const std::string &path) = 0;
  virtual std::vector<FileInfo> direct_list_directory(const std::string &path) = 0;
  
  void update_offset(size_t value);
  void parse_media_metadata(FileInfo *file_info);
  
  FileInfo *current_file_;
};

class StorageClient : public EntityBase {
 public:
  std::vector<FileInfo> list_directory(const std::string &path);
  FileInfo get_file_info(const std::string &path);
  void set_file(const std::string &path);
  uint8_t read();
  void set_read_offset(size_t offset);
  bool write(uint8_t data);
  bool append(uint8_t data);
  size_t read_array(uint8_t *data, size_t data_length);
  bool write_array(uint8_t *data, size_t data_length);
  bool append_array(uint8_t *data, size_t data_length);
  
  // Nouvelles méthodes
  bool seek(size_t position);
  size_t get_position();
  bool create_file(const std::string &path, size_t initial_size = 0);
  bool delete_file(const std::string &path);
  bool file_exists(const std::string &path);
  
  // Méthodes spécialisées pour l'audio
  bool write_audio_header_wav(uint32_t sample_rate, uint8_t channels, uint8_t bits_per_sample, uint32_t data_size);
  size_t read_audio_chunk(uint8_t *buffer, size_t chunk_size);
  bool write_audio_chunk(const uint8_t *buffer, size_t chunk_size);
  
  // Méthodes spécialisées pour les images
  bool write_image_header_bmp(uint32_t width, uint32_t height, uint8_t bits_per_pixel);
  size_t read_image_line(uint8_t *buffer, uint32_t line_number, uint32_t bytes_per_line);
  bool write_image_line(const uint8_t *buffer, uint32_t line_number, uint32_t bytes_per_line);
  
  // Méthodes utilitaires
  std::vector<FileInfo> list_audio_files(const std::string &path = "");
  std::vector<FileInfo> list_image_files(const std::string &path = "");
  
  static void add_storage(Storage *storage_inst, std::string prefix);

 protected:
  static std::map<std::string, Storage *> storages;
  Storage *current_storage_;
  FileInfo current_file_;
  
  Storage* get_storage_for_path(const std::string &path, std::string &relative_path);
};

}  // namespace storage
}  // namespace esphome
