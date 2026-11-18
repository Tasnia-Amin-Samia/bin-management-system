CREATE DATABASE IF NOT EXISTS smart_bin
  CHARACTER SET utf8mb4
  COLLATE utf8mb4_unicode_ci;

USE smart_bin;

CREATE TABLE IF NOT EXISTS bins (
    bin_id VARCHAR(40) PRIMARY KEY,
    bin_name VARCHAR(100) NOT NULL,
    location VARCHAR(150) NOT NULL,
    full_threshold TINYINT UNSIGNED NOT NULL DEFAULT 80,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS readings (
    reading_id BIGINT UNSIGNED AUTO_INCREMENT PRIMARY KEY,
    bin_id VARCHAR(40) NOT NULL,
    distance_cm DECIMAL(7,2) NOT NULL,
    fill_percent TINYINT UNSIGNED NOT NULL,
    recorded_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    INDEX idx_bin_recorded (bin_id, recorded_at),
    CONSTRAINT fk_readings_bin
      FOREIGN KEY (bin_id) REFERENCES bins(bin_id)
      ON DELETE CASCADE
);

INSERT INTO bins (bin_id, bin_name, location, full_threshold)
VALUES ('BIN-001', 'Smart Bin 001', 'Main Campus Area', 80)
ON DUPLICATE KEY UPDATE
  bin_name = VALUES(bin_name),
  location = VALUES(location),
  full_threshold = VALUES(full_threshold);
