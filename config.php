<?php
// Local XAMPP database configuration.
// Change these values if your MySQL setup uses different credentials.
define('DB_HOST', 'localhost');
define('DB_NAME', 'smart_bin');
define('DB_USER', 'root');
define('DB_PASS', '');

// Replace this with a long random secret. Use the same value in the ESP8266 sketch.
// Do not use a real password or publish this token in a public repository.
define('API_TOKEN', 'CHANGE_THIS_TO_A_LONG_RANDOM_SECRET');

function get_db_connection(): PDO {
    $dsn = 'mysql:host=' . DB_HOST . ';dbname=' . DB_NAME . ';charset=utf8mb4';
    return new PDO($dsn, DB_USER, DB_PASS, [
        PDO::ATTR_ERRMODE => PDO::ERRMODE_EXCEPTION,
        PDO::ATTR_DEFAULT_FETCH_MODE => PDO::FETCH_ASSOC,
        PDO::ATTR_EMULATE_PREPARES => false,
    ]);
}
