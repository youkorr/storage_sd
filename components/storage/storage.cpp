#include "storage.h"

#include "esphome/core/hal.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"
#include "esphome/core/component.h"
#include <algorithm>
#include <cctype>

namespace esphome {
namespace storage {

static const char *const TAG = "storage";

// Signatures de fichiers pour la détection automatique
static const uint8_t WAV_SIGNATURE[] = {0x52, 0x49, 0x46, 0x46}; // "RIFF"
static const uint8_t MP3_SIGNATURE[] = {0xFF, 0xFB}; // MP3 frame sync
static const uint8_t JPEG_SIGNATURE[] = {0xFF, 0xD8, 0xFF};
static const uint8_t PNG_SIGNATURE[] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
static const uint8_t BMP_SIGNATURE[] = {0x42, 0x4D}; // "BM"

FileInfo::FileInfo(std::string const &path, size_t size, bool is_directory)
    : path(path), size(size), is_directory(is_directory), file_type(FileType::UNKNOWN) {
  this->read_offset = 0;
  detect_file_type();
}

FileInfo::FileInfo() : path(), size(), is_directory(), file_type(FileType::UNKNOWN) { 
  this->read_offset = 0; 
}

void FileInfo::detect_file_type() {
  if (is_directory) {
    file_type = FileType::UNKNOWN;
    return;
  }
  
  // Conversion en minuscules pour la comparaison
  std::string lower_path = path;
  std::transform(lower_path.begin(), lower_path.end(), lower_path.begin(), ::tolower);
  
  // Détection par extension
  if (lower_path.ends_with(".wav")) {
    file_type = FileType::AUDIO_WAV;
  } else if (lower_path.ends_with(".mp3")) {
    file_type = FileType::AUDIO_MP3;
  } else if (lower_path.ends_with(".jpg") || lower_path.ends_with(".jpeg")) {
    file_type = FileType::IMAGE_JPEG;
  } else if (lower_path.ends_with(".png")) {
    file_type = FileType::IMAGE_PNG;
  } else if (lower_path.ends_with(".bmp")) {
    file_type = FileType::IMAGE_BMP;
  } else if (lower_path.ends_with(".txt") || lower_path.ends_with(".log")) {
    file_type = FileType::TEXT;
  } else {
    file_type = FileType::BINARY;
  }
}

std::string FileInfo::get_mime_type() const {
  switch (file_type) {
    case FileType::AUDIO_WAV: return "audio/wav";
    case FileType::AUDIO_MP3: return "audio/mpeg";
    case FileType::IMAGE_JPEG: return "image/jpeg";
    case FileType::IMAGE_PNG: return "image/png";
    case FileType::IMAGE_BMP: return "image/bmp";
    case FileType::TEXT: return "text/plain";
    default: return "application/octet-stream";
  }
}

// Implémentation Storage
std::vector<FileInfo> Storage::list_directory(const std::string &path) { 
  return this->direct_list_directory(path); 
}

FileInfo Storage::get_file_info(const std::string &path) { 
  FileInfo info = this->direct_get_file_info(path);
  if (!info.is_directory && info.size > 0) {
    parse_media_metadata(&info);
  }
  return info;
}

void Storage::set_file(FileInfo *file) {
  if (this->current_file_ != file) {
    this->current_file_ = file;
    this->direct_set_file(this->current_file_->path);
  }
}

uint8_t Storage::read() {
  uint8_t data = this->direct_read_byte(this->current_file_->read_offset);
  this->update_offset(1);
  return data;
}

bool Storage::write(uint8_t data) {
  return this->direct_write_byte(data);
}

bool Storage::append(uint8_t data) { 
  return this->direct_append_byte(data); 
}

size_t Storage::read_array(uint8_t *data, size_t data_length) {
  size_t num_bytes_read = this->direct_read_byte_array(this->current_file_->read_offset, data, data_length);
  this->update_offset(num_bytes_read);
  return num_bytes_read;
}

bool Storage::write_array(uint8_t *data, size_t data_length) {
  return this->direct_write_byte_array(data, data_length);
}

bool Storage::append_array(uint8_t *data, size_t data_length) {
  return this->direct_append_byte_array(data, data_length);
}

bool Storage::seek(size_t position) {
  if (this->current_file_) {
    this->current_file_->read_offset = position;
    return this->direct_seek(position);
  }
  return false;
}

size_t Storage::get_position() {
  if (this->current_file_) {
    return this->current_file_->read_offset;
  }
  return 0;
}

bool Storage::create_file(const std::string &path, size_t initial_size) {
  return this->direct_create_file(path, initial_size);
}

bool Storage::delete_file(const std::string &path) {
  return this->direct_delete_file(path);
}

bool Storage::file_exists(const std::string &path) {
  return this->direct_file_exists(path);
}

bool Storage::write_audio_header_wav(uint32_t sample_rate, uint8_t channels, uint8_t bits_per_sample, uint32_t data_size) {
  // En-tête WAV simplifié
  uint8_t header[44];
  uint32_t file_size = data_size + 36;
  uint32_t byte_rate = sample_rate * channels * (bits_per_sample / 8);
  uint16_t block_align = channels * (bits_per_sample / 8);
  
  // RIFF header
  memcpy(header, "RIFF", 4);
  memcpy(header + 4, &file_size, 4);
  memcpy(header + 8, "WAVE", 4);
  
  // fmt chunk
  memcpy(header + 12, "fmt ", 4);
  uint32_t fmt_size = 16;
  memcpy(header + 16, &fmt_size, 4);
  uint16_t audio_format = 1; // PCM
  memcpy(header + 20, &audio_format, 2);
  memcpy(header + 22, &channels, 2);
  memcpy(header + 24, &sample_rate, 4);
  memcpy(header + 28, &byte_rate, 4);
  memcpy(header + 32, &block_align, 2);
  memcpy(header + 34, &bits_per_sample, 2);
  
  // data chunk header
  memcpy(header + 36, "data", 4);
  memcpy(header + 40, &data_size, 4);
  
  return this->write_array(header, 44);
}

size_t Storage::read_audio_chunk(uint8_t *buffer, size_t chunk_size) {
  return this->read_array(buffer, chunk_size);
}

bool Storage::write_audio_chunk(const uint8_t *buffer, size_t chunk_size) {
  return this->write_array(const_cast<uint8_t*>(buffer), chunk_size);
}

bool Storage::write_image_header_bmp(uint32_t width, uint32_t height, uint8_t bits_per_pixel) {
  uint32_t row_size = ((bits_per_pixel * width + 31) / 32) * 4;
  uint32_t image_size = row_size * height;
  uint32_t file_size = 54 + image_size;
  
  uint8_t header[54] = {0};
  
  // BMP signature
  header[0] = 'B';
  header[1] = 'M';
  
  // File size
  memcpy(header + 2, &file_size, 4);
  
  // Data offset
  uint32_t data_offset = 54;
  memcpy(header + 10, &data_offset, 4);
  
  // Info header size
  uint32_t info_size = 40;
  memcpy(header + 14, &info_size, 4);
  
  // Width and height
  memcpy(header + 18, &width, 4);
  memcpy(header + 22, &height, 4);
  
  // Planes
  uint16_t planes = 1;
  memcpy(header + 26, &planes, 2);
  
  // Bits per pixel
  memcpy(header + 28, &bits_per_pixel, 2);
  
  // Image size
  memcpy(header + 34, &image_size, 4);
  
  return this->write_array(header, 54);
}

size_t Storage::read_image_line(uint8_t *buffer, uint32_t line_number, uint32_t bytes_per_line) {
  size_t offset = 54 + (line_number * bytes_per_line); // 54 = BMP header size
  return this->direct_read_byte_array(offset, buffer, bytes_per_line);
}

bool Storage::write_image_line(const uint8_t *buffer, uint32_t line_number, uint32_t bytes_per_line) {
  // Pour BMP, les lignes sont stockées de bas en haut
  if (!this->seek(54 + (line_number * bytes_per_line))) {
    return false;
  }
  return this->write_array(const_cast<uint8_t*>(buffer), bytes_per_line);
}

void Storage::update_offset(size_t value) {
  this->current_file_->read_offset += value;
}

void Storage::parse_media_metadata(FileInfo *file_info) {
  if (!file_info || file_info->is_directory || file_info->size < 8) {
    return;
  }
  
  // Sauvegarde de la position actuelle
  FileInfo *old_file = this->current_file_;
  this->current_file_ = file_info;
  this->direct_set_file(file_info->path);
  
  uint8_t header[44];
  size_t bytes_read = this->direct_read_byte_array(0, header, std::min(sizeof(header), file_info->size));
  
  if (bytes_read >= 8) {
    // Vérification WAV
    if (memcmp(header, WAV_SIGNATURE, 4) == 0 && memcmp(header + 8, "WAVE", 4) == 0) {
      file_info->file_type = FileType::AUDIO_WAV;
      if (bytes_read >= 44) {
        memcpy(&file_info->audio_info.sample_rate, header + 24, 4);
        file_info->audio_info.channels = header[22];
        file_info->audio_info.bits_per_sample = header[34];
        uint32_t data_size;
        memcpy(&data_size, header + 40, 4);
        if (file_info->audio_info.sample_rate > 0 && file_info->audio_info.channels > 0) {
          uint32_t byte_rate = file_info->audio_info.sample_rate * file_info->audio_info.channels * (file_info->audio_info.bits_per_sample / 8);
          if (byte_rate > 0) {
            file_info->audio_info.duration_ms = (data_size * 1000) / byte_rate;
          }
        }
      }
    }
    // Vérification BMP
    else if (memcmp(header, BMP_SIGNATURE, 2) == 0) {
      file_info->file_type = FileType::IMAGE_BMP;
      if (bytes_read >= 30) {
        memcpy(&file_info->image_info.width, header + 18, 4);
        memcpy(&file_info->image_info.height, header + 22, 4);
        file_info->image_info.bits_per_pixel = header[28];
      }
    }
    // Vérification PNG
    else if (bytes_read >= 8 && memcmp(header, PNG_SIGNATURE, 8) == 0) {
      file_info->file_type = FileType::IMAGE_PNG;
      // Pour PNG, les dimensions sont aux octets 16-23 (après IHDR)
      if (bytes_read >= 24) {
        // Lecture directe des dimensions PNG (big-endian)
        file_info->image_info.width = (header[16] << 24) | (header[17] << 16) | (header[18] << 8) | header[19];
        file_info->image_info.height = (header[20] << 24) | (header[21] << 16) | (header[22] << 8) | header[23];
        file_info->image_info.bits_per_pixel = header[24];
      }
    }
    // Vérification JPEG
    else if (memcmp(header, JPEG_SIGNATURE, 3) == 0) {
      file_info->file_type = FileType::IMAGE_JPEG;
      // Pour JPEG, il faudrait parser les segments pour trouver SOF
      // Implémentation simplifiée
    }
  }
  
  // Restauration de l'ancien fichier
  this->current_file_ = old_file;
  if (old_file) {
    this->direct_set_file(old_file->path);
  }
}

// Implémentation StorageClient
Storage* StorageClient::get_storage_for_path(const std::string &path, std::string &relative_path) {
  int prefix_end = path.find("://");
  if (prefix_end < 0) {
    ESP_LOGE(TAG, "Invalid path. Must start with a valid prefix");
    return nullptr;
  }
  
  std::string prefix = path.substr(0, prefix_end);
  auto nstorage = storages.find(prefix);
  if (nstorage == storages.end()) {
    ESP_LOGE(TAG, "storage %s prefix does not exist", prefix.c_str());
    return nullptr;
  }
  
  relative_path = path.substr(prefix_end + 3);
  return nstorage->second;
}

std::vector<FileInfo> StorageClient::list_directory(const std::string &path) {
  std::string relative_path;
  Storage* storage = get_storage_for_path(path, relative_path);
  if (!storage) {
    return std::vector<FileInfo>();
  }
  
  std::vector<FileInfo> result = storage->list_directory(relative_path);
  std::string prefix = path.substr(0, path.find("://"));
  
  for (auto &file : result) {
    file.path = prefix + "://" + file.path;
  }
  return result;
}

FileInfo StorageClient::get_file_info(const std::string &path) {
  std::string relative_path;
  Storage* storage = get_storage_for_path(path, relative_path);
  if (!storage) {
    return FileInfo();
  }
  
  FileInfo result = storage->get_file_info(relative_path);
  std::string prefix = path.substr(0, path.find("://"));
  result.path = prefix + "://" + result.path;
  return result;
}

void StorageClient::set_file(const std::string &path) {
  std::string relative_path;
  Storage* storage = get_storage_for_path(path,
}

}  // namespace storage
}  // namespace esphome
